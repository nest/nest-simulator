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

# ConnPlotter --- A Tool to Generate Connectivity Pattern Matrices

from distutils.core import setup

setup(name='ConnPlotter',
      version='0.7a0',
      description=('ConnPlotter is a tool to create ' +
                   'connectivity pattern tables'),
      author='Hans Ekkehard Plesser (Idea: Eilen Nordlie)',
      author_email='hans.ekkehard.plesser@umb.no',
      url='https://www.nest-simulator.org',
      license='GNU Public License v2 or later',
      packages=['ConnPlotter', 'ConnPlotter.examples'],
      package_dir={'ConnPlotter': ''}
      )
