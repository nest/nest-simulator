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

import copy

__all__ = [
    'bernoulli',
    'bernoulli_hpc',
    'bernoulli_lbl',
    'clopath',
    'clopath_hpc',
    'clopath_lbl',
    'cont_delay',
    'cont_delay_hpc',
    'cont_delay_lbl',
    'copy_synapse_class',
    'diffusion_connection',
    'diffusion_connection_lbl',
    'gap_junction',
    'gap_junction_lbl',
    'ht',
    'ht_hpc',
    'ht_lbl',
    'jonke',
    'jonke_hpc',
    'jonke_lbl',
    'quantal_stp',
    'quantal_stp_hpc',
    'quantal_stp_lbl',
    'rate_connection_delayed',
    'rate_connection_delayed_lbl',
    'rate_connection_instantaneous',
    'rate_connection_instantaneous_lbl',
    'static',
    'static_hom_w',
    'static_hom_w_hpc',
    'static_hom_w_lbl',
    'static_hpc',
    'static_lbl',
    'stdp',
    'stdp_dopamine',
    'stdp_dopamine_hpc',
    'stdp_dopamine_lbl',
    'stdp_facetshw_hom',
    'stdp_facetshw_hom_hpc',
    'stdp_facetshw_hom_lbl',
    'stdp_hom',
    'stdp_hom_hpc',
    'stdp_hom_lbl',
    'stdp_hpc',
    'stdp_lbl',
    'stdp_nn_pre_centered',
    'stdp_nn_pre_centered_hpc',
    'stdp_nn_pre_centered_lbl',
    'stdp_nn_restr',
    'stdp_nn_restr_hpc',
    'stdp_nn_restr_lbl',
    'stdp_nn_symm',
    'stdp_nn_symm_hpc',
    'stdp_nn_symm_lbl',
    'stdp_pl_hom',
    'stdp_pl_hom_hpc',
    'stdp_pl_hom_lbl',
    'stdp_triplet',
    'stdp_triplet_hpc',
    'stdp_triplet_lbl',
    'tsodyks2',
    'tsodyks2_hpc',
    'tsodyks2_lbl',
    'tsodyks',
    'tsodyks_hom',
    'tsodyks_hom_hpc',
    'tsodyks_hom_lbl',
    'tsodyks_hpc',
    'tsodyks_lbl',
    'urbanczik',
    'urbanczik_hpc',
    'urbanczik_lbl',
    'vogels_sprekeler',
    'vogels_sprekeler_hpc',
    'vogels_sprekeler_lbl',
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

    def clone(self):
        return copy.deepcopy(self)

    def __str__(self):
        return f'synapse_model: {self.synapse_model}, specs: {self.specs}'


def copy_synapse_class(model):
    def model_init(self, model):
        SynapseModel.__init__(self, model)

    NewSynapse = type(model, (SynapseModel,), {"__init__": model_init})

    synapse = NewSynapse(model)
    return synapse


class bernoulli(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('bernoulli_synapse', **kwargs)


class bernoulli_hpc(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('bernoulli_synapse_hpc', **kwargs)


class bernoulli_lbl(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('bernoulli_synapse_lbl', **kwargs)


class clopath(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('clopath_synapse', **kwargs)


class clopath_hpc(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('clopath_synapse_hpc', **kwargs)


class clopath_lbl(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('clopath_synapse_lbl', **kwargs)


class cont_delay(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('cont_delay_synapse', **kwargs)


class cont_delay_hpc(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('cont_delay_synapse_hpc', **kwargs)


class cont_delay_lbl(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('cont_delay_synapse_lbl', **kwargs)


class diffusion_connection(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('diffusion_connection', **kwargs)


class diffusion_connection_lbl(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('diffusion_connection_lbl', **kwargs)


class gap_junction(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('gap_junction', **kwargs)


class gap_junction_lbl(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('gab_junction_lbl', **kwargs)


class ht(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('ht_synapse', **kwargs)


class ht_hpc(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('ht_synapse_hpc', **kwargs)


class ht_lbl(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('ht_synapse_lbl', **kwargs)


class jonke(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('jonke_synapse', **kwargs)


class jonke_hpc(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('jonke_synapse_hpc', **kwargs)


class jonke_lbl(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('jonke_synapse_lbl', **kwargs)


class quantal_stp(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('quantal_stp_synapse', **kwargs)


class quantal_stp_hpc(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('quantal_stp_synapse_hpc', **kwargs)


class quantal_stp_lbl(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('quantal_stp_synapse_lbl', **kwargs)


class rate_connection_delayed(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('rate_connection_delayed', **kwargs)


class rate_connection_delayed_lbl(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('rate_connection_delayed_lbl', **kwargs)


class rate_connection_instantaneous(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('rate_connection_instantaneous', **kwargs)


class rate_connection_instantaneous_lbl(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('rate_connection_instantaneous_lbl', **kwargs)


class static(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('static_synapse', **kwargs)


class static_hom_w(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('static_synapse_hom_w', **kwargs)


class static_hom_w_hpc(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('static_synapse_hom_w_hpc', **kwargs)


class static_hom_w_lbl(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('static_synapse_hom_w_lbl', **kwargs)


class static_hpc(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('static_synapse_hpc', **kwargs)


class static_lbl(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('static_synapse_lbl', **kwargs)


class stdp(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('stdp_synapse', **kwargs)


class stdp_dopamine(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('stdp_dopamine_synapse', **kwargs)


class stdp_dopamine_hpc(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('stdp_dopamine_synapse_hpc', **kwargs)


class stdp_dopamine_lbl(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('stdp_dopamine_synapse_lbl', **kwargs)


class stdp_facetshw_hom(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('stdp_facetshw_synapse_hom', **kwargs)


class stdp_facetshw_hom_hpc(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('stdp_facetshw_synapse_hom_hpc', **kwargs)


class stdp_facetshw_hom_lbl(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('stdp_facetshw_synapse_hom_lbl', **kwargs)


class stdp_hom(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('stdp_synapse_hom', **kwargs)


class stdp_hom_hpc(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('stdp_synapse_hom_hpc', **kwargs)


class stdp_hom_lbl(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('stdp_synapse_hom_lbl', **kwargs)


class stdp_hpc(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('stdp_synapse_hpc', **kwargs)


class stdp_lbl(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('stdp_synapse_lbl', **kwargs)


class stdp_nn_pre_centered(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('stdp_nn_pre_centered_synapse', **kwargs)


class stdp_nn_pre_centered_hpc(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('stdp_nn_pre_centered_synapse_hpc', **kwargs)


class stdp_nn_pre_centered_lbl(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('stdp_nn_pre_centered_synapse_lbl', **kwargs)


class stdp_nn_restr(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('stdp_nn_restr_synapse', **kwargs)


class stdp_nn_restr_hpc(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('stdp_nn_restr_synapse_hpc', **kwargs)


class stdp_nn_restr_lbl(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('stdp_nn_restr_synapse_lbl', **kwargs)


class stdp_nn_symm(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('stdp_nn_symm_synapse', **kwargs)


class stdp_nn_symm_hpc(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('stdp_nn_symm_synapse_hpc', **kwargs)


class stdp_nn_symm_lbl(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('stdp_nn_symm_synapse_lbl', **kwargs)


class stdp_pl_hom(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('stdp_pl_synapse_hom', **kwargs)


class stdp_pl_hom_hpc(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('stdp_pl_synapse_hom_hpc', **kwargs)


class stdp_pl_hom_lbl(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('stdp_pl_synapse_hom_lbl', **kwargs)


class stdp_triplet(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('stdp_triplet_synapse', **kwargs)


class stdp_triplet_hpc(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('stdp_triplet_synapse_hpc', **kwargs)


class stdp_triplet_lbl(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('stdp_triplet_synapse_lbl', **kwargs)


class tsodyks2(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('tsodyks2_synapse', **kwargs)


class tsodyks2_hpc(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('tsodyks2_synapse_hpc', **kwargs)


class tsodyks2_lbl(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('tsodyks2_synapse_lbl', **kwargs)


class tsodyks(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('tsodyks_synapse', **kwargs)


class tsodyks_hom(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('tsodyks_synapse_hom', **kwargs)


class tsodyks_hom_hpc(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('tsodyks_synapse_hom_hpc', **kwargs)


class tsodyks_hom_lbl(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('tsodyks_synapse_hom_lbl', **kwargs)


class tsodyks_hpc(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('tsodyks_synapse_hpc', **kwargs)


class tsodyks_lbl(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('tsodyks_synapse_lbl', **kwargs)


class urbanczik(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('urbanczik_synapse', **kwargs)


class urbanczik_hpc(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('urbanczik_synapse_hpc', **kwargs)


class urbanczik_lbl(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('urbanczik_synapse_lbl', **kwargs)


class vogels_sprekeler(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('vogels_sprekeler_synapse', **kwargs)


class vogels_sprekeler_hpc(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('vogels_sprekeler_synapse_hpc', **kwargs)


class vogels_sprekeler_lbl(SynapseModel):
    def __init__(self, **kwargs):
        super().__init__('vogels_sprekeler_synapse_lbl', **kwargs)
