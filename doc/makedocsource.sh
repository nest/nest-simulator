#!/usr/bin/env bash

# makedocsource.sh
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


# This shell script is part of the NEST Travis CI build and test environment.
# It is invoked by the top-level Travis script '.travis.yml'.
#
# NOTE: This shell script is tightly coupled to Python script
#       'extras/parse_travis_log.py'.
#       Any changes to message numbers (MSGBLDnnnn) or the variable name
#      'file_names' have effects on the build/test-log parsing process.

cd ../extras/help_generator
python webdoc.py ../../pynest/examples ../../doc/examples ../../doc/examples

# Now integrated in webdoc.py
# cd ../../doc/ipynb/
# ipython nbconvert --to markdown exact-integration.ipynb
