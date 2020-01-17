# -*- coding: utf-8 -*-
#
# __init__.py
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
Initializer of PyNEST.
"""

from . import ll_api                  # noqa
from .ll_api import set_communicator  # noqa

from . import pynestkernel as kernel  # noqa
from .hl_api import *                 # noqa

from . import random                  # noqa
from . import math                    # noqa
from . import spatial_distributions   # noqa
from . import logic                   # noqa
from . import spatial                 # noqa needs to be imported last because of documentation generation
