# ConnPlotter --- A Tool to Generate Connectivity Pattern Matrices
#
# This file is part of ConnPlotter.
#
# Copyright (C) 2009 Hans Ekkehard Plesser/UMB
#
# ConnPlotter is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# ConnPlotter is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with ConnPlotter.  If not, see <http://www.gnu.org/licenses/>.

from distutils.core import setup

setup(name        = 'ConnPlotter',
      version     = '0.7a',
      description = 'ConnPlotter is a tool to create connectivity pattern tables',
      author      = 'Hans Ekkehard Plesser (Idea: Eilen Nordlie)',
      author_email= 'hans.ekkehard.plesser@umb.no',
      url         = 'http://www.nest-initiative.org',
      license     = 'GNU Public License v. 2 or later',
      packages    = ['ConnPlotter', 'ConnPlotter.examples'],
      package_dir = {'ConnPlotter': ''}
      )
