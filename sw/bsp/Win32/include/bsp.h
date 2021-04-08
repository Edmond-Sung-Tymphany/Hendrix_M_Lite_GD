/*****************************************************************************
* Product: DPP example
* Last Updated for Version: 4.5.04
* Date of the Last Update:  Jun 22, 2013
*
*                    Q u a n t u m     L e a P s
*                    ---------------------------
*                    innovating embedded systems
*
* Copyright (C) 2002-2013 Quantum Leaps, LLC. All rights reserved.
*
* This program is open source software: you can redistribute it and/or
* modify it under the terms of the GNU General Public License as published
* by the Free Software Foundation, either version 2 of the License, or
* (at your option) any later version.
*
* Alternatively, this program may be distributed and modified under the
* terms of Quantum Leaps commercial licenses, which expressly supersede
* the GNU General Public License and are specifically designed for
* licensees interested in retaining the proprietary status of their code.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*
* Contact information:
* Quantum Leaps Web sites: http://www.quantum-leaps.com
*                          http://www.state-machine.com
* e-mail:                  info@quantum-leaps.com
*****************************************************************************/
#ifndef bsp_h
#define bsp_h

#include "product.config"
#include "deviceTypes.h"


#include "stdint.h"


/* Microchip compiler may not ignore "inline" keyword.
 * The keyword should be " __attribute__((always_inline))"
 */
#define INLINE inline

/* If not defined we may define it for use */
#ifndef  BSP_TICKS_PER_SEC
#define BSP_TICKS_PER_SEC    1000UL
#endif

#define GET_TICKS_IN_MS(x)      (x * BSP_TICKS_PER_SEC / 1000)


/* Mandatory implementation */
void BSP_init(void);
void Hardware_Init(void);

// Watchdog
void BSP_watchdog_start(void);
void BSP_watchdog_stop(void);
void BSP_watchdog_reset(void);
void delay_us(uint32_t time_us);

/* Optional */

#endif                                                             /* bsp_h */
