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

"""This template demonstrates how a docstring should look.

   It is based on the NumPy style docstring and uses reStructured text mark
   up. Extra annotations are marked with brackets like this
   [[ remove content ]].  Copy this docstring and replace the text to fit
   your function, but do not change the headings and keep the order. The
   bracketed sections should be removed completely from the final version.

"""


def get.Connections(source=None, target=None, synape_model=None,
                    synapse_label=None):
    """Return an array of connection identifiers

    Any combination of source, target, synapse_model and
    synapse_label parameters is permitted.

    [[ Deprecation warnings should appear directly after a brief description
      It should state  what version the object was deprecated, when it will be
      removed and what recommend way obtains the same functionality]]

    .. deprecated:: 1.6.0

            `ndobj_old` will be removed in NumPy 2.0.0, it is replaced by
            `ndobj_new` because the latter works also with array subclasses.

    Parameters
    ----------
    source : list, optional
        Source GIDs, only connections from these
        pre-synaptic neurons are returned
    target : list, optional
        Target GIDs, only connections to these
        post-synaptic neurons are returned
    synapse_model : str, optional
        Only connections with this synapse type are returned
    synapse_label : int, optional
        (non-negative) only connections with this synapse label are returned


    Returns
    -------
    array:
        Connections as 5-tuples with entries
        (source-gid, target-gid, target-thread, synapse-id, port)

    Raises
    -------
    TypeError

    Notes
    -------

    Details on the connectivity. [[ Here details regarding the code or further
    explanations can be included. This section may include mathematical
    equations, written in LaTeX format. You can include references to relevant
    papers using the reStructured format ]]

    The discrete-time Fourier time-convolution [1]_ property states that

    .. math::

         x(n) * y(n) \Leftrightarrow X(e^{j\omega } )Y(e^{j\omega } )


    The value of :math:`\omega` is larger than 5.



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

    .. [1] Bonewald LF. 2011. The amazing osteocyte. Journal of Bone and
           Mineral Research 26(2):229–238. DOI: 10.1002/jbmr.320.

    [[Keywords that idenfity important aspects of the function but not the
     function name itself can be included in a comma separated list. These
     terms will help us increase discoverability of related documents ]]

    KEYWORDS: Important terms, Comma Separated
    """

    # [[ in line comments should be used to explain why this code is here]]
    # This code was included because of bug Y when running X
    # Temporary, I HOPE HOPE HOPE

    if model is not None and syn_spec is not None:
        raise kernel.NESTerror(
            "'model' is an alias for 'syn_spec' and cannot"
            " be used together with 'syn_spec'.")
