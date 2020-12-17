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

#include "pmc.h"

#define PMC_MAX 8
#define TYPE_FIXED_CTR  		(1ULL << 30)

static uint64_t g_start[PMC_MAX];
static uint64_t g_stop[PMC_MAX];
static uint64_t delta[PMC_MAX];

static uint64_t pmc_inst_start;
static uint64_t pmc_inst_stop;

static inline void native_rdpmc(uint32_t ecx, volatile uint64_t * result)
{
	asm volatile(
		"rdpmc               ;\n\t"
		"mov %%eax, (%0)     ;\n\t"
		"mov %%edx,4(%0)     ;\n\t"
		:
		:"r"(result),"c"(ecx)
		:"eax","edx","memory");
}

void pmc_post_read_start(void)
{
	int cnt;

	native_rdpmc(TYPE_FIXED_CTR, &pmc_inst_start);
	for( cnt = 0; cnt < PMC_MAX; cnt++) {
		native_rdpmc(cnt, &g_start[cnt]);
	}
}

void pmc_post_read_stop(void)
{
	int cnt;

	native_rdpmc(TYPE_FIXED_CTR, &pmc_inst_stop);
	for( cnt = 0; cnt < PMC_MAX; cnt++) {
		native_rdpmc(cnt, &g_stop[cnt]);
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

int pmc_post_dump_info(char *buf, uint32_t times)
{
	int size = sprintf(buf, "times:%8u inst:%8lu [0]:%8lu [1]:%8lu [2]:%8lu [3]:%8lu [4]:%8lu [5]:%8lu [6]:%8lu [7]:%8lu\n",
		times, pmc_inst_stop - pmc_inst_start, delta[0], delta[1], delta[2], delta[3], delta[4], delta[5],
        delta[6], delta[7]);

	return size;
}