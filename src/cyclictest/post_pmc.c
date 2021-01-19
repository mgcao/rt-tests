#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stddef.h>
#include <string.h>

#include "msr.h"
#include "pmc.h"

//#define PMC_MAX 8
#define PMC_MAX 4  //EHL just 4 general counters

#define TYPE_FIXED_CTR0  		(1ULL << 30)
#define TYPE_FIXED_CTR1  		((1ULL << 30) | 1)
#define TYPE_FIXED_CTR2  		((1ULL << 30) | 2)

#define PERF_GLOBAL_CTRL		0x38f /* Global CTRL MSR */

static uint64_t g_start[PMC_MAX];
static uint64_t g_stop[PMC_MAX];
static uint64_t delta[PMC_MAX];

static uint64_t pmc_inst_start;  //total inst
static uint64_t pmc_inst_stop;
static uint64_t pmc_cycles_start; //total core cycles
static uint64_t pmc_cycles_stop;
static uint64_t pmc_tsc_start;  //total cycles/tsc based
static uint64_t pmc_tsc_stop;

static inline uint64_t native_rdpmc(uint32_t idx)
{
    uint32_t lo, hi;

    __asm__ volatile("rdpmc" : "=a" (lo), "=d" (hi) : "c" (idx));

    return ((uint64_t)lo) | (((uint64_t)hi) << 32);
}

void pmc_post_read_start(void)
{
	int cnt;

	pmc_inst_start = native_rdpmc(TYPE_FIXED_CTR0);
	pmc_cycles_start = native_rdpmc(TYPE_FIXED_CTR1);
	pmc_tsc_start = native_rdpmc(TYPE_FIXED_CTR2);

	for( cnt = 0; cnt < PMC_MAX; cnt++) {
		g_start[cnt] = native_rdpmc(cnt);
	}
}

void pmc_post_read_stop(void)
{
	int cnt;

	pmc_inst_stop = native_rdpmc(TYPE_FIXED_CTR0);
	pmc_cycles_stop = native_rdpmc(TYPE_FIXED_CTR1);
	pmc_tsc_stop = native_rdpmc(TYPE_FIXED_CTR2);

	for( cnt = 0; cnt < PMC_MAX; cnt++) {
		g_stop[cnt] = native_rdpmc(cnt);
	}

    for( cnt = 0; cnt < PMC_MAX; cnt++) {
        delta[cnt] = g_stop[cnt] - g_start[cnt];
    }

}

void pmc_post_report(FILE *fd, uint32_t times)
{
#if PMC_MAX == 8 // just for normal case
	fprintf(fd, "times:%8u inst:%8lu [0]:%8lu [1]:%8lu [2]:%8lu [3]:%8lu [4]:%8lu [5]:%8lu [6]:%8lu [7]:%8lu\n",
		times, pmc_inst_stop - pmc_inst_start, delta[0], delta[1], delta[2], delta[3], delta[4], delta[5],
        delta[6], delta[7]);
#endif
}


#define FOR_EXCEL_FORMAT 1

int pmc_post_dump_info(char *buf, uint32_t times, uint32_t latency)
{
#if FOR_EXCEL_FORMAT
#if PMC_MAX == 8
	int size = sprintf(buf, "%u\t%u\t%lu\t%lu\t%lu\t%lu\t%lu\t%lu\t%lu\t%lu\t%lu\t%lu\t%lu\n",
		latency, times, pmc_inst_stop - pmc_inst_start, pmc_cycles_stop - pmc_cycles_start,
		pmc_tsc_stop - pmc_tsc_start, delta[0], delta[1], delta[2], delta[3], delta[4], delta[5],
        delta[6], delta[7]);
#else
	int size = sprintf(buf, "%u\t%u\t%lu\t%lu\t%lu\t%lu\t%lu\t%lu\t%lu\n",
		latency, times, pmc_inst_stop - pmc_inst_start, pmc_cycles_stop - pmc_cycles_start,
		pmc_tsc_stop - pmc_tsc_start, delta[0], delta[1], delta[2], delta[3]);
#endif

#else
	int size = sprintf(buf, "latency:%8u times:%8u inst:%8lu [0]:%8lu [1]:%8lu [2]:%8lu [3]:%8lu [4]:%8lu [5]:%8lu "
		"[6]:%8lu [7]:%8lu\n",
		latency, times, pmc_inst_stop - pmc_inst_start, delta[0], delta[1], delta[2], delta[3], delta[4], delta[5],
        delta[6], delta[7]);
#endif
	return size;
}

void pmc_post_start(int cpu)
{
#if PMC_MAX == 4
	wrmsr_on_cpu(PERF_GLOBAL_CTRL, cpu, 0x70000000f); //start 3 fixed and 4 general counter
#else
	wrmsr_on_cpu(PERF_GLOBAL_CTRL, cpu, 0x7000000ff); //start 3 fixed and 8 general counter
#endif
}

void pmc_post_stop(int cpu)
{
	wrmsr_on_cpu(PERF_GLOBAL_CTRL, cpu, 0); //stop all counter
}