# -*- coding: utf-8 -*-
#
# conf.py
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
import sys
import os
import re
import subprocess
import glob

from pathlib import Path

import nbformat

from nbconvert import PythonExporter
# This is using the branch ebrains-button (set as default on jessica-mitchell github).
# This should use either stable or master once done
link_puller = ("https://lab.ebrains.eu/hub/user-redirect/git-pull?repo=https%3A%2F%2Fgithub.com"
              "%2Fnest%2Fnest-simulator&urlpath=lab%2Ftree%2Fnest-simulator%2Fpynest"
              "%2Fexamples%2Fnotebooks%2Fone_neuron_with_noise.ipynb&branch=v3.3")

filepath = "pynest-examples/"
# button syntax to insert into notebook
md_ebrains_button = "[![EBRAINS Notebook](https://nest-simulator.org/TryItOnEBRAINS.png)](nblink)\n"

python_button = "[![Download Python file](../../static/img/python-download.png)](pythonlink)\n"

def make_link(filename, link_puller):

    # get the filename only from the path
    name = os.path.basename(filename)
    # get the path of file only
    path2dir = os.path.dirname(filename)
    # get the last segement of path
    path2sub = os.path.basename(path2dir)
    # Create proper links for nbgitpuller for each notebook
    # check if subdirectory
    if path2sub != "pynest-examples":
        link_sub_path = path2sub + "%2F" + name
        links = link_puller.replace('one_neuron_with_noise.ipynb', link_sub_path)
    else:

        links = link_puller.replace('one_neuron_with_noise.ipynb', name)

    return links

for filename in Path(filepath).rglob('*.ipynb'):
    with open(filename, 'r') as checkfile:
        content = checkfile.read()
        if "EBRAINS Notebook" in content:
            continue

# if filename pynest-examples/
# filename contains path
# name contains only file name
# need to find pynest-examples/filename an dpynest-examples/something/filename

# I need to replace the link puller with either the filename or the subdirectory + filename
#
############# Insert subdirectory (if exists) and filename into nblinknpuller link ###########
    links = make_link(filename, link_puller)
############## create ebrains link to the markdown styled button for the notebook ##############

    button = md_ebrains_button.replace('nblink', links)


########## convert ipynb to py ##############
    exporter = PythonExporter()
    (source, meta) = exporter.from_filename(filename)
    # cleanup python file: remove excess lines produced in python
    source = source.replace("# In[ ]:", "")
    source = source.replace("# \n", "")
    # ensure name is the same for python file and ipynb
    base = os.path.splitext(name)[0]
    pyname = base + '.py'


    # create link for python button
    pybutton = python_button.replace('pythonlink', pyname)

    with open(os.path.dirname(filename) + '/' + pyname, 'w') as outfile:
        outfile.write(source)
    # append the buttons for nblinkpuller, python download to each notebook
    nb = nbformat.read(filename, as_version=4)
    cells = [nbformat.v4.new_markdown_cell(button), nbformat.v4.new_markdown_cell(pybutton)]
    nb['cells'].extend(cells)
    nbformat.write(nb, filename)


# 1. branch = tag or master / if tag must be updated with each release / if master then notebook might be
# incompatible (possibly)
# 2. EBRAINS nest install - currently 3.0 (actual3.3), so we are already behind, and again if we
# use recent release we might find incompatibilities
# 3. Running script is slow, should it be applied to notebooks one time and only updated
# manually / or in CI if examples folder changss
