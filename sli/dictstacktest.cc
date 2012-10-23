/*
 *  dictstacktest.cc
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

#include <typeinfo>
#include "namedatum.h"
#include "integerdatum.h"
#include "doubledatum.h"
#include "stringdatum.h"
#include "dictdatum.h"
#include "dictstack.h"

//
// test program for class DictionaryStack
//                       -----------------
// 
// 



int main(void)
{

    DictionaryStack ds;

    NameDatum n1("eins");
    NameDatum n2("zwei");
    NameDatum n3("drei");
    
    Token t1(new IntegerDatum(1));
    Token t2(new DoubleDatum(3.14));
    Token t3(new StringDatum("Schweinebacke"));

    ds.push(Token(new DictionaryDatum()));
    std::cerr << "t1 : "<< t1 << std::endl;   
    ds.def(n1,t1);
    std::cerr << "lookup: " << ds.lookup(n1) << std::endl;
    std::cerr << "lookup: " << ds.lookup(n2) << std::endl;
    ds.def(n2,t2);
    std::cerr << "lookup: " << ds.lookup(n1) << std::endl;
    std::cerr << "lookup: " << ds.lookup(n2) << std::endl;

    Dictionary dios;

    dios.insert(Name("right"),IntegerDatum(5));
    dios.insert(Name("scientific"),IntegerDatum(7));
    dios.insert(Name("showpoint"),IntegerDatum(2));

    ds.push(Token(new DictionaryDatum(dios)));

    std::cerr << "lookup: " << ds.lookup(Name("zwei")) << std::endl;
    std::cerr << "lookup: " << ds.lookup(Name("scientific")) << std::endl;

    ds.def(n2,IntegerDatum(-5));

    std::cerr << "lookup: " << ds.lookup(Name("zwei")) << std::endl;
    ds.pop();
    std::cerr << "lookup: " << ds.lookup(Name("zwei")) << std::endl;
    ds.push(Token(new DictionaryDatum(dios)));

    Token r;

    ds.top(r);

    std::cerr << r << std::endl;

    ds.def_move(n3,t3);

    std::cerr << "lookup: " << ds.lookup(n3) << std::endl;   
    std::cerr << "lookup: " << t3 << std::endl;   
 
    ds.info(std::cerr);
 
    return 0;
}
