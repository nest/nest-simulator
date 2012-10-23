/*
 *  sligraphics.cc
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

#include <cctype>     // for isspace
#include <cstdio>
#include <iostream>

#include "arraydatum.h"
#include "stringdatum.h"
#include "integerdatum.h"
#include "aggregatedatum.h"
#include "numericdatum.h"
#include "fdstream.h"
#include "sligraphics.h"


/*   BeginDocumentation
Name:readPGM - read in grey-level image in PGM Format.

Synopsis:string readPGM -> int    int    int   arraytype
fname  readPGM -> width height maxval [grayvals]

Description:this function reads an image file in the PGM format and 
returns the width, height, maximum gray value and the 
image itself (as a linear array).
On Unix systems, man pgm should give you a description
of the PGM (Portable GrayMap) image format.

Parameters:fname      - name of file to be read
[grayvals] - one-dim. array containing the pixel gray values,
             starting from the upper left corner of the image, 
   	  continuing rowwise (normal englissh reading order).
maxval     - the maximum gray value
width      - width of image in pixels (no. of columns)
height     - height of image (no. of rows)

Examples:(FancyImage.pgm) readPGM -> 16 24 255 [grayvals]
    
This reads the image FancyImage.pgm, and tells you that it has 
16 columns, 24 rows, and a maximum gray value of 255. The pixel
gray values are stored in the array.

Author:Schmuker, Gewaltig

FirstVersion:9.1.2003

SeeAlso:writePGM
*/
void SLIgraphics::ReadPGMFunction::execute(SLIInterpreter *i) const
{
  // call: filename readPGM -> width height maxval image(array)
  
  if(i->OStack.load()<1)
    {
      i->raiseerror(i->StackUnderflowError);
      return;
    }

  StringDatum *sd= dynamic_cast<StringDatum *>(i->OStack.top().datum());
  
  if(sd ==NULL)
    {
      i->raiseerror(i->ArgumentTypeError);
      return;
    }
  std::istream *in=NULL;  
  vector<long> image;
  int width=0, height=0, maxval=0;  // for the image parameters: width, height, maxval

  try
    {
      in = openPGMFile(sd);
      char magic[2];
      readMagicNumber(in, magic);
      initRead(in, width, height, maxval);
      readImage(in, magic, image, width, height, maxval);
      delete in;
    }
  catch(string const &s)
    {
      delete in;
      i->message(SLIInterpreter::M_ERROR, "readPGM","Error reading image.");
      i->message(SLIInterpreter::M_ERROR, "readPGM",s.c_str());
      i->raiseerror(i->BadIOError);
      return;
    }
  
  i->EStack.pop();
  i->OStack.pop();
  i->OStack.push(ArrayDatum(image));
  i->OStack.push(maxval);
  i->OStack.push(height);
  i->OStack.push(width);
}

std::istream *  SLIgraphics::ReadPGMFunction::openPGMFile(StringDatum *filename) const
{
  //opens pgm file for reading and returns pointer to the istream
  std::istream *in = new ifdstream(filename->c_str());
  if(in->good())
    {
      return in;
    }
  else 
    {
      throw string("File open error.");
    }
} 

void  SLIgraphics::ReadPGMFunction::readMagicNumber(std::istream * in, 
					       char * magic) const
{
  // reads in the magic number which determines the file format
  try
    {
      *in >> magic;
    }
  catch(std::exception& e) 
    {
      throw string("Magic number read error: ")+ e.what();
    }
}

void  SLIgraphics::ReadPGMFunction::initRead(std::istream * in, 
					int &width,
					int &height,
					int &maxval) const
{
  //reads the width, height, and max. gray value in this order
  char temp[256];
  try
    {
      //throw away whitespaces after magic number
      //otherwise, >> gets confused about the newline before the numbers
      char trash;
      while(std::isspace(trash=in->get()))
	continue;
      in->putback(trash);
      //skip comments
      do
      {
	in->getline(temp, 255);
      } 
      while (temp[0]=='#');
      //width and height are now in temp, so parse it
      sscanf(temp, "%d %d", &width, &height);
      *in >> maxval;
    }
  catch(std::exception& e) 
    {
      throw string("Read init error: ")+ e.what();
    }
}

