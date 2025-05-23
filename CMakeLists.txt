cmake_minimum_required(VERSION 3.10)
project(tcp-socket-bluedrop-demo VERSION 1.0 LANGUAGES CXX)

#set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_POSITION_INDEPENDENT_CODE ON)

#
# Set TARGET_PROJECT_NAME for each executable (same as snap "app name" =ExeName)
#
set(TARGET_PROJECT_NAME tcp-socket-bluedrop-demo)

#
# Option to Build the Snap
#
option (BUILD_SNAP "Build Snap Package" OFF)

MESSAGE( STATUS "Selected BuildKit: ${BUILD_KIT}")
MESSAGE( STATUS "######################################")
MESSAGE( STATUS "Option: BUILD_SNAP ${BUILD_SNAP}")
MESSAGE( STATUS "######################################")

#
# Compiler setting
#
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")


# Future Data Layer integration
# find_package(ctrlx-automation-sdk REQUIRED COMPONENTS datalayer-client-cpp)

# No external networking libs: use POSIX sockets

IF( ${BUILD_KIT} MATCHES "aarch64")
    SET( PLATFORM_NAME "ubuntu22-gcc-aarch64")
    SET( SNAP_ARCH "arm64")
    set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
    set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)
ELSE()
    SET( PLATFORM_NAME "ubuntu22-gcc-x64")
    SET( SNAP_ARCH "amd64")
    set(CMAKE_CXX_COMPILER g++)
ENDIF()

include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(tcp-socket-bluedrop-demo src/main.cpp)

target_link_libraries(tcp-socket-bluedrop-demo
        PRIVATE
        pthread
)

if (BUILD_SNAP)
    #
    # Set destination paths for installation
    #
    install( TARGETS ${TARGET_PROJECT_NAME}
            RUNTIME DESTINATION ${CMAKE_CURRENT_LIST_DIR}/generated/build
            LIBRARY DESTINATION ${CMAKE_CURRENT_LIST_DIR}/generated/build
            ARCHIVE DESTINATION ${CMAKE_CURRENT_LIST_DIR}/generated/build
            COMPONENT ${BUNDLE_PROJECT_NAME}
    )

endif(BUILD_SNAP)

add_custom_command(TARGET ${TARGET_PROJECT_NAME}
        POST_BUILD
        COMMAND make install
)

if (BUILD_SNAP)
    add_custom_command( TARGET ${TARGET_PROJECT_NAME}
            POST_BUILD
            COMMAND sudo snapcraft clean
            COMMAND sudo snapcraft --build-for=${SNAP_ARCH} --verbosity=verbose
            WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
    )
endif(BUILD_SNAP)
