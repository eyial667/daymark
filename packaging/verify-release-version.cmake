# SPDX-License-Identifier: GPL-3.0-or-later

if(NOT DEFINED RELEASE_TAG)
    message(FATAL_ERROR "RELEASE_TAG is required")
endif()

if(NOT RELEASE_TAG MATCHES "^v([0-9]+\\.[0-9]+\\.[0-9]+)$")
    message(FATAL_ERROR "Release tag '${RELEASE_TAG}' must look like v1.2.3")
endif()
set(tag_version "${CMAKE_MATCH_1}")

file(READ "${CMAKE_CURRENT_LIST_DIR}/../CMakeLists.txt" daymark_cmake)
if(NOT daymark_cmake MATCHES "VERSION[ \t\r\n]+([0-9]+\\.[0-9]+\\.[0-9]+)")
    message(FATAL_ERROR "Could not read Daymark's project version")
endif()
set(project_version "${CMAKE_MATCH_1}")

if(NOT tag_version STREQUAL project_version)
    message(FATAL_ERROR
        "Release tag ${RELEASE_TAG} does not match project version ${project_version}")
endif()

message(STATUS "Release tag and project version both identify Daymark ${project_version}")