void  SLIgraphics::ReadPGMFunction::readImage(std::istream *in, 
					 char magic[2], 
					 vector<long> &image, 
					 int width,
					 int height,
					 int maxval) const
{
  // this reads the gray value array
  image.clear();
  image.reserve(width*height);
  
  try{
    if(string(magic)==string("P2"))  //ASCII PGM
      { 
	int  tmp;
	while((*in >> tmp) && !(in->eof()))
	  {
	    image.push_back((long) tmp);
	  }
      }
    else if(string(magic)==string("P5")  || 
	    string(magic)==string("P6"))    //Raw PGM (resp. PPM)
      { 
	if(maxval > 255)
	  {
	    throw string("read: maxval too large for format RawPGM(P5).");
	  }
	char tmp;
	long tmp2;
	in->read(&tmp, 1); //throw away LF after maxval
	//TODO: Protect this from reading too much data like trailing 
	//newlines: use for instead of while
	while(in->read(&tmp, 1) && !(in->eof())){
	  tmp2=(unsigned char) tmp;
	  image.push_back((long) tmp2);
	}	  
      }
    else
      {
	throw string("image read error:") + string(magic) + string(": Unsupported file type.");
      }
  }
  catch(std::exception& e){
    throw string("image read error: ") + e.what();
  }
}

/*   BeginDocumentation
Name:writePGM - write out a grey-level image in PGM format

Synopsis:string arraytype   int    int   int   writePGM
fname  [grayvals] maxval height width writePGM

Description:This writes an array of integers as grey-level image
using the PGM (PortableGrayMap) format. 
On Unix systems, man 5 pgm should give you a description of
the PGM image format. 

Parameters:fname      - name of file to be written
[grayvals] - one-dim. array containing the pixel gray values,
             starting at the upper left corner and continuing
	     rowwise (normal english reading order).
maxval     - the maximum gray value
width      - width of image in pixels (no. of columns)
height     - height of image (no. of rows)

Remarks:So far, only the plain ASCII variant of the PGM Format is 
used. In the PGM manual, this is referred to as "P2".

Examples:(FancyImage.pgm) [grayvals] 255 24 16 writePGM
This writes an image named FancyImage.pgm with the gray values 
from the array, having 16 columns and 24 rows.

Author:Schmuker, Gewaltig

FirstVersion:9.1.2003

SeeAlso:readPGM
*/	     
void SLIgraphics::WritePGMFunction::execute(SLIInterpreter *i) const
{
  //TODO: fix argument order!!! Should be the same as when getting 
  //the parameters read by readPGM.First make sure that your script is 
  //properly working! 
  
  // call: filename image(array) maxval height width writePGM
  if(i->OStack.load()<5)
    {
      i->raiseerror(i->StackUnderflowError);
      return;
    }
  
  IntegerDatum * w = dynamic_cast<IntegerDatum*>(i->OStack.pick(0).datum());
  IntegerDatum * h = dynamic_cast<IntegerDatum*>(i->OStack.pick(1).datum());
  IntegerDatum * m = dynamic_cast<IntegerDatum*>(i->OStack.pick(2).datum());
  ArrayDatum * image  = dynamic_cast<ArrayDatum*>(i->OStack.pick(3).datum());
  StringDatum * filename= dynamic_cast<StringDatum*>(i->OStack.pick(4).datum());
  
  long width  = (long) w->get();
  long height = (long) h->get();
  long maxval = (long) m->get();
  
  std::ostream * out=NULL;
  
  try
    {
      out = new ofdstream(filename->c_str());
      
      if(!out->good()) 
	throw string("Error when opening file for writing.");
      
      if ((long) image->size() != width * height) 
	throw string("Array size does not match given dimensions.");
      
      //Plain ASCII PGM format
      *out << "P2" << std::endl;  //Magic Number
      *out << "# CREATOR: SLI/Synod. The NEST cooperation 2003." << std::endl;
      *out << width << " " << height << std::endl;
      *out << maxval << std::endl;
      for (unsigned int i=0; i < image->size(); i++){
	*out << image->get(i);

	// write newline after 20 written numbers or
	// one pixel row, which ever comes first
	if (width > 20) 
	  {
	    if ((i+1) % 20 == 0) *out << std::endl;
	    else *out << " ";
	  }
	else 
	  {
	    if ((i+1)% width == 0) *out << std::endl;
	    else *out << " ";
	  }
      }

      *out << std::endl;  //make sure file ends in a newline
      delete out;
    }
  catch(std::exception& e) {
    throw string("exception: ") + e.what();
  }
  catch(string const &s)
    {
      delete out;
      i->message(SLIInterpreter::M_ERROR, "writePGM","Error writing image.");
      i->message(SLIInterpreter::M_ERROR, "writePGM",s.c_str());
      i->raiseerror(i->BadIOError);
      return;
    }

  // clean up
  i->EStack.pop();
  i->OStack.pop();
  i->OStack.pop();
  i->OStack.pop();
  i->OStack.pop();
  i->OStack.pop();
}



void SLIgraphics::init(SLIInterpreter *i)
{
  i->createcommand("readPGM",&readpgmfunction);
  i->createcommand("writePGM",&writepgmfunction);
}

const string SLIgraphics::name() const
{
  return string("SLIgraphics");
}

const string SLIgraphics::commandstring() const
{
  return "M_DEBUG (SLIgraphics) (Initialising Graphics IO) message";
}
