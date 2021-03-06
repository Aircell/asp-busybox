/* vi: set sw=4 ts=4: */
/*
 * Mini watchdog implementation for busybox
 *
 * Copyright (C) 2003  Paul Mundt <lethal@linux-sh.org>
 * Copyright (C) 2006  Bernhard Reutner-Fischer <busybox@busybox.net>
 * Copyright (C) 2008  Darius Augulis <augulis.darius@gmail.com>
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */

//usage:#define watchdog_trivial_usage
//usage:       "[-t N[ms]] [-T N[ms]] [-F] DEV"
//usage:#define watchdog_full_usage "\n\n"
//usage:       "Periodically write to watchdog device DEV\n"
//usage:     "\n	-T N	Reboot after N seconds if not reset (default 60)"
//usage:     "\n	-t N	Reset every N seconds (default 30)"
//usage:     "\n	-F	Run in foreground"
//usage:     "\n"
//usage:     "\nUse 500ms to specify period in milliseconds"

#include "libbb.h"
#include "linux/types.h" /* for __u32 */
#include "linux/watchdog.h"

#define OPT_DELAY		(1 << 0)
#define OPT_FOREGROUND  (1 << 1)
#define OPT_STIMER      (1 << 2)
#define OPT_HTIMER      (1 << 3)

static void watchdog_shutdown(int sig)
{
	int x = 0;
	if ( sig == SIGUSR1 )
		x = 0;
	else
		x = 1;
	sync();
	usleep(100 * 1000);
	ioctl_or_warn(3, WDIOC_SETTIMEOUT, &x);
	//write(3, &V, 1);  /* Magic, see watchdog-api.txt in kernel */
	if (ENABLE_FEATURE_CLEAN_UP)
		close(3);
	_exit(EXIT_SUCCESS);
}

int watchdog_main(int argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
int watchdog_main(int argc, char **argv)
{
	unsigned opts;
	unsigned delay_duration; /* how long to wait before starting - sec */
	unsigned stimer_duration; /* how often to restart */
	unsigned htimer_duration = 30; /* reboots after Ns if not restarted */
	char *d_arg;
	char *st_arg;
	char *ht_arg;

	opt_complementary = "=1"; /* must have exactly 1 argument */
	opts = getopt32(argv, "D:Ft:T:", &d_arg,&st_arg, &ht_arg);

	/* We need to daemonize *before* opening the watchdog as many drivers
	 * will only allow one process at a time to do so.  Since daemonizing
	 * is not perfect (child may run before parent finishes exiting), we
	 * can't rely on parent exiting before us (let alone *cleanly* releasing
	 * the watchdog fd -- something else that may not even be allowed).
	 */

	delay_duration = 0;
	if ( opts & OPT_DELAY )
		delay_duration = atoi(d_arg);
	if (!(opts & OPT_FOREGROUND))
		bb_daemonize_or_rexec(DAEMON_CHDIR_ROOT, argv);
	if (opts & OPT_HTIMER)
		htimer_duration = atoi(ht_arg);
	stimer_duration = htimer_duration / 4;
	if (opts & OPT_STIMER)
		stimer_duration = atoi(st_arg);

	bb_signals(BB_FATAL_SIGS, watchdog_shutdown);

	sleep(delay_duration);

	/* Use known fd # - avoid needing global 'int fd' */
	xmove_fd(xopen(argv[argc - 1], O_WRONLY), 3);

	/* WDIOC_SETTIMEOUT takes seconds, not milliseconds */
	htimer_duration = htimer_duration;
#ifndef WDIOC_SETTIMEOUT
# error WDIOC_SETTIMEOUT is not defined, cannot compile watchdog applet
#else
# if defined WDIOC_SETOPTIONS && defined WDIOS_ENABLECARD
	{
		static const int enable = WDIOS_ENABLECARD;
		ioctl_or_warn(3, WDIOC_SETOPTIONS, (void*) &enable);
	}
# endif
	ioctl_or_warn(3, WDIOC_SETTIMEOUT, &htimer_duration);
#endif

#if 0
	ioctl_or_warn(3, WDIOC_GETTIMEOUT, &htimer_duration);
	printf("watchdog: SW timer is %dms, HW timer is %ds\n",
		stimer_duration, htimer_duration * 1000);
#endif

	while (1) {
		/*
		 * Make sure we clear the counter before sleeping,
		 * as the counter value is undefined at this point -- PFM
		 */
		write(3, "", 1); /* write zero byte */
		sleep(stimer_duration);
	}
	return EXIT_SUCCESS; /* - not reached, but gcc 4.2.1 is too dumb! */
}
