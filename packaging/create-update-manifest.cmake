# SPDX-License-Identifier: GPL-3.0-or-later

if(NOT DEFINED RELEASE_TAG)
    message(FATAL_ERROR "RELEASE_TAG is required")
endif()
if(NOT DEFINED PLATFORM_KEY)
    message(FATAL_ERROR "PLATFORM_KEY is required")
endif()
if(NOT DEFINED PACKAGE_DIRECTORY)
    message(FATAL_ERROR "PACKAGE_DIRECTORY is required")
endif()

include("${CMAKE_CURRENT_LIST_DIR}/verify-release-version.cmake")

if(PLATFORM_KEY STREQUAL "windows")
    set(asset_name "Daymark-${tag_version}-win64-setup.exe")
elseif(PLATFORM_KEY STREQUAL "macos")
    set(asset_name "Daymark-${tag_version}-Darwin.dmg")
elseif(PLATFORM_KEY STREQUAL "linux")
    set(asset_name "Daymark-${tag_version}-Linux.tar.gz")
else()
    message(FATAL_ERROR "Unsupported updater platform '${PLATFORM_KEY}'")
endif()

get_filename_component(
    package_directory
    "${PACKAGE_DIRECTORY}"
    ABSOLUTE
    BASE_DIR "${CMAKE_CURRENT_LIST_DIR}/..")
set(package_path "${package_directory}/${asset_name}")
if(NOT EXISTS "${package_path}")
    message(FATAL_ERROR "Expected release package '${package_path}' does not exist")
endif()

file(SIZE "${package_path}" package_size)
file(SHA256 "${package_path}" package_sha256)

file(READ "${CMAKE_CURRENT_LIST_DIR}/../CHANGELOG.md" changelog)
set(release_header "## [${tag_version}]")
string(FIND "${changelog}" "${release_header}" release_start)
if(release_start EQUAL -1)
    message(FATAL_ERROR "CHANGELOG.md has no ${tag_version} release section")
endif()
string(SUBSTRING "${changelog}" ${release_start} -1 release_tail)
string(FIND "${release_tail}" "\n" header_end)
if(header_end EQUAL -1)
    message(FATAL_ERROR "The ${tag_version} changelog section has no content")
endif()
math(EXPR notes_start "${header_end} + 1")
string(SUBSTRING "${release_tail}" ${notes_start} -1 release_notes)
string(FIND "${release_notes}" "\n## [" next_release)
if(NOT next_release EQUAL -1)
    string(SUBSTRING "${release_notes}" 0 ${next_release} release_notes)
endif()
string(STRIP "${release_notes}" release_notes)

string(REPLACE "\\" "\\\\" release_notes_json "${release_notes}")
string(REPLACE "\"" "\\\"" release_notes_json "${release_notes_json}")
string(REPLACE "\r" "\\r" release_notes_json "${release_notes_json}")
string(REPLACE "\n" "\\n" release_notes_json "${release_notes_json}")
string(REPLACE "\t" "\\t" release_notes_json "${release_notes_json}")

set(manifest_path "${package_directory}/Daymark-update-${PLATFORM_KEY}.json")
file(WRITE "${manifest_path}"
    "{\n"
    "  \"schemaVersion\": 1,\n"
    "  \"version\": \"${tag_version}\",\n"
    "  \"platform\": \"${PLATFORM_KEY}\",\n"
    "  \"notes\": \"${release_notes_json}\",\n"
    "  \"assetName\": \"${asset_name}\",\n"
    "  \"downloadUrl\": \"https://github.com/eyial667/daymark/releases/download/${RELEASE_TAG}/${asset_name}\",\n"
    "  \"size\": ${package_size},\n"
    "  \"sha256\": \"${package_sha256}\"\n"
    "}\n")

message(STATUS "Created ${manifest_path}")
