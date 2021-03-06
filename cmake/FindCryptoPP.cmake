# Taken from https://github.com/mateuszzz88/FindCryptoPP.cmake

find_path(CryptoPP_INCLUDE_DIR NAMES cryptopp/config.h DOC "CryptoPP include directory")
find_library(CryptoPP_LIBRARY NAMES cryptopp DOC "CryptoPP library")

if(CryptoPP_INCLUDE_DIR)
    file(STRINGS ${CryptoPP_INCLUDE_DIR}/cryptopp/config.h _config_version REGEX "CRYPTOPP_VERSION")
    string(REGEX MATCH "([0-9])([0-9])([0-9])" _match_version ${_config_version})
    set(CryptoPP_VERSION_STRING "${CMAKE_MATCH_1}.${CMAKE_MATCH_2}.${CMAKE_MATCH_3}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(CryptoPP
    REQUIRED_VARS CryptoPP_INCLUDE_DIR CryptoPP_LIBRARY
    FOUND_VAR CryptoPP_FOUND
    VERSION_VAR CryptoPP_VERSION_STRING)

if(CryptoPP_FOUND AND NOT TARGET CryptoPP::CryptoPP)
    add_library(CryptoPP::CryptoPP UNKNOWN IMPORTED)
    set_target_properties(CryptoPP::CryptoPP PROPERTIES
        IMPORTED_LOCATION "${CryptoPP_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${CryptoPP_INCLUDE_DIR}")
endif()

mark_as_advanced(CryptoPP_INCLUDE_DIR CryptoPP_LIBRARY)
set(CryptoPP_INCLUDE_DIRS ${CryptoPP_INCLUDE_DIR})
set(CryptoPP_LIBRARIES ${CryptoPP_LIBRARY})