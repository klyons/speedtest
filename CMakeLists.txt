cmake_minimum_required(VERSION 3.15)
project(speedtests)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS NO)

# Set build type to Debug
set(CMAKE_BUILD_TYPE Debug)

get_filename_component(ROOT_DIR "${CMAKE_SOURCE_DIR}/../vcpkg" ABSOLUTE)

# Include vcpkg toolchain using a relative path
set(CMAKE_TOOLCHAIN_FILE "${ROOT_DIR}/scripts/buildsystems/vcpkg.cmake")

# Get Halide tools and include directories
message(STATUS "Here is your ROOT_DIR= ${ROOT_DIR}")

set(HALIDE_TOOLS_DIR "${ROOT_DIR}/packages/halide_x64-windows/share/halide/tools")
set(HALIDE_INCLUDE_DIR "${ROOT_DIR}/packages/halide_x64-windows/include")
set(HALIDE_LIBARY "${ROOT_DIR}/packages/halide_x64-windows/lib/Halide.lib")

# Get ZLIB, JPEG, and TIFF directories
set(ZLIB_INCLUDE_DIR "${ROOT_DIR}/packages/zlib_x64-windows/include")
set(ZLIB_LIBRARY "${ROOT_DIR}/packages/zlib_x64-windows/lib/zlib.lib")
set(TIFF_INCLUDE_DIR "${ROOT_DIR}/packages/tiff_x64-windows/include")
set(TIFF_LIBRARIES "${ROOT_DIR}/packages/tiff_x64-windows/lib/tiff.lib")
set(TIFF_LIBRARY "${ROOT_DIR}/packages/tiff_x64-windows/lib/tiff.lib")

# Use plurals in the name
set(JPEG_LIBRARIES "${ROOT_DIR}/packages/libjpeg-turbo_x64-windows/lib/turbojpeg.lib")
set(JPEG_INCLUDE_DIRS "${ROOT_DIR}/packages/libjpeg-turbo_x64-windows/include")

set(PNG_LIBRARIES "${ROOT_DIR}/packages/libpng_x64-windows/lib/libpng16.lib")
set(PNG_PNG_INCLUDE_DIRS "${ROOT_DIR}/packages/libpng_x64-windows/include/libpng16")

# Print the HALIDE_TOOLS_DIR path
message(STATUS "HALIDE_TOOLS_DIR : ${HALIDE_TOOLS_DIR}")
message(STATUS "ROOT_DIR : ${ROOT_DIR}")

# Manually set Halide directory
set(CMAKE_PREFIX_PATH "${ROOT_DIR}/installed/x64-windows/share/halide;${ROOT_DIR}/packages/libjpeg-turbo_x64-windows/share/libjpeg-turbo;
	${ROOT_DIR}/packages/tiff_x64-windows/share/tiff;${ROOT_DIR}/packages/zlib_x64-windows/share/zlib;
	${ROOT_DIR}/installed/x64-windows/halide/share/HalideHelpers;${ROOT_DIR}/packages/libpng_x64-windows/share/libpng;")

# Use link_directories to specify the path to the DLL
link_directories("${ROOT_DIR}/installed/x64-windows/bin")

set(PNG_LIBRARY "${ROOT_DIR}/packages/libpng_x64-windows/lib/libpng16.lib")
set(PNG_PNG_INCLUDE_DIR "${ROOT_DIR}/packages/libpng_x64-windows/include/libpng16")
set(JPEG_LIBRARY "${ROOT_DIR}/packages/libjpeg-turbo_x64-windows/lib/jpeg.lib")
set(JPEG_INCLUDE_DIR "${ROOT_DIR}/packages/libjpeg-turbo_x64-windows/include")


# Find packages
find_package(Halide REQUIRED)
find_package(Halide CONFIG REQUIRED)
find_package(HalideHelpers REQUIRED)
find_package(JPEG REQUIRED)
#find_package(PNG REQUIRED)
find_package(ZLIB REQUIRED)
find_package(TIFF REQUIRED)

# Print found Halide targets
get_target_property(Halide_TARGETS Halide::Halide INTERFACE_LINK_LIBRARIES)
message(STATUS "Halide targets: ${Halide_TARGETS}")

# Add Halide bin dir to PATH
set(ENV{PATH} "$ENV{PATH};C:/ws/vcpkg/installed/x64-windows/bin" )


# Add executable
add_executable(speedtests "speedtests.cpp" "PGMImage.cpp" "PGMImage.h" "TiffSrcFile.cpp" "TiffSrcFile.h")

#add custom command to point to the Halide dll
# Add custom command to copy all DLLs from the bin directory
# add_custom_command(TARGET speedtests POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy_directory
#    "${ROOT_DIR}/installed/x64-windows/bin"
#    $<TARGET_FILE_DIR:speedtests>)
#

# Include directories
target_include_directories(speedtests PRIVATE ${HALIDE_TOOLS_DIR} ${HALIDE_INCLUDE_DIR} ${TIFF_INCLUDE_DIR} ${JPEG_INCLUDE_DIR} ${PNG_PNG_INCLUDE_DIRS} ${PNG_PNG_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR})

# Link libraries
target_link_libraries(speedtests PRIVATE Halide::Halide Halide::Tools ${TIFF_LIBRARIES} ${ZLIB_LIBRARY} ${JPEG_LIBRARY} ${PNG_LIBRARIES} ${PNG_LIBRARY} ${HALIDE_LIBARY})
