/*
 *  slimathlink.cc
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
extern "C" {
#include "mlopen.h"
}

#include"slimathlink.h"
#include<string>
#include<iomanip>
#include<sstream>
#include<algorithm>
#include"stringdatum.h"
#include<cstdio>

/* BeginDocumentation
   Name: Mathematica-SLI_Interface - Interface via Mathlink 

  Description: The Mathematica-SLI Interface is started from the
side of Mathematica. To make the interface available run the 
configure-script with the mathlink option (--with-mathlink=Mathematica-prefix).
The prefix is the path of the Mathematica source (not of the binaries).
A documentation of how you use the interface is given in the
SLICell_demo.nb in extras/mathematica. The whole documentation of the
interface is in LinkMathematica.m in the same folder, which is a
Mathematica package file.
The documentation of MathLink is contained in the Mathematica help.

The interface supports the execution and handling of SLI from within 
Mathematica notebooks, as well as an own cell type called SLIInputCell
in Mathematica.

  Availability: only when configured with mathlink option (see above)

  Author: Markus Diesmann, Sirko Straube		   
  
  FirstVersion: 10.01.03
  
  SeeAlso: MLOpen, MLClose, MLPutString, MLGetString, ToMathematica, ToMathematicaExpression
*/

using namespace std;


/* BeginDocumentation
   Name: MLOpen - opens a Mathlink connection

  Synopsis: MLOpen -> --
		                
  Parameters: none

  Availability: only when configured with mathlink option (see below)

  Remarks: Note that the Mathematica-SLI Interface is started from the
side of Mathematica. These functions are just listed in the
documentation for debugging and will not work without Mathematica!
To make the interface (and these functions) available run the 
configure-script with the mathlink option (--with-mathlink=Mathematica-prefix).
The prefix is the path of the Mathematica source (not of the binaries).
A documentation of how you use the interface is given in the
SLICell_demo.nb in extras/mathematica. The whole documentation of the
interface is in LinkMathematica.m in the same folder, which is a
Mathematica package file.
The documentation of MathLink is contained in the Mathematica help.

  Author: Markus Diesmann, Sirko Straube		   
  
  FirstVersion: 10.01.03
  
  SeeAlso: MLClose, Mathematica-SLI_Interface
*/
void MLInterface::MathLinkOpenFunction::execute(SLIInterpreter *i) const
{
    assert(i->OStack.load() > 0);    

    StringDatum  *sd= dynamic_cast<StringDatum *>(i->OStack.top().datum());

    assert(sd !=NULL);

    std::string args("-linkconnect -linkname ");
    args+=*sd;


    MathLinkInit(args.c_str());  

    i->OStack.pop();
    i->EStack.pop();
}


/* BeginDocumentation
   Name: MLClose - closes a Mathlink connection

  Synopsis: MLClose
		                
  Parameters: none

  Availability: only when configured with mathlink option (see below)

  Remarks: Note that the Mathematica-SLI Interface is started from the
side of Mathematica. These functions are just listed in the
documentation for debugging and will not work without Mathematica!
To make the interface (and these functions) available run the 
configure-script with the mathlink option (--with-mathlink=Mathematica-prefix).
The prefix is the path of the Mathematica source (not of the binaries).
A documentation of how you use the interface is given in the
SLICell_demo.nb in extras/mathematica. The whole documentation of the
interface is in LinkMathematica.m in the same folder, which is a
Mathematica package file.
The documentation of MathLink is contained in the Mathematica help.

  Author: Markus Diesmann, Sirko Straube

  FirstVersion: 10.01.03

  SeeAlso: MLOpen, Mathematica-SLI_Interface
*/
void MLInterface::MathLinkCloseFunction::execute(SLIInterpreter *i) const{
    MathLinkClose(); 
    i->EStack.pop();
}

void MLInterface::MathLinkFlushFunction::execute(SLIInterpreter *i) const{
    MathLinkFlush(); 
    i->EStack.pop();
}


