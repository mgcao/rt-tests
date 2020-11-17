#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <getopt.h>
#include <pthread.h>
#include <signal.h>
#include <sched.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdbool.h>

#include <linux/unistd.h>

#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/utsname.h>
#include <sys/mman.h>

#include "msr.h"
#include "pmc.h"

#define DEBUG 0

#if DEBUG
	#define dbg_info(...) printf(__VA_ARGS__)
#else
	#define dbg_info(...)
#endif

//for PMU (Performance Monitor Unit counter), port from Kaige's PMU module code

#define PMC_MAX 8

#define PERF_GLOBAL_CTRL		0x38f /* Global CTRL MSR */
#define PERF_FIXED_CTR0			0x309 /* Fixed counter MSR for retired instructions */
#define PERF_FIXED_CTR1			0x30a /* Fixed counter MSR for cpu cycles */
#define PERF_FIXED_CTR2			0x30b /* Fixed counter MSR for cpu ref cycles */
#define PERF_FIXED_CTR_CTRL		0x38d /* Fixed counter control MSR */

#define PERF_GP_CTR_START_ADDR		0x4c1 /* General purpose counter MSR start address */
#define PERF_GP_EVTSEL_START_ADDR 	0x186 /* General purpose eventsel MSR start address */

#define PERFMON_EVENTSEL_ENABLE		(1ULL << 22)
#define PERFMON_EVENTSEL_USR           	(1ULL << 16)
#define PERFMON_EVENTSEL_OS            	(1ULL << 17)

#define RETIRED_INSTRUCTION_IDX  	0
#define CPU_CYCLES_IDX           	1
#define CPU_REF_CYCLES_IDX       	2

#define CPU_CACHE_REFERENCES_IDX 	0
#define CPU_CACHE_MISSES_IDX     	1
#define CPU_BRANCHES_IDX         	2
#define CPU_BRANCH_MISSES_IDX    	3

#define COUNTER_INITIAL_VAL 		0x800000000001ULL
#define TYPE_FIXED_CTR  		(1ULL << 30)

#define CACHE_REFERENCES_EVENT_SELECT   0x2e
#define CACHE_REFERENCES_UMASK          0x4f

#define CACHE_MISS_EVENT_SELECT   	0x2e
#define CACHE_MISS_UMASK          	0x41

#define BRANCH_EVENT_SELECT   		0xc4
#define BRANCH_UMASK          		0x00

#define BRANCH_MISSED_EVENT_SELECT   	0xc5
#define BRANCH_MISSED_UMASK          	0x00

static uint64_t cpu_retired_instructions;
static uint64_t cpu_cycles;
static uint64_t cpu_ref_cycles;
static uint64_t cache_references;
static uint64_t cache_misses;
static uint64_t branches;
static uint64_t branch_misses;

static int cpu_index = -1;

static inline uint64_t _x86_pmc_read(unsigned int idx)
{
    uint32_t lo, hi;

	dbg_info("read-pmc: %d\n", idx);

    __asm__ volatile("rdpmc" : "=a" (lo), "=d" (hi) : "c" (idx));

    return ((uint64_t)lo) | (((uint64_t)hi) << 32);
}


static void start_fixed_perf_counter(uint32_t counter_idx, uint32_t counter_msr)
{
	uint64_t bits, mask, val;

	/* set initial value to counter MSR */
    wrmsr_on_cpu(counter_msr, cpu_index, COUNTER_INITIAL_VAL);

	/* config eventsel MSR */
	bits = 0x8ULL;
	bits |= 0x2 | 0x1;
	bits <<= (counter_idx * 4);

	mask = 0xfULL << (counter_idx * 4);

    val = rdmsr_on_cpu(PERF_FIXED_CTR_CTRL, cpu_index);

	val &= ~mask;
	val |= bits;
	
    wrmsr_on_cpu(PERF_FIXED_CTR_CTRL, cpu_index, val);
}

static void stop_fixed_perf_counter(uint32_t counter_idx, uint64_t *ret)
{
	/* reset eventsel MSR */
	uint64_t val, mask;

	mask = 0xfULL << (counter_idx * 4);

    val = rdmsr_on_cpu(PERF_FIXED_CTR_CTRL, cpu_index);
	val &= ~mask;
	
    wrmsr_on_cpu(PERF_FIXED_CTR_CTRL, cpu_index, val);

	/* rdpmcl */
	val = _x86_pmc_read(counter_idx | TYPE_FIXED_CTR);
	val -= COUNTER_INITIAL_VAL;

	if (ret)
		*ret = val;
}

static void start_gp_perf_counter(uint32_t idx, uint64_t evtsel, uint64_t umask)
{
	uint64_t val;

	/* set initial value to counter MSR */
    wrmsr_on_cpu(PERF_GP_CTR_START_ADDR + idx, cpu_index, COUNTER_INITIAL_VAL);

	/* config event select MSR */
	val = 0;
	val |= PERFMON_EVENTSEL_USR | PERFMON_EVENTSEL_OS;
	val |= PERFMON_EVENTSEL_ENABLE;
	val |= evtsel;
	val |= umask << 8;
	
    wrmsr_on_cpu(PERF_GP_EVTSEL_START_ADDR + idx, cpu_index, val);
}

