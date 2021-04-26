# thirdparty/CMakeLists.txt
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

set( thirdparty_headers
    compose.hpp
    randutils.hpp
    Random123/gsl_microrng.h
    Random123/aes.h
    Random123/array.h
    Random123/u01fixedpt.h
    Random123/philox.h
    Random123/threefry.h
    Random123/ars.h
    Random123/conventional/gsl_cbrng.h
    Random123/features/open64features.h
    Random123/features/xlcfeatures.h
    Random123/features/nvccfeatures.h
    Random123/features/metalfeatures.h
    Random123/features/sunprofeatures.h
    Random123/features/sse.h
    Random123/features/pgccfeatures.h
    Random123/features/openclfeatures.h
    Random123/features/iccfeatures.h
    Random123/features/compilerfeatures.h
    Random123/features/clangfeatures.h
    Random123/features/msvcfeatures.h
    Random123/features/gccfeatures.h
    )


install( FILES compose.hpp randutils.hpp DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/nest)
install( DIRECTORY Random123/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/nest/Random123
         FILES_MATCHING PATTERN "*.h" )

