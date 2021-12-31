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
#include <cpuid.h>

#include "msr.h"

#include "uncore.h"

#define MSR_UNC_PERF_GLOBAL_CTRL  0xE01
#define MSR_UNC_ARB_PERFCTR0_0  0x3B0
#define MSR_UNC_ARB_PERFCTR0_1  0x3B1
#define MSR_UNC_PERF_FIXED_CTR  0x395

#define GROUP_COUNTERS 4

//CBox 0-3, counter0-1, calc total values
static const uint32_t group_0_counters[GROUP_COUNTERS] = {0x706, 0x716, 0x726, 0x736};
static const uint32_t group_1_counters[GROUP_COUNTERS] = {0x707, 0x717, 0x727, 0x737};

//total calc: group: 2, arb: 2, fixed: 1, 5 kinds of data
#define UNC_DATA_TYPES 5
static uint64_t g_unc_start[UNC_DATA_TYPES];
static uint64_t g_unc_stop[UNC_DATA_TYPES];
static uint64_t g_unc_delta[UNC_DATA_TYPES];
static int target_cpu;

#define read_uncore_msr(reg)  rdmsr_on_cpu(reg, target_cpu)
#define write_uncore_msr(reg, regval)   wrmsr_on_cpu(reg, target_cpu, regval);

bool uncore_init(int cpu)
{
    // it shall read 0x396 MSR_UNC_CBO_CONFIG, now just return true

    target_cpu = cpu;
    return true;
}

void uncore_start_counting(void)
{
    write_uncore_msr(MSR_UNC_PERF_GLOBAL_CTRL, 0x20000000);
}

void uncore_stop_counting(void)
{
    write_uncore_msr(MSR_UNC_PERF_GLOBAL_CTRL, 0);
}

void uncore_pre_read(void)
{
 //   uncore_start_counting();

    g_unc_start[0] = read_uncore_msr(group_0_counters[0]);
    g_unc_start[1] = read_uncore_msr(group_1_counters[0]);

    for (int i = 1; i < GROUP_COUNTERS; i++) {
        g_unc_start[0] += read_uncore_msr(group_0_counters[i]);
        g_unc_start[1] += read_uncore_msr(group_1_counters[i]);
    }

    g_unc_start[2] = read_uncore_msr(MSR_UNC_ARB_PERFCTR0_0);
    g_unc_start[3] = read_uncore_msr(MSR_UNC_ARB_PERFCTR0_1);

    g_unc_start[4] = read_uncore_msr(MSR_UNC_PERF_FIXED_CTR);

}

void uncore_post_read(void)
{
 //   uncore_stop_counting();

    g_unc_stop[0] = read_uncore_msr(group_0_counters[0]);
    g_unc_stop[1] = read_uncore_msr(group_1_counters[0]);

    for (int i = 1; i < GROUP_COUNTERS; i++) {
        g_unc_stop[0] += read_uncore_msr(group_0_counters[i]);
        g_unc_stop[1] += read_uncore_msr(group_1_counters[i]);
    }

    g_unc_stop[2] = read_uncore_msr(MSR_UNC_ARB_PERFCTR0_0);
    g_unc_stop[3] = read_uncore_msr(MSR_UNC_ARB_PERFCTR0_1);

    g_unc_stop[4] = read_uncore_msr(MSR_UNC_PERF_FIXED_CTR);

    for (int i = 0; i < UNC_DATA_TYPES; i++) {
        g_unc_delta[i] = g_unc_stop[i] - g_unc_start[i];
    }

}

int uncore_dump_delta_info(char *buf)
{
    int size = 0;

    for (int i = 0; i < UNC_DATA_TYPES; i++) {
		size += sprintf(buf + size, "\t%lu", g_unc_delta[i]);
	}

    return size;
}
