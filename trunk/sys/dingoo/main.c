/*
** Dingoo A320 main
** modelled on generic gnuboy main.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <signal.h>

#include "gnuboy.h"
#include "loader.h"
#include "input.h"
#include "rc.h"


#include "Version"


/* Dingoo SDL key mappings are the default, can over ride in gnuboy.rc */
static char *defaultconfig[] =
{
	"bind space quit", /* X button */
	"bind shift quit", /* Y button - LSHIFT*/
	"bind tab quit", /* Left shoulder */
	"bind backspace quit", /* Right shoulder */
	"bind up +up", /*  */
	"bind down +down", /*  */
	"bind left +left", /*  */
	"bind right +right", /*  */
	"bind ctrl +a", /* A button - LEFTCTRL */
	"bind alt +b", /* B button - LEFTALT */
	"bind enter +start", /* START button */
	"bind esc +select", /* SELECT button */
	/*
	"bind 1 \"set saveslot 1\"",
	"bind 2 \"set saveslot 2\"",
	"bind 3 \"set saveslot 3\"",
	"bind 4 \"set saveslot 4\"",
	"bind 5 \"set saveslot 5\"",
	"bind 6 \"set saveslot 6\"",
	"bind 7 \"set saveslot 7\"",
	"bind 8 \"set saveslot 8\"",
	"bind 9 \"set saveslot 9\"",
	"bind 0 \"set saveslot 0\"",
	"bind ins savestate",
	"bind del loadstate",
	*/
	"source gnuboy.rc",
	NULL
};


void doevents()
{
	event_t ev;
	int st;

	ev_poll();
	while (ev_getevent(&ev))
	{
		if (ev.type != EV_PRESS && ev.type != EV_RELEASE)
			continue;
		st = (ev.type != EV_RELEASE);
		rc_dokey(ev.code, st);
	}
}


static void shutdown()
{
	vid_close();
	pcm_close();
}

void die(char *fmt, ...)
{
	va_list ap;
	char tmp_buf[1024];

	va_start(ap, fmt);
	vsnprintf(tmp_buf, sizeof(tmp_buf)-1, fmt, ap);
	fprintf(stderr, fmt, ap); /* NOTE with Oct 2010 Dingoo native SDK this goes to the serial port */
	va_end(ap);
	exit(1);
}

static int bad_signals[] =
{
	/* These are all standard, so no need to #ifdef them... */
	SIGINT, SIGSEGV, SIGTERM, SIGFPE, SIGABRT, SIGILL,
#ifdef SIGQUIT
	SIGQUIT,
#endif
#ifdef SIGPIPE
	SIGPIPE,
#endif
	0
};

static void fatalsignal(int s)
{
	die("Signal %d\n", s);
}

static void catch_signals()
{
	int i;
	for (i = 0; bad_signals[i]; i++)
		signal(bad_signals[i], fatalsignal);
}


int main(int argc, char *argv[])
{
	int i;
	char *opt, *arg, *cmd, *s, *rom = 0;

	/* Avoid initializing video if we don't have to */
	for (i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "--bind")) i += 2;
		else if (!strcmp(argv[i], "--source")) i++;
		else if (!strcmp(argv[i], "--showvars"))
		{
			show_exports(); /* FIXME from exports.c but no prototype (header) */
			exit(0);
		}
		else if (argv[i][0] == '-' && argv[i][1] == '-');
		else if (argv[i][0] == '-' && argv[i][1]);
		else rom = argv[i];
	}
	
	/*
	** TMP nasty hack
	** if no rom name parameter, default to a hard coded rom name
	** Adjustris by Dave VanEe
	** available from http://www.pdroms.de/files/910/
	** TODO 1 - make a .SIM instead of a .APP
	** TODO 2 - add a ROM loading menu item
	*/
	if (argc == 1) rom = "Adjustris.GB"; 
	/*
	if (!rom) usage(base(argv[0]));
	*/
    
	/* If we have special perms, drop them ASAP! */
	vid_preinit();

	init_exports(); /* FIXME from exports.c but no prototype (header) */

	s = strdup(argv[0]);
	sys_sanitize(s);
	sys_initpath(s);

	for (i = 0; defaultconfig[i]; i++)
		rc_command(defaultconfig[i]);

	cmd = malloc(strlen(rom) + 11);
	sprintf(cmd, "source %s", rom);
/* DEBUG extern const char *strchr(const char *inStr, int inChar); */
	s = (char *) strchr(cmd, '.');
	if (s) *s = 0;
	strcat(cmd, ".rc");
	rc_command(cmd);

	for (i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "--bind"))
		{
			if (i + 2 >= argc) die("missing arguments to bind\n");
			cmd = malloc(strlen(argv[i+1]) + strlen(argv[i+2]) + 9);
			sprintf(cmd, "bind %s \"%s\"", argv[i+1], argv[i+2]);
			rc_command(cmd);
			free(cmd);
			i += 2;
		}
		else if (!strcmp(argv[i], "--source"))
		{
			if (i + 1 >= argc) die("missing argument to source\n");
			cmd = malloc(strlen(argv[i+1]) + 6);
			sprintf(cmd, "source %s", argv[++i]);
			rc_command(cmd);
			free(cmd);
		}
		else if (!strncmp(argv[i], "--no-", 5))
		{
			opt = strdup(argv[i]+5);
			while ((s = (char *) strchr(opt, '-'))) *s = '_';
			cmd = malloc(strlen(opt) + 7);
			sprintf(cmd, "set %s 0", opt);
			rc_command(cmd);
			free(cmd);
			free(opt);
		}
		else if (argv[i][0] == '-' && argv[i][1] == '-')
		{
			opt = strdup(argv[i]+2);
			if ((s = (char *) strchr(opt, '=')))
			{
				*s = 0;
				arg = s+1;
			}
			else arg = "1";
			while ((s = (char *) strchr(opt, '-'))) *s = '_';
			while ((s = (char *) strchr(arg, ','))) *s = ' ';
			
			cmd = malloc(strlen(opt) + strlen(arg) + 6);
			sprintf(cmd, "set %s %s", opt, arg);
			
			rc_command(cmd);
			free(cmd);
			free(opt);
		}
	}

	/* FIXME - make interface modules responsible for atexit() */
	atexit(shutdown);
	catch_signals();
	vid_init();
	pcm_init();

	rom = strdup(rom);
	sys_sanitize(rom);
	
	loader_init(rom);
	
	emu_reset();
	emu_run();

	/* never reached */
	return 0;
}
