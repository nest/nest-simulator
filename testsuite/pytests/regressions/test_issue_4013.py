# -*- coding: utf-8 -*-
#
# test_issue_4013.py
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
Regression test for Issue #4013 (GitHub).

A recording backend registered by a module must be usable, also after a change of the number of threads following
``Install()``. The test compiles such a module with the compiler and flags reported by ``nest-config``.
"""

import platform
import shlex
import shutil
import subprocess

import nest
import pytest

MODULE_SOURCE = """
#include "io_manager_impl.h"
#include "kernel_manager.h"
#include "nest_extension_interface.h"
#include "recording_backend_screen.h"

class backendmodule : public nest::NESTExtensionInterface
{
public:
  void
  initialize() override
  {
    nest::kernel().io_manager.register_recording_backend< nest::RecordingBackendScreen >( "screen2" );
  }
};

backendmodule backendmodule_LTX_module;
"""


def nest_config(*args):
    return subprocess.run(["nest-config", *args], check=True, capture_output=True, text=True).stdout.split()


@pytest.fixture(scope="module")
def backend_module(tmp_path_factory):
    """Compile the module into a temporary directory."""

    if shutil.which("nest-config") is None:
        pytest.skip("requires nest-config")
    directory = tmp_path_factory.mktemp("issue4013")
    source = directory / "backendmodule.cpp"
    source.write_text(MODULE_SOURCE)
    link = ["-undefined", "dynamic_lookup"] if platform.system() == "Darwin" else []
    cmd = [
        *nest_config("--compiler"),
        *nest_config("--cflags"),
        *nest_config("--includes"),
        "-fPIC",
        "-shared",
        str(source),
        "-o",
        str(directory / "backendmodule.so"),
        *link,
    ]
    result = subprocess.run(cmd, capture_output=True, text=True, check=False)
    if result.returncode != 0:
        pytest.skip(f"cannot compile a module ({shlex.join(cmd)}):\n{result.stderr}")
    return str(directory / "backendmodule.so")


def record_to_module_backend():
    """Record the spikes of a neuron with the backend of the module."""

    assert "screen2" in nest.GetKernelStatus("recording_backends")
    neuron = nest.Create("iaf_psc_alpha", params={"I_e": 500.0})
    recorder = nest.Create("spike_recorder", params={"record_to": "screen2"})
    nest.Connect(neuron, recorder)
    nest.Simulate(50.0)


def test_module_backend_records(backend_module):
    """Ensure that the backend of the module is initialized on registration and can record."""

    nest.ResetKernel()
    nest.Install(backend_module)
    record_to_module_backend()
    nest.ResetKernel()


@pytest.mark.skipif_missing_threads
def test_thread_change_after_install_keeps_module_backend(backend_module):
    """Ensure that the backend of the module survives a change of the number of threads and can record."""

    nest.ResetKernel()
    nest.Install(backend_module)
    nest.local_num_threads = 2
    record_to_module_backend()
    nest.ResetKernel()
