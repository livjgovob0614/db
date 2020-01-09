/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2000, 2011 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */
#include "db_config.h"
#include "db_int.h"
#pragma hdrstop

static int interrupt;
static void set_signal __P((int, int));
static void signal_handler(int);
/*
 * signal_handler --
 *	Interrupt signal handler.
 */
static void signal_handler(int signo)
{
#ifndef HAVE_SIGACTION
	/* Assume signal() is unreliable and reset it, first thing. */
	set_signal(signo, 0);
#endif
	/* Some systems don't pass in the correct signal value -- check. */
	if((interrupt = signo) == 0)
		interrupt = SIGINT;
}
/*
 * set_signal
 */
static void set_signal(int s, int is_dflt)
{
	/*
	 * Use sigaction if it's available, otherwise use signal().
	 */
#ifdef HAVE_SIGACTION
	struct sigaction sa, osa;
	sa.sa_handler = is_dflt ? SIG_DFL : signal_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(s, &sa, &osa);
#else
	signal(s, is_dflt ? SIG_DFL : signal_handler);
#endif
}
/*
 * __db_util_siginit --
 */
void __db_util_siginit()
{
	/*
	 * Initialize the set of signals for which we want to clean up.
	 * Generally, we try not to leave the shared regions locked if
	 * we can.
	 */
#ifdef SIGHUP
	set_signal(SIGHUP, 0);
#endif
#ifdef SIGINT
	set_signal(SIGINT, 0);
#endif
#ifdef SIGPIPE
	set_signal(SIGPIPE, 0);
#endif
#ifdef SIGTERM
	set_signal(SIGTERM, 0);
#endif
}
/*
 * __db_util_interrupted --
 *	Return if interrupted.
 */
int __db_util_interrupted()
{
	return interrupt != 0;
}
/*
 * __db_util_sigresend --
 */
void __db_util_sigresend()
{
	/* Resend any caught signal. */
	if(interrupt != 0) {
		set_signal(interrupt, 1);
		raise(interrupt);
		/* NOTREACHED */
	}
}

