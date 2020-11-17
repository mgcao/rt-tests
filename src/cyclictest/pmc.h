#if !defined(__PMC_H__)
#define __PMC_H__

bool pmc_init(int cpu);

void pmc_start(int cpu);
void pmc_stop(int cpu);

void pmc_report(FILE *fd, uint32_t times);

uint64_t pmc_get_cache_refs(void);
uint64_t pmc_get_cache_misses(void);

#endif