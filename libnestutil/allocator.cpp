/*
 *  allocator.cpp
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

#include "allocator.h"

sli::pool::pool()
  : initial_block_size(1024),
    growth_factor(1),
    block_size(initial_block_size),
    el_size(sizeof(link)),
    instantiations(0),
    total(0),
    capacity(0),
    chunks(0), 
    head(0),
    initialized_(false)
{}

sli::pool::pool(const sli::pool &p)
  : initial_block_size(p.initial_block_size),
    growth_factor(p.growth_factor),
    block_size(initial_block_size),
    el_size(sizeof(link)),
    instantiations(0),
    total(0),
    capacity(0),
    chunks(0), 
    head(0),
    initialized_(false)
{}


sli::pool::pool(size_t n, size_t initial, size_t growth)
  : initial_block_size(initial),
    growth_factor(growth),
    block_size(initial_block_size),
    el_size( ( n < sizeof(link))? sizeof(link): n),
    instantiations(0),
    total(0),
    capacity(0),
    chunks(0), 
    head(0),
    initialized_(true)
{}

void sli::pool::init(size_t n, size_t initial, size_t growth)
{
  assert(instantiations == 0);

  initialized_=true;

  initial_block_size=initial;
  growth_factor=growth;
  block_size=initial_block_size;
  el_size =  ( n < sizeof(link))? sizeof(link): n;
  instantiations=0;
  total=0;
  capacity=0;
  chunks=0; 
  head=0;
}

sli::pool::~pool()
{
  chunk *n=chunks;
  while(n)
  {
    chunk *p=n;
    n=n->next;
    delete p;
  }
}

sli::pool & sli::pool::operator=(const sli::pool&p)
{
  if(&p == this)
    return *this;

  initial_block_size=p.initial_block_size;
  growth_factor=p.growth_factor;
  block_size=initial_block_size;
  el_size=p.el_size;
  instantiations=0;
  total=0;
  chunks=0; 
  head=0;
  initialized_=false;
  
  return *this;
}

void sli::pool::grow(size_t nelements)
{
  chunk *n = new chunk(nelements*el_size);
  total    += nelements;

  n->next = chunks;
  chunks = n;
  char *start= n->mem;
  char *last= &start[ (nelements-1)*el_size];

  for(char *p=start; p<last; p+=el_size)
    reinterpret_cast<link*>(p)->next=reinterpret_cast<link*>(p+el_size);
  reinterpret_cast<link*>(last)->next = NULL;
  head = reinterpret_cast<link*>(start);

}

void sli::pool::grow(void)
{
  grow(block_size);
  block_size *= growth_factor;
}

void sli::pool::reserve(size_t n)
{
    const size_t capacity=total-instantiations; 
    if(capacity < n)
    grow(((n-capacity)/block_size+1)*block_size);
}
