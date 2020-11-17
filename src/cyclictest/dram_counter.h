#if !defined(__DRAM_COUNTER_H__)
#define __DRAM_COUNTER_H__

bool dram_mon_init(void);
void dram_mon_deinit(void);

void dram_mon_start(void);
void dram_mon_stop(void);
void dram_mon_dump(FILE *fd);
int dram_copy_dump_info(char *buf);

#endif