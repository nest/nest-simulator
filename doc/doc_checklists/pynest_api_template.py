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

"""Brief Description of Module

"""


def function.name(x=default, y=None):
    """Connect pre nodes to post nodes

    Nodes in pre and post are connected using the specified connectivity
    (all-to-all by default) and synapse type (static_synapse by default).
    Details depend on the connectivity rule (see Notes).

    .. deprecated:: 1.6.0

            `ndobj_old` will be removed in NumPy 2.0.0, it is replaced by
            `ndobj_new` because the latter works also with array subclasses.

    Parameters
    ----------
    x : type
        Description of parameter `x`.
    y
         Description of parameter `y` (with type not specified)

    Returns
    --------
    err_code : int
        Non-zero value indicates error code, or zero on success.

    int
         Description of anonymous integer return value.

    Raises
    -------
     kernel.NESTError
         If the matrix is not numerically invertible.

    Notes
    -------

    Details on the connectivity. This section may include
    mathematical equations:
    The discrete-time Fourier time-convolution  [1]_ property states that

    .. math::

         x(n) * y(n) \Leftrightarrow X(e^{j\omega } )Y(e^{j\omega } )


    The value of :math:`\omega` is larger than 5.


    * use the asterisk for bullet items
    * second item

    References
    -----------

    .. [1] Bonewald LF. 2011. The amazing osteocyte. Journal of Bone and
           Mineral Research 26(2):229–238. DOI: 10.1002/jbmr.320.

    See Also
    ---------

    func_a : Function a with its description.
    func_b, func_c


    KEYWORDS: Important terms, Comma Separated
    """

    # This code was included because of bug when running X
    # Temporary, I HOPE HOPE HOPE

    if model is not None and syn_spec is not None:
        raise kernel.NESTerror(
            "'model' is an alias for 'syn_spec' and cannot"
            " be used together with 'syn_spec'.")
