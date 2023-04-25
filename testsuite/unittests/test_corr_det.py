# -*- coding: utf-8 -*-
#
# test_corr_det.py
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
Name: testsuite::test_corr_det - minimal test of correlation detector

Synopsis: (test_corr_det) run -> dies if assertion fails

Description:
  Feeds correlation detector with two hand-crafted spike trains with
  known correlation. Correlation detector parameters are set in model.

Remarks:
  The test does not test weighted correlations.

Author: July 2008, Plesser
SeeAlso: correlation_detector
"""
import nest
import pytest


@pytest.fixture(autouse=True)
def prepare():
    nest.ResetKernel()


def test_changing_params():
    new_params = {"d_tau": 2.0, "tau_max": 20.0}

    original_model_instance = nest.Create("correlation_detector")
    original_model_instance.set(new_params)

    nest.SetDefaults("correlation_detector", new_params)
    modified_model_instance = nest.Create("correlation_detector")

    assert modified_model_instance.get() == original_model_instance.get()
