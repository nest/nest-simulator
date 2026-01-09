#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# test_message_receiver.py
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
import numpy as np

ref_times = [30.0, 70.0, 90.0]
ref_messages = ["First", "Second", "Third"]

mmip = nest.Create("music_message_in_proxy", params={"port_name": "msgdata"})

nest.Simulate(100)

np.testing.assert_allclose(mmip.data["messages_times"], ref_times)
np.testing.assert_equal(mmip.data["messages"], ref_messages)
