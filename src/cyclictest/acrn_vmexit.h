#if !defined(__ACRN_VMEXIT_H__)
#define __ACRN_VMEXIT_H__

bool acrn_vmexit_init(void);
void acrn_vmexit_deinit(void);

void acrn_vmexit_begin(void);
bool acrn_vmexit_end(void);
void acrn_vmexit_dump(FILE *fd);

int acrn_vmexit_copy_dump_info(char *buf);

#endif