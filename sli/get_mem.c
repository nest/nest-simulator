/*
 *  bg_get_mem.c
 *
 *  This file is part of NEST.
 *
 *  Copyright (C) 2004 The NEST Initiative
 *
 *  NEST is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  NEST is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with NEST.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

// Generated includes:
#include "config.h"

// C includes:
#include <assert.h>

#if defined IS_BLUEGENE_P || defined IS_BLUEGENE_Q

#if defined IS_BLUEGENE_P
#include <spi/bgp_SPI.h>
typedef uint32_t bgmemsize_t;
#elif defined IS_BLUEGENE_Q
#include <kernel/memory.h>
typedef uint64_t bgmemsize_t;
#endif

unsigned long
bg_get_heap_mem()
{
  bgmemsize_t memory = 0;
  Kernel_GetMemorySize( KERNEL_MEMSIZE_HEAP, &memory );
  return ( unsigned long ) memory;
}

unsigned long
bg_get_stack_mem()
{
  bgmemsize_t memory = 0;
  Kernel_GetMemorySize( KERNEL_MEMSIZE_STACK, &memory );
  return ( unsigned long ) memory;
}

unsigned long
bg_get_mmap_mem()
{
  bgmemsize_t memory = 0;
  Kernel_GetMemorySize( KERNEL_MEMSIZE_MMAP, &memory );
  return ( unsigned long ) memory;
}

#else

/* ISO C forbids an empty translation unit, so we define dummies. */
unsigned long
bg_get_heap_mem()
{
  assert( 0 || "Only implemented on BlueGene." );
  return 0;
}

unsigned long
bg_get_stack_mem()
{
  assert( 0 || "Only implemented on BlueGene." );
  return 0;
}

unsigned long
bg_get_mmap_mem()
{
  assert( 0 || "Only implemented on BlueGene." );
  return 0;
}

#endif

#if defined __APPLE__ && defined HAVE_MACH_MACH_H
#include <mach/mach.h>
unsigned long
darwin_get_used_mem()
{
  struct task_basic_info t_info;
  mach_msg_type_number_t t_info_count = TASK_BASIC_INFO_COUNT;

  kern_return_t result = task_info( mach_task_self(), TASK_BASIC_INFO, ( task_info_t ) &t_info, &t_info_count );
  assert( result == KERN_SUCCESS || "Problem occured during getting of task_info." );
  return t_info.resident_size;
}
#else

unsigned long
darwin_get_used_mem()
{
  assert( 0 || "Only implemented on Darwin/Apple with mach/mach.h available." );
  return 0;
}

#endif
