#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "snmalloc::snmallocshim-static" for configuration "Release"
set_property(TARGET snmalloc::snmallocshim-static APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(snmalloc::snmallocshim-static PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libsnmallocshim-static.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS snmalloc::snmallocshim-static )
list(APPEND _IMPORT_CHECK_FILES_FOR_snmalloc::snmallocshim-static "${_IMPORT_PREFIX}/lib/libsnmallocshim-static.a" )

# Import target "snmalloc::snmallocshim" for configuration "Release"
set_property(TARGET snmalloc::snmallocshim APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(snmalloc::snmallocshim PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libsnmallocshim.so"
  IMPORTED_SONAME_RELEASE "libsnmallocshim.so"
  )

list(APPEND _IMPORT_CHECK_TARGETS snmalloc::snmallocshim )
list(APPEND _IMPORT_CHECK_FILES_FOR_snmalloc::snmallocshim "${_IMPORT_PREFIX}/lib/libsnmallocshim.so" )

# Import target "snmalloc::snmallocshim-checks-memcpy-only" for configuration "Release"
set_property(TARGET snmalloc::snmallocshim-checks-memcpy-only APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(snmalloc::snmallocshim-checks-memcpy-only PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libsnmallocshim-checks-memcpy-only.so"
  IMPORTED_SONAME_RELEASE "libsnmallocshim-checks-memcpy-only.so"
  )

list(APPEND _IMPORT_CHECK_TARGETS snmalloc::snmallocshim-checks-memcpy-only )
list(APPEND _IMPORT_CHECK_FILES_FOR_snmalloc::snmallocshim-checks-memcpy-only "${_IMPORT_PREFIX}/lib/libsnmallocshim-checks-memcpy-only.so" )

# Import target "snmalloc::snmallocshim-checks" for configuration "Release"
set_property(TARGET snmalloc::snmallocshim-checks APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(snmalloc::snmallocshim-checks PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libsnmallocshim-checks.so"
  IMPORTED_SONAME_RELEASE "libsnmallocshim-checks.so"
  )

list(APPEND _IMPORT_CHECK_TARGETS snmalloc::snmallocshim-checks )
list(APPEND _IMPORT_CHECK_FILES_FOR_snmalloc::snmallocshim-checks "${_IMPORT_PREFIX}/lib/libsnmallocshim-checks.so" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