/* BeginDocumentation
  Name: MLPutString - send a string to Mathematica via MathLink 
  Synopsis: string MLPutString  ->
			                
  Parameters: string - a result of a calculation
								
  Description: 
   MathLinkPutString removes the top element from the operand 
   stack and sends it via a MathLink connection to a Mathematica
   interpreter. This function is part of a set of functions 
   implementing interpreter-interpreter interaction between 
   SLI and  Mathematica. Thus, the string typically is a 
   Mathematica command to be executed by the listening Mathematica
   engine.
  
  Availability: only when configured with mathlink option (see below)

  Remarks: Note that the Mathematica-SLI Interface is started from the
side of Mathematica. These functions are just listed in the
documentation for debugging and will not work without Mathematica!
To make the interface (and these functions) available run the 
configure-script with the mathlink option (--with-mathlink=Mathematica-prefix).
The prefix is the path of the Mathematica source (not of the binaries).
A documentation of how you use the interface is given in the
SLICell_demo.nb in extras/mathematica. The whole documentation of the
interface is in LinkMathematica.m in the same folder, which is a
Mathematica package file.
The documentation of MathLink is contained in the Mathematica help.

  Author: Markus Diesmann, Sirko Straube

  FirstVersion: 10.01.03

  SeeAlso: MLGetString, Mathematica-SLI_Interface
*/
void MLInterface::MathLinkPutStringFunction::execute(SLIInterpreter *i) const
{
    assert(i->OStack.load() > 0);    

    StringDatum  *sd= dynamic_cast<StringDatum *>(i->OStack.top().datum());

    assert(sd !=NULL);

    MathLinkPutCharString(sd->c_str());

    i->OStack.pop();
    i->EStack.pop();
}

//MathLinkGetString:
//Function that takes a String (sent by Mathematica) from
//the link and puts it into a String-Object.
//Author: Diesmann, Straube /10.01.2003

void MathLinkGetString(string *s)
{  
 const char *b;
 bool valid;

 valid=MathLinkGetCharString(&b);
 if (valid)
 {
  *s=b;
  MathLinkDisownCharString(b);
 }
}

/* BeginDocumentation
  Name: MLGetString - receives a string from a MathLink connection
  Synopsis: MLGetString
			                
  Parameters: none
								
  Description: 
  MLInterface is used in a loop in the initialization of SLI, if you 
  used the Mathematica-Option. It can be terminated by the "quit" command.
  It is used to take a command (sent by Mathematica) from the link and put
  it on the stack.

  Availability: only when configured with mathlink option (see below)

  Remarks: Note that the Mathematica-SLI Interface is started from the
side of Mathematica. These functions are just listed in the
documentation for debugging and will not work without Mathematica!
To make the interface (and these functions) available run the 
configure-script with the mathlink option (--with-mathlink=Mathematica-prefix).
The prefix is the path of the Mathematica source (not of the binaries).
A documentation of how you use the interface is given in the
SLICell_demo.nb in extras/mathematica. The whole documentation of the
interface is in LinkMathematica.m in the same folder, which is a
Mathematica package file.
The documentation of MathLink is contained in the Mathematica help.

  Author: Markus Diesmann, Sirko Straube

  FirstVersion: 10.01.03

  SeeAlso: MLPutString, Mathematica-SLI_Interface
*/
void MLInterface::MathLinkGetStringFunction::execute(SLIInterpreter *i) const
{                      
    string command;

    MathLinkGetString(&command);
    
    Token t(command);
    i->OStack.push(t);
    
    i->EStack.pop();
}


void MLInterface::init(SLIInterpreter *i)
{
    i->createcommand("MLOpen", &mathlinkopenfunction);
    i->createcommand("MLClose", &mathlinkclosefunction);
    i->createcommand("MLFlush", &mathlinkclosefunction);
    i->createcommand("MLPutString", &mathlinkputstringfunction);
    i->createcommand("MLGetString", &mathlinkgetstringfunction);
    
}

















