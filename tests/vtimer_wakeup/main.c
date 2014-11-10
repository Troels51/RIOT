/*
 * Copyright (C) 2013 INRIA
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup tests
 * @{
 *
 * @file
 * @brief Thread test application
 *
 * @author      Oliver Hahm <oliver.hahm@inria.fr>
 * @author      Christian Mehlis <mehlis@inf.fu-berlin.de>
 *
 * @}
 */

#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include "vtimer.h"
#include "thread.h"
#include "msg.h"

char timer_stack[KERNEL_CONF_STACKSIZE_MAIN];

vtimer_t wakeup_timer;
timex_t  interval;
void *timer_thread(void *arg)
{
    (void) arg;

    printf("This is thread %d" PRIkernel_pid "\n", thread_getpid());
    interval = timex_set(5,0);

    while (1)
    {
        //vtimer_remove(&wakeup_timer);
        //vtimer_set_wakeup(&wakeup_timer, interval, thread_getpid());
        //thread_sleep();
        vtimer_usleep(5000000);
        printf("I'm awake now \n");

    }
}


int main(void)
{
    kernel_pid_t pid = thread_create(
                  timer_stack,
                  sizeof(timer_stack),
                  PRIORITY_MAIN - 1,
                  CREATE_STACKTEST,
                  timer_thread,
                  NULL,
                  "timer");


    while (1) {
      
    }
}
