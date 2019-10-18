if(NOT CMAKE_CROSSCOMPILING)
  find_package(PkgConfig QUIET)
  pkg_check_modules(PC_OPUS opus)
endif()

set_extra_dirs_lib(OPUS opus)
find_library(OPUS_LIBRARY
  NAMES opus
  HINTS ${HINTS_OPUS_LIBDIR} ${PC_OPUS_LIBDIR} ${PC_OPUS_LIBRARY_DIRS}
  PATHS ${PATHS_OPUS_LIBDIR}
  ${CROSSCOMPILING_NO_CMAKE_SYSTEM_PATH}
)
set_extra_dirs_include(OPUS opus "${OPUS_LIBRARY}")
find_path(OPUS_INCLUDEDIR opus.h
  PATH_SUFFIXES opus
  HINTS ${HINTS_OPUS_INCLUDEDIR} ${PC_OPUS_INCLUDEDIR} ${PC_OPUS_INCLUDE_DIRS}
  PATHS ${PATHS_OPUS_INCLUDEDIR}
  ${CROSSCOMPILING_NO_CMAKE_SYSTEM_PATH}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Opus DEFAULT_MSG OPUS_INCLUDEDIR)

mark_as_advanced(OPUS_INCLUDEDIR OPUS_LIBRARY)

if(OPUS_FOUND)
  set(OPUS_INCLUDE_DIRS ${OPUS_INCLUDEDIR})
  if(OPUS_LIBRARY)
    set(OPUS_LIBRARIES ${OPUS_LIBRARY})
  else()
    set(OPUS_LIBRARIES)
  endif()
endif()
