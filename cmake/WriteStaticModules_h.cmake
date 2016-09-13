# cmake/WriteStaticModules_h.cmake
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

# The following instructions write the file nest/static_modules.h in the
# binary directory, aka the build directory.

# write static_modules.h
function( NEST_WRITE_STATIC_MODULE_HEADER filename )
  file( WRITE "${filename}" "#ifndef STATIC_MODULES_H\n" )
  file( APPEND "${filename}" "#define STATIC_MODULES_H\n\n" )
  file( APPEND "${filename}" "// Add all in source modules:\n" )
  foreach ( mod ${SLI_MODULES} )
    file( APPEND "${filename}" "#include \"${mod}module.h\"\n" )
  endforeach ()

  # when we build statically, we need to add headers and addmodule for external modules
  # just as if it were a in source module.
  if ( static-libraries AND external-modules )
    file( APPEND "${filename}" "\n// Add all external modules:\n" )
    foreach ( mod ${EXTERNAL_MODULE_INCLUDES} )
      get_filename_component( mod_header ${mod} NAME )
      file( APPEND "${filename}" "#include \"${mod_header}\"\n" )
    endforeach ()
  endif ()
  file( APPEND "${filename}" "\n// Others\n" )
  file( APPEND "${filename}" "#include \"interpret.h\"\n\n" )

  # start `add_static_modules` function
  file( APPEND "${filename}" "void add_static_modules( SLIInterpreter& engine )\n{\n" )
  file( APPEND "${filename}" "  // Add all in source modules:\n" )
  foreach ( mod ${SLI_MODULES} )
    # get class name, is always in nest namespace
    file( STRINGS "${PROJECT_SOURCE_DIR}/${mod}/${mod}module.h" module_class_string REGEX "class.*: public SLIModule" )
    string( REGEX REPLACE "class ([a-zA-Z0-9_]+) : public SLIModule" "\\1" module_class ${module_class_string} )
    file( APPEND "${filename}" "  engine.addmodule( new nest::${module_class}() );\n" )
  endforeach ()

  # when we build statically, we need to add headers and addmodule for external modules
  # just as if it were a in source module.
  if ( static-libraries AND external-modules )
    file( APPEND "${filename}" "\n  // Add all external modules:\n" )
    foreach ( mod_header ${EXTERNAL_MODULE_INCLUDES} )
      # get namespace:
      file( STRINGS "${mod_header}" module_namespace_string REGEX "^namespace.*" )
      if ( NOT module_namespace_string )
        message( FATAL_ERROR "Could not find namespace in '${mod_header}'." )
      endif ()
      string( REGEX REPLACE "namespace ([a-zA-Z0-9_]+)" "\\1" module_namespace ${module_namespace_string} )

      # get class name
      file( STRINGS "${mod_header}" module_class_string REGEX "^class.*: public SLIModule" )
      if ( NOT module_class_string )
        message( FATAL_ERROR "Could not find class that extends SLIModule in '${mod_header}'." )
      endif ()
      string( REGEX REPLACE "class ([a-zA-Z0-9_]+) : public SLIModule" "\\1" module_class ${module_class_string} )
      file( APPEND "${filename}" "  engine.addmodule( new ${module_namespace}::${module_class}() );\n" )
    endforeach ()
  endif ()

  file( APPEND "${filename}" "}\n\n#endif\n" )
endfunction()
