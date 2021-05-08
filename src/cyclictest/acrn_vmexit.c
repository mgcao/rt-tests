/*
 *
 * (C) 2021      Minggui Cao <minggui.cao@intel.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License Version
 * 2 as published by the Free Software Foundation.
 *
 */

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

#include "acrn_vmexit.h"

#define DEBUG 0

#if DEBUG
	#define dbg_info(...) printf(__VA_ARGS__)
#else
	#define dbg_info(...)
#endif

static volatile uint64_t* vmexit_magic_flag;

static volatile uint64_t* vmexit_cnt;
static volatile uint64_t* vmexit_reason;
static volatile uint64_t* vmexit_data;
static volatile uint64_t* vmexit_tsc_start;
static volatile uint64_t* vmexit_tsc_stop;

static uint64_t vmexit_cnt_start;
static uint64_t vmexit_cnt_stop;

static uint64_t vmexit_info[5];

static int dev_mem_handle;
static uint64_t* vmexit_vaddr;

#define ACRN_MAGIC_FLAG   (uint64_t)0x4143524E45584954 //ACRNEXIT
#define ACRN_VMEXIT_GPA_BASE 0x100000

bool acrn_vmexit_init(void)
{
	dev_mem_handle = open("/dev/mem", O_RDONLY);

	dbg_info("vmexit-init: open /dev/mem: %d!\n", dev_mem_handle);

	if (dev_mem_handle < 0) {
		fprintf(stderr, "open /dev/mem failed, please check kernel config! err: %s\n", strerror(errno));
		return false;
	}

	vmexit_vaddr = (uint64_t *)mmap(NULL, 4096, PROT_READ, MAP_SHARED, dev_mem_handle, ACRN_VMEXIT_GPA_BASE);

	printf("ACRN VMEXIT base, GPA=0x%x, map to: 0x%p\n", ACRN_VMEXIT_GPA_BASE, vmexit_vaddr);

	if (((uint64_t)vmexit_vaddr & 0xFFF) != 0) {
		printf("mmap failed! can't support vmexit sampling!\n");
		return false;
	}
	vmexit_magic_flag = vmexit_vaddr + 5;

	if (*vmexit_magic_flag == ACRN_MAGIC_FLAG) {
		printf("ACRN-VMEXIT magic flag: 0x%lx matched!\n", ACRN_MAGIC_FLAG);
	} else {
		printf("ACRN-VMEXIT not supported! magic flag not matched, read: 0x%lx, shall be 0x%lx!\n", *vmexit_magic_flag, ACRN_MAGIC_FLAG);
		return false;
	}

	vmexit_cnt = vmexit_vaddr + 0;
	vmexit_reason = vmexit_vaddr + 1;
	vmexit_data = vmexit_vaddr + 2;
	vmexit_tsc_start = vmexit_vaddr + 3;
	vmexit_tsc_stop = vmexit_vaddr + 4;

	return true;
}

void acrn_vmexit_deinit(void)
{
	munmap(vmexit_vaddr, 4096);
	close(dev_mem_handle);
}

void acrn_vmexit_begin(void)
{
	dbg_info("vmexit begin\n");
	vmexit_cnt_start = *vmexit_cnt;
}

bool acrn_vmexit_end(void)
{
	dbg_info("vmexit end\n");
	vmexit_cnt_stop = *vmexit_cnt;	

	if (vmexit_cnt_stop > vmexit_cnt_start) {

		vmexit_info[0] = vmexit_cnt_stop - vmexit_cnt_start;
		vmexit_info[1] = *vmexit_reason;
		vmexit_info[2] = *vmexit_data;
		vmexit_info[3] = *vmexit_tsc_start;
		vmexit_info[4] = *vmexit_tsc_stop;

		return true;
	}

	return false;
}

void acrn_vmexit_dump(FILE *fd)
{
	fprintf(fd, "vmexit: cnt delta:%lu (%lu - %lu), last one: 0x%02lx, data: 0x%lx  tsc delta: %lu\n",
			vmexit_info[0],  vmexit_cnt_stop, vmexit_cnt_start,
			vmexit_info[1], vmexit_info[2], vmexit_info[4] - vmexit_info[3]);
}

int acrn_vmexit_copy_dump_info(char *buf)
{
	int size = sprintf(buf, "vmexit: cnt delta:%lu (%lu - %lu), last one: 0x%02lx, data: 0x%lx  tsc delta: %lu\n",
			vmexit_info[0],  vmexit_cnt_stop, vmexit_cnt_start,
			vmexit_info[1], vmexit_info[2], vmexit_info[4] - vmexit_info[3]);

	return size;
}