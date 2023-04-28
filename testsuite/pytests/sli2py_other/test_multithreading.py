# -*- coding: utf-8 -*-
#
# test_multithreading.py
#
# This file is part of NEST.
#
# Copyright (C) 2004 The NEST Initiative
#
# NEST is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# NEST is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with NEST.  If not, see <http://www.gnu.org/licenses/>.


"""
This is a simple testscript to test if multithreading is working
correctly. The following things are tested:
  * Does setting the number of threads to x result in x threads?
  * Does ResetKernel reset the number of threads to 1?
  * Does default node distribution (modulo) work as expected?
  * Are spikes transmitted between threads as expected?

The data collection over threads is tested in a separate script. See
SeeAlso key below.
"""
