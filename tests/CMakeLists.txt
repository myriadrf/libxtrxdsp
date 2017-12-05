# test files
include_directories(..)

set_source_files_properties(test_xtrxdsp_sc32i_iq16.c PROPERTIES COMPILE_FLAGS "-O2 ${GENERIC_TUNE}")
add_executable(test_xtrxdsp_sc32i_iq16 test_xtrxdsp_sc32i_iq16.c)
target_link_libraries(test_xtrxdsp_sc32i_iq16 xtrxdsp ${SYSTEM_LIBS})


set_source_files_properties(test_filter.c PROPERTIES COMPILE_FLAGS "-O2 ${GENERIC_TUNE}")
add_executable(test_filter test_filter.c)
target_link_libraries(test_filter xtrxdsp m ${SYSTEM_LIBS})


install(TARGETS test_filter test_xtrxdsp_sc32i_iq16 DESTINATION ${XTRXDSP_UTILS_DIR})
