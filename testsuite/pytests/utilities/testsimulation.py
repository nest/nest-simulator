# -*- coding: utf-8 -*-
#
# testsimulation.py
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

import dataclasses

import nest
import numpy as np
import testutil


@dataclasses.dataclass
class Simulation:
    local_num_threads: int = 1
    resolution: float = 0.1
    duration: float = 8.0
    weight: float = 100.0
    delay: float = 1.0

    def setup(self):
        pass

    def simulate(self):
        nest.Simulate(self.duration)
        if hasattr(self, "voltmeter"):
            return np.column_stack((self.voltmeter.events["times"], self.voltmeter.events["V_m"]))

    def run(self):
        self.setup()
        return self.simulate()

    def __init_subclass__(cls, **kwargs):
        super().__init_subclass__(**kwargs)
        testutil.create_dataclass_fixtures(cls)
