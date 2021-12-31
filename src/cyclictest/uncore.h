#if !defined(__UNCORE_H__)
#define __UNCORE_H__

bool uncore_init(int cpu);

void uncore_start_counting(void);
void uncore_stop_counting(void);

void uncore_pre_read(void);
void uncore_post_read(void);

int uncore_dump_delta_info(char *buf);

#endif