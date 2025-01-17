enable_testing()

add_executable(test_cc test_cc.cpp
        ../src/common.cpp
        ../src/spmatops.cpp
        ../src/ecc.cpp
        ../src/matds.cpp
        ../src/graphs.cpp)

target_link_libraries(test_cc PRIVATE zf_log)

add_test(NAME test_cc COMMAND test_cc)

add_executable(test_matrix_io test_matrix_io.cpp
        ../src/common.cpp
        ../src/matds.cpp
        ../src/matio.cpp)

target_link_libraries(test_matrix_io PRIVATE mmio)
target_link_libraries(test_matrix_io PRIVATE zf_log)

add_test(NAME test_matrix_io COMMAND test_matrix_io)

add_executable(test_spmatops test_spmatops.cpp
        ../src/common.cpp
        ../src/spmatops.cpp
        ../src/matds.cpp)

target_link_libraries(test_spmatops PRIVATE zf_log)

add_test(NAME test_spmatops COMMAND test_spmatops)

add_executable(bench benchmark_bc.cpp
        ../src/common.cpp
        ../src/graphs.cpp
        ../src/matds.cpp
        ../src/matio.cpp
        ../src/spmatops.cpp
        ../src/ecc.cpp)

if(OpenMP_CXX_FOUND)
    target_link_libraries(bench PRIVATE OpenMP::OpenMP_CXX)
endif()

target_link_libraries(bench PRIVATE snap)
target_link_libraries(bench PRIVATE zf_log)
target_link_libraries(bench PRIVATE mmio)
