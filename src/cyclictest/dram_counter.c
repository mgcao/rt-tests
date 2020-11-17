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

#include "dram_counter.h"

//for memory bandwidth info read from hostbridge

#define DEBUG 0

#if DEBUG
	#define dbg_info(...) printf(__VA_ARGS__)
#else
	#define dbg_info(...)
#endif

static int dev_mem_handle;
static char* mem_mon_vaddr;

static volatile uint32_t* mem_bw_mon[5];

static uint64_t mem_bw_start[5];
static uint64_t mem_bw_stop[5];
static uint64_t mem_bw_kbyte[5];

#define DRAM_MON_GPA_BASE 0xFED15000  //BAR + 0x5000 as base to map

bool dram_mon_init(void)
{
	dev_mem_handle = open("/dev/mem", O_RDONLY);

	dbg_info("dram_mon_init: open /dev/mem: %d!\n", dev_mem_handle);

	if (dev_mem_handle < 0) {
		fprintf(stderr, "open /dev/mem failed, please check kernel config! err: %s\n", strerror(errno));
		return false;
	}

	mem_mon_vaddr = (char *) mmap(NULL, 4096, PROT_READ, MAP_SHARED, dev_mem_handle, DRAM_MON_GPA_BASE);

	mem_bw_mon[0] =  (uint32_t *)(mem_mon_vaddr + 0x40);
	mem_bw_mon[1] =  (uint32_t *)(mem_mon_vaddr + 0x44);
	mem_bw_mon[2] =  (uint32_t *)(mem_mon_vaddr + 0x48);
	mem_bw_mon[3] =  (uint32_t *)(mem_mon_vaddr + 0x50);
	mem_bw_mon[4] =  (uint32_t *)(mem_mon_vaddr + 0x54);

	return true;
}

void dram_mon_deinit(void)
{
	munmap(mem_mon_vaddr, 4096);
	close(dev_mem_handle);
}

void dram_mon_start(void)
{
	int cnt;

	for( cnt = 0; cnt < 5; cnt++) {
		mem_bw_start[cnt] = *mem_bw_mon[cnt];
	}
}

void dram_mon_stop(void)
{
	int cnt;

	for( cnt = 0; cnt < 5; cnt++) {
		mem_bw_stop[cnt] = *mem_bw_mon[cnt];
	}

	for( cnt = 0; cnt < 5; cnt++) {
		if (mem_bw_stop[cnt] < mem_bw_start[cnt]) {
			mem_bw_kbyte[cnt]  = (mem_bw_stop[cnt] + (1UL << 32U) - mem_bw_start[cnt]) * 64 / 1024;
		} else {
			mem_bw_kbyte[cnt] = (mem_bw_stop[cnt] - mem_bw_start[cnt]) * 64 / 1024;
		}
	}
}

void dram_mon_dump(FILE *fd)
{
	fprintf(fd, "DRAM: GPU %8lu KB  CPU %8lu KB  IO %8lu KB  All-Read %8lu KB All-Write %8lu KB\n",
		mem_bw_kbyte[0],
		mem_bw_kbyte[1],
		mem_bw_kbyte[2],
		mem_bw_kbyte[3], 
        mem_bw_kbyte[4]);
}

int dram_copy_dump_info(char *buf)
{
	int size = sprintf(buf, "DRAM(KB): GPU:%lu CPU:%lu IO:%lu\n",
		mem_bw_kbyte[0],
		mem_bw_kbyte[1],
		mem_bw_kbyte[2]);

	return size;
}