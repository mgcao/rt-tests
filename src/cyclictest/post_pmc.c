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

#define PMC_MAX 8
#define TYPE_FIXED_CTR  		(1ULL << 30)
#define PERF_GLOBAL_CTRL		0x38f /* Global CTRL MSR */

static uint64_t g_start[PMC_MAX];
static uint64_t g_stop[PMC_MAX];
static uint64_t delta[PMC_MAX];

static uint64_t pmc_inst_start;
static uint64_t pmc_inst_stop;


static inline uint64_t native_rdpmc(uint32_t idx)
{
    uint32_t lo, hi;

    __asm__ volatile("rdpmc" : "=a" (lo), "=d" (hi) : "c" (idx));

    return ((uint64_t)lo) | (((uint64_t)hi) << 32);
}

void pmc_post_read_start(void)
{
	int cnt;

	pmc_inst_start = native_rdpmc(TYPE_FIXED_CTR);
	for( cnt = 0; cnt < PMC_MAX; cnt++) {
		g_start[cnt] = native_rdpmc(cnt);
	}
}

void pmc_post_read_stop(void)
{
	int cnt;

	pmc_inst_stop = native_rdpmc(TYPE_FIXED_CTR);
	for( cnt = 0; cnt < PMC_MAX; cnt++) {
		g_stop[cnt] = native_rdpmc(cnt);
	}

    for( cnt = 0; cnt < PMC_MAX; cnt++) {
        delta[cnt] = g_stop[cnt] - g_start[cnt];
    }

}

void pmc_post_report(FILE *fd, uint32_t times)
{
	fprintf(fd, "times:%8u inst:%8lu [0]:%8lu [1]:%8lu [2]:%8lu [3]:%8lu [4]:%8lu [5]:%8lu [6]:%8lu [7]:%8lu\n",
		times, pmc_inst_stop - pmc_inst_start, delta[0], delta[1], delta[2], delta[3], delta[4], delta[5],
        delta[6], delta[7]);
}

int pmc_post_dump_info(char *buf, uint32_t times, uint32_t latency)
{
	int size = sprintf(buf, "latency:%8u times:%8u inst:%8lu [0]:%8lu [1]:%8lu [2]:%8lu [3]:%8lu [4]:%8lu [5]:%8lu "
		"[6]:%8lu [7]:%8lu\n",
		latency, times, pmc_inst_stop - pmc_inst_start, delta[0], delta[1], delta[2], delta[3], delta[4], delta[5],
        delta[6], delta[7]);

	return size;
}

void pmc_post_start(int cpu)
{
	wrmsr_on_cpu(PERF_GLOBAL_CTRL, cpu, 0x7000000ff); //start 3 fixed and 8 general counter
}

void pmc_post_stop(int cpu)
{
	wrmsr_on_cpu(PERF_GLOBAL_CTRL, cpu, 0); //stop all counter
}