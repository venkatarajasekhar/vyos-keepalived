/*
 * Soft:        Keepalived is a failover program for the LVS project
 *              <www.linuxvirtualserver.org>. It monitor & manipulate
 *              a loadbalanced server pool using multi-layer checks.
 *
 * Part:        scheduler.c include file.
 *
 * Author:      Alexandre Cassen, <acassen@linux-vs.org>
 *
 *              This program is distributed in the hope that it will be useful,
 *              but WITHOUT ANY WARRANTY; without even the implied warranty of
 *              MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *              See the GNU General Public License for more details.
 *
 *              This program is free software; you can redistribute it and/or
 *              modify it under the terms of the GNU General Public License
 *              as published by the Free Software Foundation; either version
 *              2 of the License, or (at your option) any later version.
 *
 * Copyright (C) 2001-2010 Alexandre Cassen, <acassen@freebox.fr>
 */

#ifndef _SCHEDULER_H
#define _SCHEDULER_H

/* system includes */
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <assert.h>
#include <fcntl.h>
#include <errno.h>
#include <syslog.h>
#include "timer.h"

/* Thread itself. */
typedef struct _thread {
	unsigned long id;
	unsigned char type;		/* thread type */
	struct _thread *next;		/* next pointer of the thread */
	struct _thread *prev;		/* previous pointer of the thread */
	struct _thread_master *master;	/* pointer to the struct thread_master. */
	int (*func) (struct _thread *);	/* event function */
	void *arg;			/* event argument */
	TIMEVAL sands;			/* rest of time sands value. */
	union {
		int val;		/* second argument of the event. */
		int fd;			/* file descriptor in case of read/write. */
		struct {
			pid_t pid;	/* process id a child thread is wanting. */
			int status;	/* return status of the process */
		} c;
	} u;
} thread;

/* Linked list of thread. */
typedef struct _thread_list {
	thread *head;
	thread *tail;
	int count;
} thread_list;

/* Master of the theads. */
typedef struct _thread_master {
	thread_list read;
	thread_list write;
	thread_list timer;
	thread_list child;
	thread_list event;
	thread_list ready;
	thread_list unuse;
	fd_set readfd;
	fd_set writefd;
	fd_set exceptfd;
	unsigned long alloc;
} thread_master;

/* Thread types. */
#define THREAD_READ		0
#define THREAD_WRITE		1
#define THREAD_TIMER		2
#define THREAD_EVENT		3
#define THREAD_CHILD		4
#define THREAD_READY		5
#define THREAD_UNUSED		6
#define THREAD_WRITE_TIMEOUT	7
#define THREAD_READ_TIMEOUT	8
#define THREAD_CHILD_TIMEOUT	9
#define THREAD_TERMINATE	10
#define THREAD_READY_FD		11

/* MICRO SEC def */
#define BOOTSTRAP_DELAY TIMER_HZ
#define RESPAWN_TIMER	60*TIMER_HZ

/* Macros. */
#define THREAD_ARG(X) ((X)->arg)
#define THREAD_FD(X)  ((X)->u.fd)
#define THREAD_VAL(X) ((X)->u.val)
#define THREAD_CHILD_PID(X) ((X)->u.c.pid)
#define THREAD_CHILD_STATUS(X) ((X)->u.c.status)

/* global vars exported */
extern thread_master *master;

/* Prototypes. */
extern thread_master *thread_make_master(void);
extern thread *thread_add_terminate_event(thread_master * m);
extern void thread_destroy_master(thread_master * m);
extern thread *thread_add_read(thread_master * m, int (*func) (thread *)
			       , void *arg, int fd, long timeout);
extern thread *thread_add_write(thread_master * m, int (*func) (thread *)
				, void *arg, int fd, long timeout);
extern thread *thread_add_timer(thread_master * m, int (*func) (thread *)
				, void *arg, long timer);
extern thread *thread_add_child(thread_master * m, int (*func) (thread *)
				, void *arg, pid_t pid, long timeout);
extern thread *thread_add_event(thread_master * m, int (*func) (thread *)
				, void *arg, int val);
extern void thread_cancel(thread * thread_obj);
extern void thread_cancel_event(thread_master * m, void *arg);
extern thread *thread_fetch(thread_master * m, thread * fetch);
extern void thread_child_handler(void * v, int sig);
extern void thread_call(thread * thread_obj);
extern void launch_scheduler(void);

#endif
