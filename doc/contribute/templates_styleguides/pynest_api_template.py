# -*- coding: utf-8 -*-
#
# pynest_api_template.py
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

"""[[ This template demonstrates how to create a docstring for the PyNEST API.

   If you have modified an API, please ensure you update the docstring!

   The format is based on `NumPy style docstring
   <https://numpydoc.readthedocs.io/en/latest/format.html>`_ and uses
   reStructuredText markup. Please review the syntax rules if you are
   unfamiliar with either reStructuredText or NumPy style docstrings.

   Copy this file and replace the sample text with a description of the API.
   The double bracketed sections [[ ]], which provide explanations, should be
   completely removed from your final version - Including this entire
   docstring!
   ]]
"""


def GetConnections(source=None, target=None, synape_model=None, synapse_label=None):
    """Return a `SynapseCollection` representing the connection identifiers.
    [[ In a single 'summary line', state what the function does ]]
    [[ All functions should have a docstring with at least a summary line ]]

    [[ Below summary line (separated by new line), there should be an extended
       summary section that should be used to clarify functionality.]]

    Any combination of `source`, `target`, `synapse_model` and
    `synapse_label` parameters is permitted.

    [[ Deprecation warnings should appear directly after the extended summary.
      It should state in what version the object was deprecated, when it will
      be removed and what recommend way obtains the same functionality]]

    .. deprecated:: 1.6.0

            `ndobj_old` will be removed in NumPy 2.0.0, it is replaced by
            `ndobj_new` because the latter works also with array subclasses.

    [[ For all headings ensure the underline --- is at least the length of the
    heading ]]

    Parameters
    ----------
    source : NodeCollection, optional
        Source node IDs, only connections from these
        pre-synaptic neurons are returned
    target : NodeCollection, optional
        Target node IDs, only connections to these
        postsynaptic neurons are returned
    synapse_model : str, optional
        Only connections with this synapse type are returned
    synapse_label : int, optional
        (non-negative) only connections with this synapse label are returned


    Returns
    -------
    SynapseCollection:
        Object representing the source-node_id, target-node_id, target-thread, synapse-id, port of connections, see
        :py:class:`.SynapseCollection` for more.

    Raises
    -------
    TypeError

    Notes
    -------

    Details on the connectivity. [[ Here details regarding the code or further
    explanations can be included. This section may include mathematical
    equations, written in LaTeX format. You can include references to relevant
    papers using the reStructuredText syntax. Do not include model formulas ]]

    The discrete-time Fourier time-convolution [1]_ property states that

    .. math::

         x(n) * y(n) \Leftrightarrow X(e^{j\omega } )Y(e^{j\omega } )


    The value of :math:`\omega` is larger than 5.



    [[ The See Also section should include 2 or 3 related functions. ]]

    See Also
    ---------

    func_a : Function a with its description.
    func_b, func_c


    References
    -----------
    [[ Note the format of the reference. No bold nor italics is used. Last name
       of author(s) followed by year, title in sentence case and full name of
       journal followed by volume and page range. Include the doi if
       applicable.]]

    .. [1] Bonewald LF. (2011). The amazing osteocyte. Journal of Bone and
           Mineral Research 26(2):229–238. DOI: 10.1002/jbmr.320.
       """

    # [[ in line comments should be used to explain why this code is here]]
    # This code was included because of bug Y when running X
    # Temporary, I HOPE HOPE HOPE

    if model is not None and syn_spec is not None:
        raise kernel.NESTerror(
            "'model' is an alias for 'syn_spec' and cannot"
            " be used together with 'syn_spec'.")
