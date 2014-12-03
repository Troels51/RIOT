/**
 * 6lowpan border router implementation
 *
 * Copyright (C) 2013  INRIA.
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 *
 * @ingroup sixlowpan
 * @{
 * @file    sixlowborder.c
 * @brief   constraint node implementation for a 6lowpan border router
 * @author  Martin Lenders <mlenders@inf.fu-berlin.de>
 * @author  Oliver Hahm <oliver.hahm@inria.fr>
 * @}
 */

/* TODO: Put this in its own module */
#include <stdint.h>

#include "thread.h"
#define READER_STACK_SIZE   (KERNEL_CONF_STACKSIZE_DEFAULT)

char serial_reader_stack[READER_STACK_SIZE];
kernel_pid_t serial_reader_pid = KERNEL_PID_UNDEF;



