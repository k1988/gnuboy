#ifndef __GNUBOY_H__
#define __GNUBOY_H__

void ev_poll();
void vid_close();
void pcm_close();
void vid_preinit();
void vid_init();
void vid_setpal(int i, int r, int g, int b);
void vid_settitle(char *title);

/* Sound */
void pcm_init();
int pcm_submit();

void sys_checkdir(char *path, int wr);
void sys_sanitize(char *s);
void sys_initpath(char *exe);
void die(char *fmt, ...);

/* FIXME bad place for these prototypes */
/*------------------------------------------*/
void emu_reset(); /* emu.c */
void emu_run(); /* emu.c */

void init_exports(); /* exports.c  */
void show_exports(); /* exports.c  */

#include "defs.h" /* need byte for below */
void hw_interrupt(byte i, byte mask); /* hw.c */

void pal_set332(); /* palette.c */
void pal_expire(); /* palette.c */
void pal_release(byte n); /* palette.c */
byte pal_getcolor(int c, int r, int g, int b); /* palette.c */

#include <stdio.h> /* need FILE for below */
void savestate(FILE *f); /* save.c */
void loadstate(FILE *f); /* save.c */

int unzip (const unsigned char *data, long *p, void (* callback) (unsigned char d)); /* inflate.c */
/*------------------------------------------*/

#endif /* __GNUBOY_H__ */
