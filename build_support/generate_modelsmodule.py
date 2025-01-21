# -*- coding: utf-8 -*-
#
# generate_modelsmodule.py
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

"""Script to generate the modelsmodule implementation file.

This script is called during the run of CMake and generates the file
models/modelsmodule.cpp as well as the list of source files to be
compiled by CMake.
"""

import argparse
import itertools
import os
import sys
from pathlib import Path
from textwrap import dedent


def parse_commandline():
    """Parse the commandline arguments and put them into variables.

    There are three arguments to this script that can be given either as
    positional arguments or by their name.

    1. srcdir: the path to the top-level NEST source directory
    2. blddir: the path to the NEST build directory (-DCMAKE_INSTALL_PREFIX)
    3. models: the semicolon-separated list of models to be built in

    This function does not return anything, but instead it checks the
    commandline arguments and makes them available as global variables
    of the script. ``srcdir`` and ``blddir`` are set as they were
    given. The model list is split and commented models (i.e. ones
    that start with '#') are filtered out. The list is then made
    available under the name model_names.
    """

    global srcdir, blddir, model_names

    description = "Generate the implementation and header files for modelsmodule."
    parser = argparse.ArgumentParser(description=description)
    parser.add_argument("srcdir", type=str, help="the source directory of NEST")
    parser.add_argument("blddir", type=str, help="the build directory of NEST")
    parser.add_argument("models", type=str, help="the models to build into NEST")
    args = parser.parse_args()

    srcdir = args.srcdir
    blddir = args.blddir

    model_names = [model_file.strip() for model_file in args.models.split(";")]
    model_names = [model for model in model_names if model and not model.startswith("#")]


def get_models_from_file(model_file):
    """Extract model information from a given model file.

    This function applies a series of simple heuristics to find the
    preprocessor guards and the list of models in the file. Guards are
    expected to be in the form "#ifdef HAVE_<LIB>" with one guard per
    line. For the models, a list of unique pattern is used to infer
    the correct model type from the line in the file.

    The majority of neuron, device, and connection models are classes
    derived from a specific base class (like Node, ArchivingNode, or
    Connection) or from another model. The latter can only be detected
    if the base model has the same name as the file.

    The rate and binary neurons are typedefs for specialized template
    classes and multiple of such typedefs may be present in a file.

    Parameters
    ----------
    model_file: str
        The base file name (i.e. without extension) of the model file
        to get the information from.

    Returns
    -------
    tuple with two components:
        0: HAVE_* preprocessor guards required for the models in the file
        1: a zip of model types and model names found in the file and
           that need registering

    """

    model_patterns = {
        "public ArchivingNode": "neuron",
        "public StructuralPlasticityNode": "neuron",
        "public StimulationDevice": "stimulator",
        "public RecordingDevice": "recorder",
        "public DeviceNode": "devicelike",
        "public Connection": "connection",
        "public Node": "node",
        "public ClopathArchivingNode": "clopath",
        "public UrbanczikArchivingNode": "urbanczik",
        "public EpropArchivingNode": "neuron",
        "typedef binary_neuron": "binary",
        "typedef rate_": "rate",
    }

    fname = Path(srcdir) / "models" / f"{model_file}.h"
    if not os.path.exists(fname):
        print(f"ERROR: Model with name {model_file}.h does not exist", file=sys.stderr)
        sys.exit(128)

    guards = []
    names = []
    types = []
    with open(fname, "r") as file:
        for line in file:
            if line.startswith("#ifdef HAVE_"):
                guards.append(line.strip().split()[1])
            if line.startswith(f"class {model_file} : "):
                for pattern, mtype in model_patterns.items():
                    if pattern in line:
                        names.append(model_file)
                        types.append(mtype)
            if line.startswith("class") and line.strip().endswith(f" : public {model_file}"):
                names.append(line.split(" ", 2)[1])
                # try to infer the type of the derived model from the base model,
                # assuming that that was defined earlier in the file
                try:
                    types.append(types[names.index(model_file)])
                except (ValueError, KeyError) as e:
                    types.append("node")
            if line.startswith("typedef "):
                for pattern, mtype in model_patterns.items():
                    if pattern in line:
                        names.append(line.rsplit(" ", 1)[-1].strip()[:-1])
                        types.append(mtype)

    return tuple(guards), zip(types, names)


