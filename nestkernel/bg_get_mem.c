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

#include "config.h"

#if defined IS_BLUEGENE_P || defined IS_BLUEGENE_Q

#if defined IS_BLUEGENE_P
  #include <spi/bgp_SPI.h>
  typedef uint32_t bgmemsize_t;
#elif defined IS_BLUEGENE_Q
  #include <kernel/memory.h>
  typedef uint64_t bgmemsize_t;
#endif

unsigned long bg_get_heap_mem()
{
  bgmemsize_t memory = 0;
  Kernel_GetMemorySize(KERNEL_MEMSIZE_HEAP, &memory);
  return (unsigned long)memory;
}

unsigned long bg_get_stack_mem()
{
  bgmemsize_t memory = 0;
  Kernel_GetMemorySize(KERNEL_MEMSIZE_STACK, &memory);
  return (unsigned long)memory;
}

#endif
