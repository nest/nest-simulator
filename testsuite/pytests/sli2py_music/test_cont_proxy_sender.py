#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# test_cont_proxy_sender.py
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

mcoproxy = nest.Create("music_cont_out_proxy")
mcoproxy.port_name = "voltage_out"
mcoproxy.record_from = ["V_m"]

n1 = nest.Create("iaf_cond_exp", params={"I_e": 300.0})
n2 = nest.Create("iaf_cond_exp", params={"I_e": 600.0})
n = n1 + n2

mcoproxy.targets = n

nest.Simulate(20)
