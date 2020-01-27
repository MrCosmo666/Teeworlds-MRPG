if(NOT CMAKE_CROSSCOMPILING)
  find_package(PkgConfig QUIET)
  pkg_check_modules(PC_OGG ogg)
endif()

set_extra_dirs_lib(OGG opus)
find_library(OGG_LIBRARY
  NAMES ogg
  HINTS ${HINTS_OGG_LIBDIR} ${PC_OGG_LIBDIR} ${PC_OGG_LIBRARY_DIRS}
  PATHS ${PATHS_OGG_LIBDIR}
  ${CROSSCOMPILING_NO_CMAKE_SYSTEM_PATH}
)
set_extra_dirs_include(OGG opus "${OGG_LIBRARY}")
find_path(OGG_INCLUDEDIR ogg.h
  PATH_SUFFIXES ogg
  HINTS ${HINTS_OGG_INCLUDEDIR} ${PC_OGG_INCLUDEDIR} ${PC_OGG_INCLUDE_DIRS}
  PATHS ${PATHS_OGG_INCLUDEDIR}
  ${CROSSCOMPILING_NO_CMAKE_SYSTEM_PATH}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Ogg DEFAULT_MSG OGG_INCLUDEDIR)

mark_as_advanced(OGG_INCLUDEDIR OGG_LIBRARY)

if(OGG_FOUND)
  set(OGG_INCLUDE_DIRS ${OGG_INCLUDEDIR})
  if(OGG_LIBRARY)
    set(OGG_LIBRARIES ${OGG_LIBRARY})
  else()
    set(OGG_LIBRARIES)
  endif()
endif()
