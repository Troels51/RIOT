/*
 * Copyright (C) 2014 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     cpu_samd21
 * @{
 *
 * @file        lpm_arch.c
 * @brief       Implementation of the kernels power management interface
 *
 * @author      Thomas Eichinger <thomas.eichinger@fu-berlin.de>
 *
 * @}
 */

#include "arch/lpm_arch.h"
#include "cpu.h"
#define ENABLE_DEBUG (1)
#include "debug.h"
void lpm_arch_init(void)
{
    lpm_awake();
}

enum lpm_mode lpm_arch_set(enum lpm_mode target)
{
	//DEBUG("LPM: %d \n", target);
	switch(target)
	{
	case LPM_ON:
		break;
	case LPM_IDLE:
		PM->SLEEP.bit.IDLE = 3;
		break;
	case LPM_SLEEP:
		PM->SLEEP.bit.IDLE = 4;
		break;
	case LPM_POWERDOWN:
		break;
	case LPM_OFF:
		break;

	}
	//se__WFI(); //Wait for interrupt
    return target;
}

enum lpm_mode lpm_arch_get(void)
{
    uint32_t mode = PM->SLEEP.bit.IDLE;
    switch(mode)
    {
    	case 1:
    		return LPM_IDLE;
    	case 4:
    		return LPM_SLEEP;
    	default:
    		return 0;
    }
}

void lpm_arch_awake(void)
{
	PM->SLEEP.bit.IDLE = 0;
}

void lpm_arch_begin_awake(void)
{
    // TODO
}

void lpm_arch_end_awake(void)
{
    // TODO
}
