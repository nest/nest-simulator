/*
 *  get_mem.c
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

// C includes:
#include <assert.h>


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
