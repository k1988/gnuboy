#ifndef __GNUBOY_H__
#define __GNUBOY_H__

void ev_poll();
void vid_close();
void pcm_close();
void vid_preinit();
void vid_init();
void vid_begin();
void vid_end();
void vid_setpal(int i, int r, int g, int b);
void vid_settitle(char *title);

void sys_sleep(int us);
void *sys_timer();
int sys_elapsed(void *in_ptr);

/* Sound */
void pcm_init();
int pcm_submit();

void sys_checkdir(char *path, int wr);
void sys_sanitize(char *s);
void sys_initpath(char *exe);
void doevents();
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

int splitline(char **argv, int max, char *line); /* split.c */

/* refresh.c */
void refresh_1(byte *dest, byte *src, byte *pal, int cnt);
void refresh_2(un16 *dest, byte *src, un16 *pal, int cnt);
void refresh_3(byte *dest, byte *src, un32 *pal, int cnt);
void refresh_4(un32 *dest, byte *src, un32 *pal, int cnt);
void refresh_2_3x(un16 *dest, byte *src, un16 *pal, int cnt);
void refresh_3_2x(byte *dest, byte *src, un32 *pal, int cnt);
void refresh_3_3x(byte *dest, byte *src, un32 *pal, int cnt);
void refresh_3_4x(byte *dest, byte *src, un32 *pal, int cnt);
void refresh_4_2x(un32 *dest, byte *src, un32 *pal, int cnt);
void refresh_4_3x(un32 *dest, byte *src, un32 *pal, int cnt);
void refresh_4_4x(un32 *dest, byte *src, un32 *pal, int cnt);

/*------------------------------------------*/

#endif /* __GNUBOY_H__ */
