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

/* FIXME this header files is a poor location for the following prototypes */
/*------------------------------------------*/

/* emu.c */
void emu_reset();
void emu_run();

 /* exports.c */
void init_exports();
void show_exports();

/* hw.c */
#include "defs.h" /* need byte for below */
void hw_interrupt(byte i, byte mask);

/* palette.c */
void pal_set332();
void pal_expire();
void pal_release(byte n);
byte pal_getcolor(int c, int r, int g, int b);

/* save.c */
#include <stdio.h> /* need FILE for below */
void savestate(FILE *f);
void loadstate(FILE *f);

/* inflate.c */
int unzip (const unsigned char *data, long *p, void (* callback) (unsigned char d));

/* split.c */
int splitline(char **argv, int max, char *line);

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

/* debug.c */
void debug_disassemble(addr a, int c);

/*------------------------------------------*/

#endif /* __GNUBOY_H__ */