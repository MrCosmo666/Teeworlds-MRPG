set_extra_dirs_lib(ICU icu)
#######################################  MSVC LIB
find_library(OWN_ICU_LIBRARYDT
  NAMES "icudt" "icudata"
  HINTS ${HINTS_ICU_LIBDIR} ${PC_ICU_LIBDIR} ${PC_ICU_LIBRARY_DIRS}
  PATHS ${PATHS_ICU_LIBDIR}
  ${CROSSCOMPILING_NO_CMAKE_SYSTEM_PATH})
  
find_library(OWN_ICU_LIBRARYIN
  NAMES "icuin" "icui18n"
  HINTS ${HINTS_ICU_LIBDIR} ${PC_ICU_LIBDIR} ${PC_ICU_LIBRARY_DIRS}
  PATHS ${PATHS_ICU_LIBDIR}
  ${CROSSCOMPILING_NO_CMAKE_SYSTEM_PATH})
  
find_library(OWN_ICU_LIBRARYUC
  NAMES "icuuc"
  HINTS ${HINTS_ICU_LIBDIR} ${PC_ICU_LIBDIR} ${PC_ICU_LIBRARY_DIRS}
  PATHS ${PATHS_ICU_LIBDIR}
  ${CROSSCOMPILING_NO_CMAKE_SYSTEM_PATH})

#######################################  INCLUDES
set_extra_dirs_include(ICU icu "${OWN_ICU_LIBRARYDT}")
find_path(OWN_ICU_INCLUDEDIR 
  NAMES "unicode/udata.h"
  HINTS ${HINTS_ICU_INCLUDEDIR} ${PC_ICU_INCLUDEDIR} ${PC_ICU_INCLUDE_DIR}
  PATHS ${PATHS_ICU_INCLUDEDIR}
  ${CROSSCOMPILING_NO_CMAKE_SYSTEM_PATH})

#######################################
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ICU DEFAULT_MSG OWN_ICU_LIBRARYDT OWN_ICU_INCLUDEDIR)
mark_as_advanced(OWN_ICU_LIBRARYDT OWN_ICU_INCLUDEDIR)

#######################################
if(NOT(ICU_FOUND))
  # [some data may not match dependencies / TODO: later fix it]
  set(CMAKE_MODULE_PATH ${ORIGINAL_CMAKE_MODULE_PATH})
  find_package(ICU REQUIRED COMPONENTS uc in)
  set(CMAKE_MODULE_PATH ${OWN_CMAKE_MODULE_PATH})
else()
  set(ICU_LIBRARIES ${OWN_ICU_LIBRARYDT} ${OWN_ICU_LIBRARYIN} ${OWN_ICU_LIBRARYUC})
  set(ICU_INCLUDE_DIRS ${OWN_ICU_INCLUDEDIR})
  
  if(TARGET_OS AND TARGET_OS STREQUAL "windows")
   set(ICU_COPY_FILES "${EXTRA_ICU_LIBDIR}/icudt65.dll" "${EXTRA_ICU_LIBDIR}/icuin65.dll" "${EXTRA_ICU_LIBDIR}/icuuc65.dll")
  else()
   set(ICU_COPY_FILES)
  endif()
endif()
