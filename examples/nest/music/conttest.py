#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# conttest.py
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

import nest

if not nest.ll_api.sli_func("statusdict/have_music ::"):
    import sys

    print("NEST was not compiled with support for MUSIC, not running.")
    sys.exit()

mcip = nest.Create('music_cont_in_proxy')
nest.SetStatus(mcip, {'port_name': 'contdata'})

# Simulate and get vector data with a granularity of 10 ms:
time = 0
while time < 1000:
    nest.Simulate(10)
    data = nest.GetStatus(mcip, 'data')
    print(data)
    time += 10
