# -*- coding: utf-8 -*-
#
# visualization.py
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
Functions to visualize a network built in NEST.
"""

import pydot
import nest

__all__ = [
    'plot_network',
]


def plot_network(nodes, filename, ext_conns=False,
                 plot_modelnames=False):
    """Plot the given nodes and the connections that originate from
    them.

    This function depends on the availability of the pydot module.

    Simplified version for NEST 3.

    Parameters
    ----------
    nodes : NodeCollection
        NodeCollection containing node IDs of nodes to plot
    filename : str
        Filename to save the plot to. Can end either in .pdf or .png to
        determine the type of the output.
    ext_conns : bool, optional
        Draw connections to targets that are not in nodes. If it is True,
        these are drawn to a node named 'ext'.
    plot_modelnames : bool, optional
        Description

    Raises
    ------
    nest.kernel.NESTError
    """

    if len(nodes) == 0:
        nest.kernel.NESTError("nodes must at least contain one node")

    if not isinstance(nodes, nest.NodeCollection):
        raise nest.kernel.NESTError("nodes must be a NodeCollection")

    if ext_conns:
        raise NotImplementedError('ext_conns')
    if plot_modelnames:
        raise NotImplementedError('plot_modelnames')

    conns = nest.GetConnections(nodes)

    graph = pydot.Dot(rankdir='LR', ranksep='5')
    for source, target in zip(conns.sources(), conns.targets()):
        graph.add_edge(pydot.Edge(str(source), str(target)))

    filetype = filename.rsplit(".", 1)[1]
    if filetype == "pdf":
        graph.write_pdf(filename)
    elif filetype == "png":
        graph.write_png(filename)
    else:
        raise nest.kernel.NESTError("Filename must end in '.png' or '.pdf'.")
