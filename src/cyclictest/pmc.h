#if !defined(__PMC_H__)
#define __PMC_H__

bool pmc_init(int cpu);

void pmc_start(int cpu);
void pmc_stop(int cpu);

void pmc_report(FILE *fd, uint32_t times);

uint64_t pmc_get_cache_refs(void);
uint64_t pmc_get_cache_misses(void);

#define USE_POST_PMC 1
void pmc_post_read_start(void);
void pmc_post_read_stop(void);

void pmc_post_report(FILE *fd, uint32_t times);
int pmc_post_dump_info(char *buf, uint32_t times);

#endif