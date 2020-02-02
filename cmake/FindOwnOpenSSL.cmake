set_extra_dirs_lib(OPENSSL openssl)
#######################################  MSVC LIB
if(MSVC)
  if(CMAKE_BUILD_TYPE STREQUAL Release)
  	find_library(OWN_OPENSSL_LIBRARY 
	    NAMES "libcrypto${TARGET_BITS}MD"
	    HINTS ${HINTS_OPENSSL_LIBDIR}/VC ${PC_OPENSSL_LIBDIR} ${PC_OPENSSL_LIBRARY_DIRS}
	    ${CROSSCOMPILING_NO_CMAKE_SYSTEM_PATH})	  
  else()
    find_library(OWN_OPENSSL_LIBRARY 
	  NAMES "libcrypto${TARGET_BITS}MDd"
	  HINTS ${HINTS_OPENSSL_LIBDIR}/VC ${PC_OPENSSL_LIBDIR} ${PC_OPENSSL_LIBRARY_DIRS}
	  ${CROSSCOMPILING_NO_CMAKE_SYSTEM_PATH})
  endif()
####################################### MINGW LIB
elseif (MINGW)
  find_library(OWN_OPENSSL_LIBRARY 
    NAMES "libcrypto" "libssl"
	HINTS ${HINTS_OPENSSL_LIBDIR}/MinGW ${PC_OPENSSL_LIBDIR} ${PC_OPENSSL_LIBRARY_DIRS}
	${CROSSCOMPILING_NO_CMAKE_SYSTEM_PATH})
endif()

#######################################  INCLUDES
set_extra_dirs_include(OPENSSL openssl "${OWN_OPENSSL_LIBRARY}")
find_path(OWN_OPENSSL_INCLUDEDIR 
  NAMES "openssl/opensslconf.h"
  HINTS ${HINTS_OPENSSL_INCLUDEDIR} ${PC_OPENSSL_INCLUDEDIR} ${PC_OPENSSL_INCLUDE_DIR}
  ${CROSSCOMPILING_NO_CMAKE_SYSTEM_PATH}
)

#######################################
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OPENSSL DEFAULT_MSG OWN_OPENSSL_LIBRARY OWN_OPENSSL_INCLUDEDIR)

mark_as_advanced(OWN_OPENSSL_LIBRARY OWN_OPENSSL_INCLUDEDIR)

#######################################
if(NOT(OPENSSL_FOUND))
  # [some data may not match dependencies / TODO: later fix it]
  set(CMAKE_MODULE_PATH ${ORIGINAL_CMAKE_MODULE_PATH})
  find_package(OpenSSL)
  set(CMAKE_MODULE_PATH ${OWN_CMAKE_MODULE_PATH})
else()
  set(OPENSSL_LIBRARIES ${OWN_OPENSSL_LIBRARY})
  set(OPENSSL_INCLUDE_DIR ${OWN_OPENSSL_INCLUDEDIR})
  set(OPENSSL_COPY_FILES "${EXTRA_OPENSSL_LIBDIR}/ssleay32.dll" "${EXTRA_OPENSSL_LIBDIR}/libeay32.dll")
endif()
