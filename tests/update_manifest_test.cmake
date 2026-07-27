# SPDX-License-Identifier: GPL-3.0-or-later

if(NOT DEFINED DAYMARK_SOURCE_DIR
    OR NOT DEFINED DAYMARK_BINARY_DIR
    OR NOT DEFINED DAYMARK_VERSION)
    message(FATAL_ERROR "Source, binary, and version inputs are required")
endif()

set(test_directory "${DAYMARK_BINARY_DIR}/update-manifest-test")
file(REMOVE_RECURSE "${test_directory}")
file(MAKE_DIRECTORY "${test_directory}")

set(package_name "Daymark-${DAYMARK_VERSION}-Linux.tar.gz")
set(package_path "${test_directory}/${package_name}")
file(WRITE "${package_path}" "daymark-update-manifest-test-package")

execute_process(
    COMMAND "${CMAKE_COMMAND}"
        "-DRELEASE_TAG=v${DAYMARK_VERSION}"
        "-DPLATFORM_KEY=linux"
        "-DPACKAGE_DIRECTORY=${test_directory}"
        -P "${DAYMARK_SOURCE_DIR}/packaging/create-update-manifest.cmake"
    RESULT_VARIABLE manifest_result
    OUTPUT_VARIABLE manifest_output
    ERROR_VARIABLE manifest_error)
if(NOT manifest_result EQUAL 0)
    message(FATAL_ERROR
        "Manifest generation failed:\n${manifest_output}\n${manifest_error}")
endif()

set(manifest_path "${test_directory}/Daymark-update-linux.json")
file(READ "${manifest_path}" manifest)
string(JSON schema_version GET "${manifest}" schemaVersion)
string(JSON version GET "${manifest}" version)
string(JSON platform GET "${manifest}" platform)
string(JSON asset_name GET "${manifest}" assetName)
string(JSON download_url GET "${manifest}" downloadUrl)
string(JSON package_size GET "${manifest}" size)
string(JSON package_sha256 GET "${manifest}" sha256)
string(JSON release_notes GET "${manifest}" notes)

file(SIZE "${package_path}" expected_size)
file(SHA256 "${package_path}" expected_sha256)
set(expected_url
    "https://github.com/eyial667/daymark/releases/download/v${DAYMARK_VERSION}/${package_name}")

if(NOT schema_version EQUAL 1
    OR NOT version STREQUAL DAYMARK_VERSION
    OR NOT platform STREQUAL "linux"
    OR NOT asset_name STREQUAL package_name
    OR NOT download_url STREQUAL expected_url
    OR NOT package_size EQUAL expected_size
    OR NOT package_sha256 STREQUAL expected_sha256
    OR release_notes STREQUAL "")
    message(FATAL_ERROR "Generated update manifest does not match the package")
endif()
