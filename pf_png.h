#ifndef _PF_PNG_H_
#define _PF_PNG_H_

int pf_png_open(const char *path);
int pf_png_read(void);
int pf_png_close(void);
uint32_t pf_png_get_rgb(uint32_t x, uint32_t y);

#endif /* _PF_PNG_H_ */
