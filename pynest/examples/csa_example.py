# -*- coding: utf-8 -*-
#
# csa_example.py
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

try:
    import csa
    haveCSA = True
except ImportError:
    haveCSA = False

try:
    from PIL import Image
    havePIL = True
except ImportError:
    havePIL = False

def csa_example():

    cs = csa.cset(csa.random(0.1), 10000.0, 1.0)

    # This does not work yet, as the implementation does not yet
    # support nested subnets.
    #
    #pop1 = nest.LayoutNetwork("iaf_neuron", [4,4])
    #pop2 = nest.LayoutNetwork("iaf_neuron", [4,4])

    pop1 = nest.LayoutNetwork("iaf_neuron", [16])
    pop2 = nest.LayoutNetwork("iaf_neuron", [16])

    nest.PrintNetwork(10)

    nest.CGConnect(pop1, pop2, cs, {"weight": 0, "delay": 1})

    pg = nest.Create("poisson_generator", params={"rate": 80000.0})
    nest.DivergentConnect(pg, nest.GetLeaves(pop1)[0], 1.2, 1.0)

    vm = nest.Create("voltmeter", params={"record_to": ["memory"], "withgid": True, "withtime": True, "interval": 0.1})
    nest.DivergentConnect(vm, nest.GetLeaves(pop2)[0])

    nest.Simulate(50.0)

    # Not yet supported in NEST
    #from nest import visualization
    #allnodes = [pg, nest.GetLeaves(pop1)[0], nest.GetLeaves(pop2)[0], vm]
    #visualization.plot_network(allnodes, "test_csa.png", "png", plot_modelnames=True

    #if havePIL:
    #    im = Image.open("test_csa.png")
    #    im.show()

    from nest import voltage_trace
    voltage_trace.from_device(vm)

    #import pylab
    #pylab.show()

if __name__ == "__main__":

    if haveCSA:
        nest.ResetKernel()
        csa_example()
    else:
        print("This example requires CSA to be installed in order to run!")
