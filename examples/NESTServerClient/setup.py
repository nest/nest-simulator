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

# NESTServerClient --- A Client for NEST Server

from distutils.core import setup

setup(name='NESTServerClient',
      version='0.1',
      description=('NESTServerClient sends JSON requests to NEST Server.'),
      author='Sebastian Spreizer',
      author_email='spreizer@web.de',
      url='https://www.nest-simulator.org',
      license='GNU Public License v2 or later',
      packages=['NESTServerClient', 'NESTServerClient.examples'],
      package_dir={'NESTServerClient': ''}
      )
