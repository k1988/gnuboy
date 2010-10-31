#ifndef __GNUBOY_H__
#define __GNUBOY_H__

void ev_poll();
void vid_close();
void pcm_close();
void vid_preinit();
void vid_init();

/* Sound */
void pcm_init();

void sys_sanitize(char *s);
void sys_initpath(char *exe);
void die(char *fmt, ...);

/* FIXME bad place for these prototypes */
void emu_reset(); /* emu.c */
void emu_run(); /* emu.c */
void init_exports(); /* exports.c  */
void show_exports(); /* exports.c  */

#endif /* __GNUBOY_H__ */
