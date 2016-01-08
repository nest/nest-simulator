install( FILES run_examples.sh
    DESTINATION ${INSTALL_DOC_DIR} )

install( FILES sli/nestrc.sli
    DESTINATION ${INSTALL_DOC_DIR}/examples
    )

install( DIRECTORY nest/
    DESTINATION ${INSTALL_DOC_DIR}/examples
    USE_SOURCE_PERMISSIONS
    )
