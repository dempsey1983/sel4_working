/*
 * Copyright 2014, General Dynamics C4 Systems
 *
 * This software may be distributed and modified according to the terms of
 * the GNU General Public License version 2. Note that NO WARRANTY is provided.
 * See "LICENSE_GPLv2.txt" for details.
 *
 * @TAG(GD_GPL)
 */

#ifndef __ARCH_API_CONSTANTS_H
#define __ARCH_API_CONSTANTS_H

#if CONFIG_MAX_NUM_TRACE_POINTS > 0
#define seL4_LogBufferSize (BIT(LARGE_PAGE_BITS))
#endif /* CONFIG_MAX_NUM_TRACE_POINTS > 0 */

#endif
