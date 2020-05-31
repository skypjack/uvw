#
# Doxygen configuration (documentation)
#

set(TARGET_DOCS docs)

set(DOXY_IN_FILE doxy.in)

set(DOXY_SOURCE_DIRECTORY ${uvw_SOURCE_DIR}/src)
set(DOXY_DOCS_DIRECTORY ${uvw_SOURCE_DIR}/docs)
set(DOXY_OUTPUT_DIRECTORY ${uvw_BINARY_DIR}/docs)
set(DOXY_CFG_FILE doxy.cfg)

configure_file(${DOXY_IN_FILE} ${DOXY_CFG_FILE} @ONLY)

add_custom_target(
    ${TARGET_DOCS}
    COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/${DOXY_CFG_FILE}
    WORKING_DIRECTORY ${uvw_SOURCE_DIR}
    VERBATIM
    SOURCES ${DOXY_IN_FILE}
)

install(
    DIRECTORY ${DOXY_OUTPUT_DIRECTORY}/html
    DESTINATION share/${PROJECT_NAME}-${PROJECT_VERSION}/
)
