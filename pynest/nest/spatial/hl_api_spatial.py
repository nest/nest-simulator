# -*- coding: utf-8 -*-
#
# hl_api_spatial.py
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

from ..lib.hl_api_types import CreateParameter

__all__ = [
    'distance',
    'pos',
    'source_pos',
    'target_pos',
]

distance = CreateParameter('distance', {})


class pos(object):
    x = CreateParameter('position', {'dimension': 0})
    y = CreateParameter('position', {'dimension': 1})
    z = CreateParameter('position', {'dimension': 2})


class source_pos(object):
    x = CreateParameter('position', {'dimension': 0, 'type_id': 1})
    y = CreateParameter('position', {'dimension': 1, 'type_id': 1})
    z = CreateParameter('position', {'dimension': 2, 'type_id': 1})


class target_pos(object):
    x = CreateParameter('position', {'dimension': 0, 'type_id': 2})
    y = CreateParameter('position', {'dimension': 1, 'type_id': 2})
    z = CreateParameter('position', {'dimension': 2, 'type_id': 2})
