/*
 *  array.h
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

#ifndef ARRAY_H
#define ARRAY_H
/* 
    Template for an array class.
*/

// special array. discuss vector and valarray implementations.
// specify!
// 
// resizing controled from outside, especially shrinking 
//
//
// p.47
//
//
#include <iostream>
#include <iomanip>
#include <cstddef>
#include <cassert>
#include <algorithm>

template<class T> 
class array 
{
 private:
    T* p;
    size_t n;
public:
    array() : p(NULL), n(0) { };
    array(size_t, const T& = T() );
    array(const array<T>&);
    ~array();

    T* begin(void) 
    {
        return p;
    }

    T* end(void)
    {
        return p+n;
    }
    
    void resize(size_t, const T& = T());
    
    size_t size(void) const 
    {
        return n;
    }

    bool empty(void) const
    {
        return n==0;
    }
    
    T& operator[](size_t);
    const T& operator[](size_t) const;
    void fill(const T&);
    
    array<T>& operator=(const array<T> &); // argument changed to const &
};




template<class T>
array<T>::array(size_t n_s, const T& t)
  : n(n_s)
{
//    p=new T[n](t); // yields warnings with -pedantic
    p=new T[n];
    assert(p!=NULL);
    fill(t);
}

template<class T>
array<T>::array(const array<T>& a)
        :p(NULL), n(0)
{
    if(a.p != NULL)
    {
        n=a.n;
        p=new T[n];   
        assert(p!=NULL);
//        for(size_t i=0; i<n; p[i]=a.p[i++]);
        copy(&a.p[0],&a.p[n],&p[0]);
    }
    
}

template<class T>
array<T>::~array()
{
    if (p!=NULL)  
        delete[] p;
}

template<class T>
void array<T>::fill(const T& e)
{
//    for(size_t i=0; i<n; p[i++]= e);
    std::fill(&p[0],&p[n],e);                   // force global name
}


template<class T>
void array<T>::resize(size_t n_s, const T& t)
{
  // T* h= new T[n_s](t); // yields warning with -pedantic
    T* h= new T[n_s];
    assert(h!=NULL);

    for(T *hi=h; hi <h+n_s; ++hi)
      (*hi)=t;

    size_t min_l,max_l;
    
    if (n < n_s)
    {
        min_l = n; max_l =  n_s;
//        for(size_t i= n; i< n_s; h[i++] = T() ); //  initialize rest
//          ::fill(&h[0],&h[n_s],t);
    }
    else
    {
        min_l = n_s; max_l =  n;
    }

    if(p != NULL)
    {
//        for(size_t i=0; i< min_l; h[i]=p[i++]);  // copy old parts
          copy(&p[0],&p[min_l],&h[0]);
        delete[] p;
    }
    
    p=h;
    n=n_s;
}

template<class T>
T& array<T>::operator[](size_t i)
{
    assert(i<n);
    
    return p[i];
}

template<class T>
const T& array<T>::operator[](size_t i) const
{
    assert(i<n);
    
    return p[i];
}

template<class T>
array<T>& array<T>::operator=(const array<T>& a) 
{
    if (this!=&a)
    {
        if (p!=NULL)
        {
            delete[] p;
            p = NULL;  // all this make something
            n = 0;     // like a = array<int> work
        }
        if(a.p !=NULL)
        {
            n=a.n;
            p=new T[n];
            assert(p!=NULL);
//            for(size_t i=0; i<n; p[i]=a.p[i++]);
              copy(&a.p[0],&a.p[n],&p[0]);
        }
        
    }

    return *this;
}


#endif













