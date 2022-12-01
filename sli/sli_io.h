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
  void execute( SLIInterpreter* ) const override;
};

class XIfstreamFunction : public SLIFunction
{
public:
  XIfstreamFunction()
  {
  }
  void execute( SLIInterpreter* ) const override;
};

class IfstreamFunction : public SLIFunction
{
public:
  IfstreamFunction()
  {
  }
  void execute( SLIInterpreter* ) const override;
};

class OfstreamFunction : public SLIFunction
{
public:
  OfstreamFunction()
  {
  }
  void execute( SLIInterpreter* ) const override;
};

class OfsopenFunction : public SLIFunction
{
public:
  OfsopenFunction()
  {
  }
  void execute( SLIInterpreter* ) const override;
};

#ifdef HAVE_SSTREAM

class IsstreamFunction : public SLIFunction
{
public:
  IsstreamFunction()
  {
  }
  void execute( SLIInterpreter* ) const override;
};

class OsstreamFunction : public SLIFunction
{
public:
  OsstreamFunction()
  {
  }
  void execute( SLIInterpreter* ) const override;
};

class StrSStreamFunction : public SLIFunction
{
public:
  StrSStreamFunction()
  {
  }
  void execute( SLIInterpreter* ) const override;
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
  void execute( SLIInterpreter* ) const override;
};

class PrettyprintFunction : public SLIFunction
{
public:
  PrettyprintFunction()
  {
  }
  void execute( SLIInterpreter* ) const override;
};

class CloseistreamFunction : public SLIFunction
{
public:
  CloseistreamFunction()
  {
  }
  void execute( SLIInterpreter* ) const override;
};

class CloseostreamFunction : public SLIFunction
{
public:
  CloseostreamFunction()
  {
  }
  void execute( SLIInterpreter* ) const override;
};

class FlushFunction : public SLIFunction
{
public:
  FlushFunction()
  {
  }
  void execute( SLIInterpreter* ) const override;
};

class EndlFunction : public SLIFunction
{
public:
  EndlFunction()
  {
  }
  void execute( SLIInterpreter* ) const override;
};

class EndsFunction : public SLIFunction
{
public:
  EndsFunction()
  {
  }
  void execute( SLIInterpreter* ) const override;
};

class EatwhiteFunction : public SLIFunction
{
public:
  EatwhiteFunction()
  {
  }
  void execute( SLIInterpreter* ) const override;
};

class SetwFunction : public SLIFunction
{
public:
  SetwFunction()
  {
  }
  void execute( SLIInterpreter* ) const override;
};

class SetprecisionFunction : public SLIFunction
{
public:
  SetprecisionFunction()
  {
  }
  void execute( SLIInterpreter* ) const override;
};

class IOSFixedFunction : public SLIFunction
{
public:
  IOSFixedFunction()
  {
  }
  void execute( SLIInterpreter* ) const override;
};

class IOSScientificFunction : public SLIFunction
{
public:
  IOSScientificFunction()
  {
  }
  void execute( SLIInterpreter* ) const override;
};

class IOSDefaultFunction : public SLIFunction
{
public:
  IOSDefaultFunction()
  {
  }
  void execute( SLIInterpreter* ) const override;
};

class IOSShowpointFunction : public SLIFunction
{
public:
  IOSShowpointFunction()
  {
  }
  void execute( SLIInterpreter* ) const override;
};

class IOSNoshowpointFunction : public SLIFunction
{
public:
  IOSNoshowpointFunction()
  {
  }
  void execute( SLIInterpreter* ) const override;
};

class IOSShowbaseFunction : public SLIFunction
{
public:
  IOSShowbaseFunction()
  {
  }
  void execute( SLIInterpreter* ) const override;
};

class IOSNoshowbaseFunction : public SLIFunction
{
public:
  IOSNoshowbaseFunction()
  {
  }
  void execute( SLIInterpreter* ) const override;
};

class IOSDecFunction : public SLIFunction
{
public:
  IOSDecFunction()
  {
  }
  void execute( SLIInterpreter* ) const override;
};

class IOSHexFunction : public SLIFunction
{
public:
  IOSHexFunction()
  {
  }
  void execute( SLIInterpreter* ) const override;
};

class IOSOctFunction : public SLIFunction
{
public:
  IOSOctFunction()
  {
  }
  void execute( SLIInterpreter* ) const override;
};

class IOSLeftFunction : public SLIFunction
{
public:
  IOSLeftFunction()
  {
  }
  void execute( SLIInterpreter* ) const override;
};

class IOSRightFunction : public SLIFunction
{
public:
  IOSRightFunction()
  {
  }
  void execute( SLIInterpreter* ) const override;
};

class IOSInternalFunction : public SLIFunction
{
public:
  IOSInternalFunction()
  {
  }
  void execute( SLIInterpreter* ) const override;
};

class GetcFunction : public SLIFunction
{
public:
  GetcFunction()
  {
  }
  void execute( SLIInterpreter* ) const override;
};

class GetsFunction : public SLIFunction
{
public:
  GetsFunction()
  {
  }
  void execute( SLIInterpreter* ) const override;
};

class GetlineFunction : public SLIFunction
{
public:
  GetlineFunction()
  {
  }
  void execute( SLIInterpreter* ) const override;
};

class IGoodFunction : public SLIFunction
{
public:
  IGoodFunction()
  {
  }
  void execute( SLIInterpreter* ) const override;
};

class IClearFunction : public SLIFunction
{
public:
  IClearFunction()
  {
  }
  void execute( SLIInterpreter* ) const override;
};

class OClearFunction : public SLIFunction
{
public:
  OClearFunction()
  {
  }
  void execute( SLIInterpreter* ) const override;
};

class IFailFunction : public SLIFunction
{
public:
  IFailFunction()
  {
  }
  void execute( SLIInterpreter* ) const override;
};

class OGoodFunction : public SLIFunction
{
public:
  OGoodFunction()
  {
  }
  void execute( SLIInterpreter* ) const override;
};

class Cvx_fFunction : public SLIFunction
{
public:
  Cvx_fFunction()
  {
  }
  void execute( SLIInterpreter* ) const override;
};

class IEofFunction : public SLIFunction
{
public:
  IEofFunction()
  {
  }
  void execute( SLIInterpreter* ) const override;
};

class OEofFunction : public SLIFunction
{
public:
  OEofFunction()
  {
  }
  void execute( SLIInterpreter* ) const override;
};

class In_AvailFunction : public SLIFunction
{
public:
  In_AvailFunction()
  {
  }
  void execute( SLIInterpreter* ) const override;
};

class ReadDoubleFunction : public SLIFunction
{
public:
  ReadDoubleFunction()
  {
  }
  void execute( SLIInterpreter* ) const override;
};

class ReadIntFunction : public SLIFunction
{
public:
  ReadIntFunction()
  {
  }
  void execute( SLIInterpreter* ) const override;
};

class ReadWordFunction : public SLIFunction
{
public:
  ReadWordFunction()
  {
  }
  void execute( SLIInterpreter* ) const override;
};

#endif
