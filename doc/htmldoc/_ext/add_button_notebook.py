# -*- coding: utf-8 -*-
#
# add_button_notebook.py
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

import glob
import os
import sys
from pathlib import Path

from sphinx.application import Sphinx


def add_button_to_examples(app, env, docnames):
    """Find all examples and include a link to launch notebook.

    Function finds all restructured text files in auto_examples
    and injects the multistring prolog, which is rendered
    as a button link in HTML. The target is set to a Jupyter notebook of
    the same name and a service to run it.
    The nameholder in the string is replaced with the file name.

    The rst files are generated at build time by Sphinx_gallery.
    The notebooks that the target points to are linked with
    services (like EBRAINS JupyterHub) that runs notebooks using nbgitpuller.
    See https://hub.jupyter.org/nbgitpuller/link.html
    The notebooks are located in the repository nest/nest-simulator-examples/.
    The notebooks are generated from the CI workflow of NEST
    on GitHub, which converts the source Python files to .ipynb.

    The link to run the notebook is rendered in an image within a card directive.
    """
    example_prolog = """
.. only:: html

----

 Run this example as a Jupyter notebook:

  .. card::
    :width: 25%
    :margin: 2
    :text-align: center
    :link: https://lab.ebrains.eu/hub/user-redirect/\
git-pull?repo=https%3A%2F%2Fgithub.com%2Fnest%2Fnest-simulator-examples&urlpath=lab\
%2Ftree%2Fnest-simulator-examples%2Fnotebooks%2Fnotebooks%2Ffilepath.ipynb&branch=main
    :link-alt: JupyterHub service

    .. image:: https://nest-simulator.org/TryItOnEBRAINS.png


.. grid:: 1 1 1 1
   :padding: 0 0 2 0

   .. grid-item::
     :class: sd-text-muted
     :margin: 0 0 3 0
     :padding: 0 0 3 0
     :columns: 4

     See :ref:`our guide <run_jupyter>` for more information and troubleshooting.

----
"""
    # Find all relevant files
    # Inject prolog into Python example
    files = list(Path("auto_examples/").rglob("*.rst"))
    for file in files:
        # Skip index files and benchmark file. These files do not have notebooks that can run
        # on the service.
        if file.stem == "index" or file.stem == "hpc_benchmark":
            continue

        with open(file, "r") as f:
            parent = Path("auto_examples/")
            path2example = os.path.relpath(file, parent)
            path2example = os.path.splitext(path2example)[0]
            path2example = path2example.replace("/", "%2F")
            prolog = example_prolog.replace("filepath", path2example)

            lines = f.readlines()

        # make sure rst file is auto generated Python file
        check_line = lines[1]
        if check_line.startswith(".."):
            # find the first heading of the file.
            for i, item in enumerate(lines):
                if item.startswith("-----"):
                    break

            # insert prolog into rst file after heading
            lines.insert(i + 1, prolog + "\n")

            with open(file, "w") as f:
                lines = "".join(lines)
                f.write(lines)
        else:
            continue


def setup(app):
    app.connect("env-before-read-docs", add_button_to_examples)

    return {
        "version": "0.1",
        "parallel_read_safe": True,
        "parallel_write_safe": True,
    }
