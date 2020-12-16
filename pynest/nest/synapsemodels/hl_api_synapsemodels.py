# -*- coding: utf-8 -*-
#
# hl_api_synapsemodels.py
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
Synapse models
"""

__all__ = [
    'static',
]


class SynapseModel:
    def __init__(self, model, **kwargs):
        self.model = model
        self.specs = kwargs

    def to_dict(self):
        return dict(self.specs, synapse_model=self.model)


class static(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('static_synapse', **kwargs)
