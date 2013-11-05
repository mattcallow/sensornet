#ifndef _TASK_H_
#define _TASK_H_

#include <stdio.h>
#if defined(ARDUINO)
#include "Arduino.h"
#elif defined(__AVR__)
#include <avr/wdt.h>
#elif defined(_PIC18)
#include <xc.h>
#else
#error unsupported system type
#endif

#include "task.h"
#include "sensornet.h"
#include "debug.h"
#include "config.h"

typedef void (*task_fn_t)(uint8_t);
typedef unsigned long task_next_t;
typedef unsigned long task_period_t;
typedef uint8_t task_data_t;

typedef struct {
	task_next_t 	next;
	task_period_t 	period;
	task_fn_t		fn;
	task_data_t 	data;
} task_list_t;

extern uint8_t num_tasks;
extern task_list_t task_list[];

#define TASK_LIST_START task_list_t task_list[] = {
#define TASK(s,p,f,d) { s, p, f, d }
#define TASK_LIST_END }; \
uint8_t num_tasks = sizeof(task_list)/sizeof(task_list[0]);

static void inline task_dispatch(void)
{
	for (int i=0;i<num_tasks;i++) {
#ifdef SERIAL_DEBUG
		DBG_STRP_U8H_NL("Timer:", i);
		DBG_STRP_U32H_NL("\tnext=", task_list[i].next);
		DBG_STRP_U32H_NL("\tperiod=", task_list[i].period);
#endif
		if (task_list[i].next == 0 && task_list[i].period == 0) continue;
#ifdef ARDUINO
		if (millis() >= task_list[i].next) { 
#else
		if (seconds() >= task_list[i].next) { 
#endif
			if (task_list[i].period == 0) task_list[i].next=0; // disable task
			else task_list[i].next += task_list[i].period;
			task_list[i].fn(task_list[i].data);	
		}
	}
}

#ifndef ARDUINO
static void inline task_loop(void)
{
	for(;;) {
		task_dispatch();
#ifdef USE_WDT
		wd_sleep();
#else
#error No sleep method defined
#endif
	}
}
#endif

#endif // _TASK_H_

// vim: ts=4 sw=4
