cmake_minimum_required(VERSION 3.10)
project(docgen VERSION 1.0.0 LANGUAGES C CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

# Coverage build type
set(CMAKE_CXX_FLAGS_COVERAGE "-O0 -g --coverage -fprofile-arcs -ftest-coverage")
set(CMAKE_C_FLAGS_COVERAGE "-O0 -g --coverage -fprofile-arcs -ftest-coverage")
set(CMAKE_EXE_LINKER_FLAGS_COVERAGE "--coverage")

# Default to Release if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Add optimization for Release
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/app/include)

# Find GTK3 and WebKit2GTK
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
pkg_check_modules(WEBKIT2 REQUIRED webkit2gtk-4.1)

# Generate GResource
find_program(GLIB_COMPILE_RESOURCES NAMES glib-compile-resources REQUIRED)
set(GRESOURCE_XML ${PROJECT_SOURCE_DIR}/resources/docgen.gresource.xml)
set(GRESOURCE_C ${CMAKE_CURRENT_BINARY_DIR}/resources.c)

add_custom_command(
    OUTPUT ${GRESOURCE_C}
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/resources
    COMMAND ${GLIB_COMPILE_RESOURCES}
    ARGS
        --target=${GRESOURCE_C}
        --generate-source
        ${GRESOURCE_XML}
    VERBATIM
    MAIN_DEPENDENCY ${GRESOURCE_XML}
    DEPENDS ${PROJECT_SOURCE_DIR}/resources/watermark_order.png
)

# Create a library for shared code
add_library(text_viewer_lib
    app/src/text_viewer.cpp
    app/src/text_section.cpp
    app/src/section_manager.cpp
    app/src/main_window.cpp
    ${GRESOURCE_C}
)

# Add executable
add_executable(${PROJECT_NAME} 
    app/src/main.cpp
)

# Include GTK and WebKit2 directories
include_directories(${GTK3_INCLUDE_DIRS} ${WEBKIT2_INCLUDE_DIRS})
link_directories(${GTK3_LIBRARY_DIRS} ${WEBKIT2_LIBRARY_DIRS})
add_definitions(${GTK3_CFLAGS_OTHER} ${WEBKIT2_CFLAGS_OTHER})

# Link the library and GTK/WebKit2 to the executable
target_link_libraries(${PROJECT_NAME} 
    text_viewer_lib
    ${GTK3_LIBRARIES}
    ${WEBKIT2_LIBRARIES}
)

# Link WebKit2 to the library
target_link_libraries(text_viewer_lib
    ${GTK3_LIBRARIES}
    ${WEBKIT2_LIBRARIES}
)

# Installation rules
install(TARGETS ${PROJECT_NAME} DESTINATION bin)

# Testing
enable_testing()

# Fetch Google Test
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Add test executable
add_executable(tests
    tests/test_main.cpp
)

# Link test executable against gtest, text_viewer library, and GTK3
target_link_libraries(tests
    gtest_main
    text_viewer_lib
    ${GTK3_LIBRARIES}
)

# Register tests
include(GoogleTest)
gtest_discover_tests(tests)

# Coverage target
if(CMAKE_BUILD_TYPE STREQUAL "Coverage")
    find_program(LCOV lcov)
    find_program(GENHTML genhtml)
    
    if(LCOV AND GENHTML)
        add_custom_target(coverage
            COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/coverage
            
            # Reset coverage data
            COMMAND ${LCOV} --directory . --zerocounters
            
            # Run tests
            COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
            
            # Capture coverage data with recommended flags
            COMMAND ${LCOV} --directory . --capture --output-file coverage.info --rc geninfo_unexecuted_blocks=0 --ignore-errors mismatch
            
            # Remove system headers and test files from coverage
            COMMAND ${LCOV} --remove coverage.info '/usr/*' '*/tests/*' '*/_deps/*' --output-file coverage.info --rc geninfo_unexecuted_blocks=0
            
            # Generate HTML report
            COMMAND ${GENHTML} coverage.info --output-directory ${CMAKE_BINARY_DIR}/coverage --ignore-errors unmapped
            
            COMMAND ${CMAKE_COMMAND} -E echo "Coverage report generated in ${CMAKE_BINARY_DIR}/coverage/index.html"
            
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            DEPENDS tests
            COMMENT "Generating code coverage report..."
        )
    else()
        message(WARNING "lcov or genhtml not found. Coverage target will not be available.")
    endif()
endif()
