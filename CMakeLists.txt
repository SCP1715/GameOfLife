cmake_minimum_required(VERSION 3.22)

project(GameOfLife LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 5.15 REQUIRED COMPONENTS Widgets)

add_executable(GameOfLife WIN32
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    ThreadController.h
    Worker.h
)

target_link_libraries(GameOfLife PRIVATE Qt5::Widgets)

if(MSVC)
    target_compile_options(GameOfLife PRIVATE /EHsc /D_SILENCE_AMP_DEPRECATION_WARNINGS)
else()
    message(FATAL_ERROR "C++ AMP requires the Microsoft Visual C++ compiler.")
endif()

option(GAMEOFLIFE_BUILD_GPU_TESTS "Build tests that require a supported GPU" OFF)
if(GAMEOFLIFE_BUILD_GPU_TESTS)
    enable_testing()
    add_executable(GameOfLifeGpuTest tests/gpu_worker_test.cpp Worker.h)
    target_link_libraries(GameOfLifeGpuTest PRIVATE Qt5::Core)
    target_compile_options(GameOfLifeGpuTest PRIVATE /EHsc /D_SILENCE_AMP_DEPRECATION_WARNINGS)
    add_test(NAME GameOfLifeGpuTest COMMAND GameOfLifeGpuTest)
    get_target_property(_qt_core_runtime Qt5::Core IMPORTED_LOCATION_DEBUG)
    get_filename_component(_qt_runtime_dir "${_qt_core_runtime}" DIRECTORY)
    set_tests_properties(GameOfLifeGpuTest PROPERTIES
        ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:${_qt_runtime_dir}"
    )
endif()