def get_include_and_model_data():
    """Create data dictionaries for include files and models.

    This function creates two nested dictionaries.

    The first (`includes`) contains the a mapping from model_type ->
    guards -> model_includes and is used in the code generation
    function to print all include lines. This basically corresponds to
    the list handed to the script as the `models` command line
    argument, but is enriched by model type information and the
    preprocessor guards needed for the individual include files.

    The second (`models`) is a mapping from model_type -> guards ->
    model_names and is used to generate the actual model registration
    lines.  model_names here is a list of models that is potentially
    larger than the ones coming in throught the `models` command line
    argument, as each file could contain multiple model definitions.

    This function does not return anything, but instead sets the
    global variables `includes` and `models` to be used by the code
    generation function.

    """

    global includes, models

    includes = {}
    models = {}

    for model_file in model_names:
        guards, model_types_names = get_models_from_file(model_file)
        for tp, nm in model_types_names:
            # Assemble a nested dictionary for the includes:
            fname = model_file + ".h"
            if tp in includes:
                if guards in includes[tp]:
                    includes[tp][guards].add(fname)
                else:
                    includes[tp][guards] = set([fname])
            else:
                includes[tp] = {guards: set([fname])}

            if (Path(srcdir) / "models" / f"{model_file}_impl.h").is_file():
                includes[tp][guards].add(f"{model_file}_impl.h")

            # Assemble a nested dictionary for the models:
            if tp in models:
                if guards in models[tp]:
                    models[tp][guards].append(nm)
                else:
                    models[tp][guards] = [nm]
            else:
                models[tp] = {guards: [nm]}


def start_guard(guards):
    """Print an #ifdef line with preprocessor guards if needed."""

    if guards:
        guard_str = " && ".join([f"defined( {guard} )" for guard in guards])
        return f"#if {guard_str}\n"
    else:
        return ""


def end_guard(guards):
    """Print an #endif line for the preprocessor guards if needed."""
    return "#endif\n" if guards else ""


def generate_modelsmodule():
    """Write the modelsmodule implementation out to file.

    This is a very straightforward function that prints several blocks
    of C++ code to the file modelsmodule.cpp in the `blddir` handed as
    a commandline argument to the script. The blocks in particular are

    1. the copyright header.
    2. a list of generic NEST includes
    3. the list of includes for the models to build into NEST
    4. the list of model registration lines for the models to build
       into NEST

    The code is enriched by structured C++ comments as to make
    debugging of the code generation process easier in case of errors.

    """

    fname = Path(srcdir) / "doc" / "copyright_header.cpp"
    with open(fname, "r") as file:
        copyright_header = file.read()

    fname = "models.cpp"
    modeldir = Path(blddir) / "models"
    modeldir.mkdir(parents=True, exist_ok=True)
    with open(modeldir / fname, "w") as file:
        file.write(copyright_header.replace("{{file_name}}", fname))
        file.write(
            dedent(
                """
            #include "models.h"

            // Generated includes
            #include "config.h"
        """
            )
        )

        for model_type, guards_fnames in includes.items():
            file.write(f"\n// {model_type.capitalize()} models\n")
            for guards, fnames in guards_fnames.items():
                file.write(start_guard(guards))
                for fname in fnames:
                    file.write(f'#include "{fname}"\n')
                file.write(end_guard(guards))

        file.write("\nvoid nest::register_models()\n{")

        for model_type, guards_mnames in models.items():
            file.write(f"\n  // {model_type.capitalize()} models\n")
            for guards, mnames in guards_mnames.items():
                file.write(start_guard(guards))
                for mname in mnames:
                    file.write(f'  register_{mname}( "{mname}" );\n')
                file.write(end_guard(guards))

        file.write("}")


def print_model_sources():
    """Hand back the list of model source files to CMake.

    In addition to the header file names handed to the script in the
    form of the `models` commandline argument, this function searches
    for corresponding implementation files with the extensions `.cpp`
    and `_impl.h`. The list of models is printed as a CMake list,
    i.e. as a semicolon separated string.

    """

    model_sources = []
    source_files = os.listdir(Path(srcdir) / "models")
    for model_name in model_names:
        source_candidates = [model_name + suffix for suffix in (".cpp", ".h", "_impl.h")]
        model_sources.extend([f for f in source_files if f in source_candidates])
    print(";".join(model_sources), end="")


if __name__ == "__main__":
    parse_commandline()
    get_include_and_model_data()
    generate_modelsmodule()
    print_model_sources()
