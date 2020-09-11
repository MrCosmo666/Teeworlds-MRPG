find_package(OpenSSL)
if(OPENSSL_FOUND)
  set(CRYPTO_FOUND ON)
  set(CRYPTO_BUNDLED OFF)
  set(CRYPTO_LIBRARY ${OPENSSL_CRYPTO_LIBRARY})
  set(CRYPTO_INCLUDEDIR ${OPENSSL_INCLUDE_DIR})
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Crypto DEFAULT_MSG CRYPTO_LIBRARY CRYPTO_INCLUDEDIR)

mark_as_advanced(CRYPTO_LIBRARY CRYPTO_INCLUDEDIR)

set_extra_dirs_lib(OPENSSL openssl)
if(CRYPTO_FOUND)
  set(CRYPTO_LIBRARIES ${CRYPTO_LIBRARY})
  set(CRYPTO_INCLUDE_DIRS ${CRYPTO_INCLUDEDIR})
  if(TARGET_OS STREQUAL "windows")
    set(OPENSSL_COPY_FILES 
    	"${EXTRA_OPENSSL_LIBDIR}/libeay32.dll" 
    	"${EXTRA_OPENSSL_LIBDIR}/ssleay32.dll"
    )
  else()
    set(OPENSSL_COPY_FILES)
  endif()
endif()