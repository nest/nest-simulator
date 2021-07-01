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
    'bernoulli',
    'clopath',
    'cont_delay',
    'copy_synapse_class',
    'diffusion',
    'gap_junction',
    'ht',
    'jonke',
    'quantal_stp',
    'rate_connection_delayed',
    'rate_connection_instantaneous',
    'static',
    'static_hom_w',
    'static_hpc',
    'stdp_dopamine',
    'stdp_nn_pre_centered',
    'stdp_nn_restr',
    'stdp_nn_symm',
    'stdp_pl_hom',
    'stdp_pl_hom_hpc',
    'stdp',
    'stdp_facetshw_hom',
    'stdp_hom',
    'stdp_triplet',
    'tsodyks2',
    'tsodyks',
    'tsodyks_hom',
    'urbanczik',
    'vogels_sprekeler',
    'SynapseModel'
]


class SynapseModel:
    def __init__(self, synapse_model, **kwargs):
        self.synapse_model = synapse_model
        self.specs = kwargs

    def to_dict(self):
        return dict(self.specs, synapse_model=self.synapse_model)

    def __getattr__(self, attr):
        return super().__getattribute__(attr)

    def __setattr__(self, attr, value):
        if attr in ['synapse_model', 'specs']:
            return super().__setattr__(attr, value)
        else: self.specs[attr] = value
    
    def __str__(self):
        return f'synapse_model: {self.synapse_model}, specs: {self.specs}'


def copy_synapse_class(model,kwargs):
    def model_init(self, model, kwargs):
        SynapseModel.__init__(self, model, **kwargs)

    NewSynapse = type(model, (SynapseModel,), {"__init__": model_init})

    synapse = NewSynapse(model, kwargs)
    return synapse


class bernoulli(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('bernoulli_synapse', **kwargs)


class clopath(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('clopath_synapse', **kwargs)


class cont_delay(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('cont_delay_synapse', **kwargs)


class diffusion(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('diffusion_connection', **kwargs)


class gap_junction(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('gab_junction', **kwargs)


class ht(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('ht_synapse', **kwargs)


class jonke(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('jonke_synapse', **kwargs)


class quantal_stp(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('quantal_stp_synapse', **kwargs)


class rate_connection_delayed(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('rate_connection_delayed', **kwargs)


class rate_connection_instantaneous(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('rate_connection_instantaneous', **kwargs)


class static(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('static_synapse', **kwargs)


class static_hom_w(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('static_synapse_hom_w', **kwargs)


class static_hpc(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('static_synapse_hpc', **kwargs)


class stdp_dopamine(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('stdp_dopamine_synapse', **kwargs)


class stdp_nn_pre_centered(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('stdp_nn_pre_centered_synapse', **kwargs)


class stdp_nn_restr(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('stdp_nn_restr_synapse', **kwargs)


class stdp_nn_symm(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('stdp_nn_symm_synapse', **kwargs)


class stdp_pl_hom(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('stdp_pl_synapse_hom', **kwargs)


class stdp_pl_hom_hpc(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('stdp_pl_synapse_hom_hpc', **kwargs)


class stdp(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('stdp_synapse', **kwargs)


class stdp_facetshw_hom(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('stdp_facetshw_synapse_hom', **kwargs)


class stdp_hom(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('stdp_synapse_hom', **kwargs)


class stdp_triplet(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('stdp_triplet_synapse', **kwargs)


class tsodyks2(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('tsodyks2_synapse', **kwargs)


class tsodyks(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('tsodyks_synapse', **kwargs)


class tsodyks_hom(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('tsodyks_synapse_hom', **kwargs)


class urbanczik(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('urbanczik_synapse', **kwargs)


class vogels_sprekeler(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('vogels_sprekeler_synapse', **kwargs)
