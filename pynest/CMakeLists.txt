message("Hi from pynest...")

if ( WITH_PYTHON )
  include( UseCython )
  set_source_files_properties(
    ${CMAKE_CURRENT_SOURCE_DIR}/pynestkernel.pyx
    PROPERTIES CYTHON_IS_CXX TRUE )

  cython_add_module( pynestkernel pynestkernel.pyx )

  target_link_libraries(pynestkernel nest_lib nestutil nestkernel random sli_lib ${SLI_MODULES})

  target_include_directories(pynestkernel PRIVATE
    ${PROJECT_BINARY_DIR}/libnestutil
    ${PROJECT_SOURCE_DIR}/libnestutil
    ${PROJECT_SOURCE_DIR}/nest
    ${PROJECT_SOURCE_DIR}/nestkernel
    ${PROJECT_SOURCE_DIR}/librandom
    ${PROJECT_SOURCE_DIR}/sli
    ${STATIC_MODULES_INDCLUDE_DIRS}
  )

  target_compile_definitions(pynestkernel PRIVATE 
    -D_IS_PYNEST
  )

  install(CODE "execute_process(COMMAND ${PYTHON} setup.py install --prefix=${CMAKE_INSTALL_PREFIX} WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})")
  install(TARGETS pynestkernel DESTINATION ${INSTALL_LIB_DIR}/python${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}/site-packages/nest/)
  install(FILES ${PROJECT_BINARY_DIR}/pynest/do_tests.py
    DESTINATION ${INSTALL_DATA_DIR}/extras)

  set(pynest_examples
    examples/balancedneuron.py 
    examples/brunel_alpha_nest.py 
    examples/brunel_alpha_numpy.py 
    examples/brunel_delta_nest.py 
    examples/BrodyHopfield.py 
    examples/brette_gerstner_fig_2c.py 
    examples/brette_gerstner_fig_3d.py 
    examples/CampbellSiegert.py 
    examples/csa_example.py 
    examples/if_curve.py 
    examples/intrinsic_currents_spiking.py 
    examples/intrinsic_currents_subthreshold.py
    examples/mc_neuron.py 
    examples/multimeter_file.py 
    examples/one_neuron.py 
    examples/one_neuron_with_noise.py 
    examples/pulsepacket.py 
    examples/sinusoidal_gamma_generator.py
    examples/sinusoidal_poisson_generator.py
    examples/testiaf.py 
    examples/tsodyks_depressing.py 
    examples/tsodyks_facilitating.py 
    examples/vinit_example.py
  )

  install(FILES ${pynest_examples}
    DESTINATION ${INSTALL_DOC_DIR}/examples/pynest)

  install(DIRECTORY examples/LeNovere_2012
    DESTINATION ${INSTALL_DOC_DIR}/examples/pynest)

endif( WITH_PYTHON )
