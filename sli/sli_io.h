/*
 *  sli_io.h
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

#ifndef SLI_IO_H
#define SLI_IO_H
/**************************************
  All SLI stream I/O functions are
  defined in this module.
  Functions related to the filesystem are located in
  filesystem.h
  *************************************/

// Includes from sli:
#include "interpret.h"


void init_sli_io( SLIInterpreter* );

class MathLinkPutStringFunction : public SLIFunction
{
public:
  MathLinkPutStringFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class XIfstreamFunction : public SLIFunction
{
public:
  XIfstreamFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class IfstreamFunction : public SLIFunction
{
public:
  IfstreamFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class OfstreamFunction : public SLIFunction
{
public:
  OfstreamFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class OfsopenFunction : public SLIFunction
{
public:
  OfsopenFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

#ifdef HAVE_SSTREAM

class IsstreamFunction : public SLIFunction
{
public:
  IsstreamFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class OsstreamFunction : public SLIFunction
{
public:
  OsstreamFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class StrSStreamFunction : public SLIFunction
{
public:
  StrSStreamFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};
#else

class OstrstreamFunction : public SLIFunction
{
public:
  OstrstreamFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class StrFunction : public SLIFunction
{
public:
  StrFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};
#endif

class PrintFunction : public SLIFunction
{
public:
  PrintFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class PrettyprintFunction : public SLIFunction
{
public:
  PrettyprintFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class CloseistreamFunction : public SLIFunction
{
public:
  CloseistreamFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class CloseostreamFunction : public SLIFunction
{
public:
  CloseostreamFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class FlushFunction : public SLIFunction
{
public:
  FlushFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class EndlFunction : public SLIFunction
{
public:
  EndlFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class EndsFunction : public SLIFunction
{
public:
  EndsFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class EatwhiteFunction : public SLIFunction
{
public:
  EatwhiteFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class SetwFunction : public SLIFunction
{
public:
  SetwFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class SetprecisionFunction : public SLIFunction
{
public:
  SetprecisionFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class IOSFixedFunction : public SLIFunction
{
public:
  IOSFixedFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class IOSScientificFunction : public SLIFunction
{
public:
  IOSScientificFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class IOSDefaultFunction : public SLIFunction
{
public:
  IOSDefaultFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class IOSShowpointFunction : public SLIFunction
{
public:
  IOSShowpointFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class IOSNoshowpointFunction : public SLIFunction
{
public:
  IOSNoshowpointFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class IOSShowbaseFunction : public SLIFunction
{
public:
  IOSShowbaseFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class IOSNoshowbaseFunction : public SLIFunction
{
public:
  IOSNoshowbaseFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class IOSDecFunction : public SLIFunction
{
public:
  IOSDecFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class IOSHexFunction : public SLIFunction
{
public:
  IOSHexFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class IOSOctFunction : public SLIFunction
{
public:
  IOSOctFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class IOSLeftFunction : public SLIFunction
{
public:
  IOSLeftFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class IOSRightFunction : public SLIFunction
{
public:
  IOSRightFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class IOSInternalFunction : public SLIFunction
{
public:
  IOSInternalFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class GetcFunction : public SLIFunction
{
public:
  GetcFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class GetsFunction : public SLIFunction
{
public:
  GetsFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class GetlineFunction : public SLIFunction
{
public:
  GetlineFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class IGoodFunction : public SLIFunction
{
public:
  IGoodFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class IClearFunction : public SLIFunction
{
public:
  IClearFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class OClearFunction : public SLIFunction
{
public:
  OClearFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class IFailFunction : public SLIFunction
{
public:
  IFailFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class OGoodFunction : public SLIFunction
{
public:
  OGoodFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class Cvx_fFunction : public SLIFunction
{
public:
  Cvx_fFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class IEofFunction : public SLIFunction
{
public:
  IEofFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class OEofFunction : public SLIFunction
{
public:
  OEofFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class In_AvailFunction : public SLIFunction
{
public:
  In_AvailFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class ReadDoubleFunction : public SLIFunction
{
public:
  ReadDoubleFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class ReadIntFunction : public SLIFunction
{
public:
  ReadIntFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

class ReadWordFunction : public SLIFunction
{
public:
  ReadWordFunction()
  {
  }
  void execute( SLIInterpreter* ) const;
};

#endif