static void stop_gp_perf_counter(int counter_idx, uint64_t *ret)
{
	uint64_t val;

	/* disable counting by writing event select MSR */

    val = rdmsr_on_cpu(PERF_GP_EVTSEL_START_ADDR + counter_idx, cpu_index);
  
	val &= ~PERFMON_EVENTSEL_ENABLE;
	wrmsr_on_cpu(PERF_GP_EVTSEL_START_ADDR + counter_idx, cpu_index, val);

	val = _x86_pmc_read(counter_idx);
	val -= COUNTER_INITIAL_VAL;

	if (ret)
		*ret = val;
}

void pmc_start(int cpu)
{
	dbg_info("pmc-start: %d\n", cpu);

    cpu_index = cpu;

	/* for cpu retired instructions */
	start_fixed_perf_counter(RETIRED_INSTRUCTION_IDX, PERF_FIXED_CTR0);

	/* for cpu cycles */
	start_fixed_perf_counter(CPU_CYCLES_IDX, PERF_FIXED_CTR1);

	/* for cpu ref cycles */
	start_fixed_perf_counter(CPU_REF_CYCLES_IDX, PERF_FIXED_CTR2);

	/* for cache references */
	start_gp_perf_counter(CPU_CACHE_REFERENCES_IDX, CACHE_REFERENCES_EVENT_SELECT, CACHE_REFERENCES_UMASK);

	/* for cache misses */
	start_gp_perf_counter(CPU_CACHE_MISSES_IDX, CACHE_MISS_EVENT_SELECT, CACHE_MISS_UMASK);

	/* for instruction branches */
	start_gp_perf_counter(CPU_BRANCHES_IDX, BRANCH_EVENT_SELECT, BRANCH_UMASK);

	/* for branch miss */
	start_gp_perf_counter(CPU_BRANCH_MISSES_IDX, BRANCH_MISSED_EVENT_SELECT, BRANCH_MISSED_UMASK);

	/* enable all counter */
	wrmsr_on_cpu(PERF_GLOBAL_CTRL, cpu_index, (0x7ULL << 32) | 0xff);

}

void pmc_stop(int cpu)
{
	dbg_info("pmc-stop: %d\n", cpu);

    cpu_index = cpu;

	/* disable all counter */
	wrmsr_on_cpu(PERF_GLOBAL_CTRL, cpu_index, 0);

	/* for cpu retired instructions */
	stop_fixed_perf_counter(RETIRED_INSTRUCTION_IDX, &cpu_retired_instructions);

	/* for cpu cycles */
	stop_fixed_perf_counter(CPU_CYCLES_IDX, &cpu_cycles);

	/* for cpu ref cycles */
	stop_fixed_perf_counter(CPU_REF_CYCLES_IDX, &cpu_ref_cycles);

	/* for cache references */
	stop_gp_perf_counter(CPU_CACHE_REFERENCES_IDX, &cache_references);

	/* for cache misses */
	stop_gp_perf_counter(CPU_CACHE_MISSES_IDX, &cache_misses);

	/* for instruction branches */
	stop_gp_perf_counter(CPU_BRANCHES_IDX, &branches);

	/* for branch misses */
	stop_gp_perf_counter(CPU_BRANCH_MISSES_IDX, &branch_misses);
}

uint64_t pmc_get_cache_refs(void)
{
	uint64_t val;

	val = _x86_pmc_read(CPU_CACHE_REFERENCES_IDX);
	val -= COUNTER_INITIAL_VAL;

	return val;
}

uint64_t pmc_get_cache_misses(void)
{
	uint64_t val;

	val = _x86_pmc_read(CPU_CACHE_MISSES_IDX);
	val -= COUNTER_INITIAL_VAL;

	return val;
}

void pmc_report(FILE *fd, uint32_t times)
{
#if 0
    fprintf("cpu: %d:\n", cpu_index);
	printf("retired_insts  = 0x%lx\n", cpu_retired_instructions);
	printf("cpu_cycles     = 0x%lx\n", cpu_cycles);
	printf("cpu_ref_cycles = 0x%lx\n", cpu_ref_cycles);
	printf("cache_refs     = 0x%lx\n", cache_references);
	printf("cache_misses   = 0x%lx\n", cache_misses);
	printf("branches       = 0x%lx\n", branches);
	printf("branches_miss  = 0x%lx\n", branch_misses);
#else

	fprintf(fd, "times:%8u  retired=%8lu cycles/refs:%8lu /%8lu  cache-ref/miss:%8lu /%8lu branch/miss:%8lu /%8lu\n",
		times, cpu_retired_instructions, cpu_cycles, cpu_ref_cycles, cache_references, cache_misses, branches, branch_misses);
#endif
}

#define RDPMC_PATH "/sys/devices/cpu/rdpmc"

bool pmc_init(int cpu)
{
    cpu_index = cpu;

    //emable rdpmc in userland
	int ret = system("echo 2 > " RDPMC_PATH);

	dbg_info("pmc_init system ret=%d\n", ret);
	if (ret != 0) {
		fprintf(stderr, RDPMC_PATH " set failed, please check! need expose it from kernel!\n");
		return false;
	}
    //use cpuid to check its version: >=4

	return true;
}