# -*- coding: utf-8 -*-
#
# setup.py
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

'''Setup the readthedocs environment
'''

# import codecs
import os
# from setuptools import setup, find_packages
# import sphinx_gallery
import shlex
import subprocess



# XXX : don't force requirements in setup.py as it tends to break
#       people's setups
install_requires = []

if os.environ.get('READTHEDOCS', None) == 'True':
    # So that we can build documentation using seaborn
    install_requires = ['seaborn']

    build_line = "sh ./build.sh"
    args = shlex.split(build_line)
    print(args)
    p = subprocess.Popen(args)

    source_line = "sh ./result/bin/nest_vars.sh"
    args = shlex.split(source_line)
    print(args)
    p = subprocess.Popen(args)



