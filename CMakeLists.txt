cmake_minimum_required(VERSION 2.8)

file(STRINGS src/game/version.h VERSION_LINE
  LIMIT_COUNT 1
  REGEX GAME_RELEASE_VERSION
)

if(VERSION_LINE MATCHES "\"([0-9]+)\\.([0-9]+)\\.([0-9]+|[0-9]+\\.[0-9]+)\"")
  set(VERSION_MAJOR ${CMAKE_MATCH_1})
  set(VERSION_MINOR ${CMAKE_MATCH_2})
  set(VERSION_PATCH ${CMAKE_MATCH_3})
elseif(VERSION_LINE MATCHES "\"([0-9]+)\\.([0-9]+)\"")
  set(VERSION_MAJOR ${CMAKE_MATCH_1})
  set(VERSION_MINOR ${CMAKE_MATCH_2})
  set(VERSION_PATCH "0")
else()
  message(FATAL_ERROR "Couldn't parse version from src/game/version.h")
endif()

if(POLICY CMP0048)
  cmake_policy(SET CMP0048 NEW)
  project(mmoteeworlds VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})
else()
  project(mmoteeworlds)
  set(PROJECT_VERSION_MAJOR ${VERSION_MAJOR})
  set(PROJECT_VERSION_MINOR ${VERSION_MINOR})
  set(PROJECT_VERSION_PATCH ${VERSION_PATCH})
  set(PROJECT_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})
endif()

if(POLICY CMP0072)
  cmake_policy(SET CMP0072 OLD)
endif()

if(POLICY CMP0091)
  cmake_policy(SET CMP0091 NEW)
endif()

if(POLICY CMP0092)
  cmake_policy(SET CMP0092 NEW)
endif()

set(ORIGINAL_CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH})
set(ORIGINAL_CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES})
set(ORIGINAL_CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES})
set(OWN_CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake)
set(CMAKE_MODULE_PATH ${OWN_CMAKE_MODULE_PATH})

if(CMAKE_SIZEOF_VOID_P EQUAL 4)
  set(TARGET_BITS "32")
else()
  set(TARGET_BITS "64")
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  set(TARGET_OS "windows")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  set(TARGET_OS "linux")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  set(TARGET_OS "mac")
endif()

include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag)
include(CheckSymbolExists)

check_symbol_exists(__i386 "" TARGET_ARCH_X86_i386)
if(TARGET_ARCH_X86_i386)
  set(TARGET_ARCH x86)
else()
  set(TARGET_ARCH)
endif()

set(AUTO_DEPENDENCIES_DEFAULT OFF)
if(TARGET_OS STREQUAL "windows")
  set(AUTO_DEPENDENCIES_DEFAULT ON)
endif()

option(CLIENT "Compile client" ON)
option(DOWNLOAD_DEPENDENCIES "Download dependencies (only available on Windows)" ${AUTO_DEPENDENCIES_DEFAULT})
option(DOWNLOAD_GTEST "Download and compile GTest if not found" ${AUTO_DEPENDENCIES_DEFAULT})
option(PREFER_BUNDLED_LIBS "Prefer bundled libraries over system libraries" ${AUTO_DEPENDENCIES_DEFAULT})
option(DEV "Don't generate stuff necessary for packaging" OFF)

set(OpenGL_GL_PREFERENCE LEGACY)

# Set the default build type to Release
if(NOT(CMAKE_BUILD_TYPE))
  if(NOT(DEV))
    set(CMAKE_BUILD_TYPE Release)
  else()
    set(CMAKE_BUILD_TYPE Debug)
  endif()
endif()

set(DBG $<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>)

set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
  src/game/version.h
)

set(CLIENT_EXECUTABLE mmoteeworlds CACHE STRING "Name of the build client executable")

########################################################################
# Download dependencies
########################################################################

find_package(PythonInterp)
if(DOWNLOAD_DEPENDENCIES)
  if(PYTHON_EXECUTABLE AND TARGET_OS STREQUAL "windows" AND TARGET_BITS)
    set(DOWNLOADS)
    foreach(d freetype sdl)
      if(NOT EXISTS "${PROJECT_SOURCE_DIR}/other/${d}/${TARGET_OS}/lib${TARGET_BITS}")
        list(APPEND DOWNLOADS ${d})
      endif()
    endforeach()
    if(DOWNLOADS)
      message(STATUS "Downloading Freetype and SDL 2")
      execute_process(COMMAND ${PYTHON_EXECUTABLE} scripts/download.py ${DOWNLOADS}
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
      )
    endif()
  endif()
endif()

########################################################################
# Compiler flags
########################################################################

function(add_c_compiler_flag_if_supported VARIABLE FLAG)
  if(ARGC GREATER 2)
    set(CHECKED_FLAG "${ARGV2}")
  else()
    set(CHECKED_FLAG "${FLAG}")
  endif()
  string(REGEX REPLACE "[^A-Za-z0-9]" "_" CONFIG_VARIABLE "FLAG_SUPPORTED${CHECKED_FLAG}")
  check_c_compiler_flag("${CHECKED_FLAG}" ${CONFIG_VARIABLE})
  if(${CONFIG_VARIABLE})
    if(${VARIABLE})
      set("${VARIABLE}" "${${VARIABLE}};${FLAG}" PARENT_SCOPE)
    else()
      set("${VARIABLE}" "${FLAG}" PARENT_SCOPE)
    endif()
  endif()
endfunction()

if(NOT MSVC)
  if(CMAKE_VERSION VERSION_LESS 3.1 OR TARGET_OS STREQUAL "mac")
    check_cxx_compiler_flag(-std=gnu++11 FLAG_SUPPORTED_std_gnu__11)
    if(FLAG_SUPPORTED_std_gnu__11)
      if(CMAKE_CXX_FLAGS)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")
      else()
        set(CMAKE_CXX_FLAGS -std=gnu++11)
      endif()
    endif()
  endif()

  # Protect the stack pointer.
  # -fstack-protector-all doesn't work on MinGW.
  add_c_compiler_flag_if_supported(OUR_FLAGS -fstack-protector-all)

  # Inaccurate floating point numbers cause problems on mingw-w64-gcc when
  # compiling for x86, might cause problems elsewhere. So don't store floats
  # in registers but keep them at higher accuracy.

  if(TARGET_ARCH STREQUAL "x86")
    add_c_compiler_flag_if_supported(OUR_FLAGS -ffloat-store)
  endif()

  if(TARGET_OS STREQUAL "mac")
    add_c_compiler_flag_if_supported(OUR_FLAGS -stdlib=libc++)
    add_c_compiler_flag_if_supported(OUR_FLAGS -mmacosx-version-min=10.7)
  endif()

  add_c_compiler_flag_if_supported(OUR_FLAGS_OWN -Wall)
  if(CMAKE_VERSION VERSION_GREATER 3.3 OR CMAKE_VERSION VERSION_EQUAL 3.3)
    add_c_compiler_flag_if_supported(OUR_FLAGS_OWN
      $<$<COMPILE_LANGUAGE:C>:-Wdeclaration-after-statement>
      -Wdeclaration-after-statement
    )
  endif()
  add_c_compiler_flag_if_supported(OUR_FLAGS_OWN -Wextra)
  add_c_compiler_flag_if_supported(OUR_FLAGS_OWN -Wno-unused-parameter)
  add_c_compiler_flag_if_supported(OUR_FLAGS_OWN -Wno-missing-field-initializers)
  add_c_compiler_flag_if_supported(OUR_FLAGS_OWN -Wformat=2) # Warn about format strings.
  add_c_compiler_flag_if_supported(OUR_FLAGS_DEP -Wno-implicit-function-declaration)
  add_c_compiler_flag_if_supported(OUR_FLAGS_OWN -Wduplicated-cond)
  add_c_compiler_flag_if_supported(OUR_FLAGS_OWN -Wduplicated-branches)
  add_c_compiler_flag_if_supported(OUR_FLAGS_OWN -Wlogical-op)
  add_c_compiler_flag_if_supported(OUR_FLAGS_OWN -Wrestrict)
  # TODO: Enable for C++ code except gtest
  #add_cxx_compiler_flag_if_supported(OUR_FLAGS_OWN "-Wuseless-cast")
endif()

if(NOT MSVC)
  check_c_compiler_flag("-O2;-Wp,-Werror;-D_FORTIFY_SOURCE=2" DEFINE_FORTIFY_SOURCE) # Some distributions define _FORTIFY_SOURCE by themselves.
endif()

########################################################################
# COMMON FUNCTIONS
########################################################################

function(set_glob VAR GLOBBING EXTS DIRECTORY) # ...
  set(GLOBS)
  foreach(ext ${EXTS})
    list(APPEND GLOBS "${DIRECTORY}/*.${ext}")
  endforeach()
  file(${GLOBBING} GLOB_RESULT ${GLOBS})
  list(SORT GLOB_RESULT)
  set(${VAR} ${GLOB_RESULT} PARENT_SCOPE)
endfunction()

function(set_src VAR GLOBBING DIRECTORY) # ...
  set_glob(${VAR} ${GLOBBING} "c;cpp;h" ${DIRECTORY} ${ARGN})
  set(${VAR} ${${VAR}} PARENT_SCOPE)
endfunction()

########################################################################
# INITIALIZE TARGET LISTS
########################################################################

set(TARGETS_OWN)
set(TARGETS_DEP)

set(TARGETS_LINK) # Targets with a linking stage.

########################################################################
# DEPENDENCIES
########################################################################

function(set_extra_dirs_lib VARIABLE NAME)
  set("PATHS_${VARIABLE}_LIBDIR" PARENT_SCOPE)
  set("HINTS_${VARIABLE}_LIBDIR" PARENT_SCOPE)
  if(PREFER_BUNDLED_LIBS)
    set(TYPE HINTS)
  else()
    set(TYPE PATHS)
  endif()
  if(TARGET_BITS AND TARGET_OS)
    set(DIR "other/${NAME}/${TARGET_OS}/lib${TARGET_BITS}")
    set("${TYPE}_${VARIABLE}_LIBDIR" "${DIR}" PARENT_SCOPE)
    set("EXTRA_${VARIABLE}_LIBDIR" "${DIR}" PARENT_SCOPE)
  endif()
endfunction()

function(set_extra_dirs_include VARIABLE NAME LIBRARY)
  set("PATHS_${VARIABLE}_INCLUDEDIR" PARENT_SCOPE)
  set("HINTS_${VARIABLE}_INCLUDEDIR" PARENT_SCOPE)
  is_bundled(IS_BUNDLED "${LIBRARY}")
  if(IS_BUNDLED)
    set("HINTS_${VARIABLE}_INCLUDEDIR" "other/${NAME}/include" "other/${NAME}/include/${TARGET_OS}" PARENT_SCOPE)
  endif()
endfunction()

if(CMAKE_CROSSCOMPILING)
  set(CROSSCOMPILING_NO_CMAKE_SYSTEM_PATH NO_CMAKE_SYSTEM_PATH)
else()
  set(CROSSCOMPILING_NO_CMAKE_SYSTEM_PATH)
endif()

function(is_bundled VARIABLE PATH)
  if(PATH)
    string(FIND "${PATH}" "${PROJECT_SOURCE_DIR}" LOCAL_PATH_POS)
    if(LOCAL_PATH_POS EQUAL 0 AND TARGET_BITS AND TARGET_OS)
      set("${VARIABLE}" ON PARENT_SCOPE)
    else()
      set("${VARIABLE}" OFF PARENT_SCOPE)
    endif()
  else()
    set("${VARIABLE}" OFF PARENT_SCOPE)
  endif()
endfunction()

if(NOT CMAKE_CROSSCOMPILING)
  # Check for PkgConfig once so all the other `find_package` calls can do it
  # quietly.
  find_package(PkgConfig)
endif()

find_package(ZLIB)
find_package(Crypto)
find_package(OwnFreetype)
find_package(Opus)
find_package(Opusfile)
find_package(Git)
find_package(GTest)
find_package(Pnglite)
find_package(SDL2)
find_package(Threads)
find_package(Wavpack)
find_package(Curl)

# includes for auto-download and install packages cmake
#include(ExternalProject)
#include(cmake/DownloadProject.cmake)

# Auto download and install websocketpp
#download_project(
#	PROJ           websocketpp
#	GIT_REPOSITORY https://github.com/zaphoyd/websocketpp.git
#	GIT_TAG        master
#	SOURCE_DIR     ${PROJECT_SOURCE_DIR}/other/websocketpp
#	UPDATE_DISCONNECTED 1
#)
#include_directories("${websocketpp_SOURCE_DIR}")

# Note websocketpp doesn't work with the latest version of asio
# Remember to go back to the master version after they are fixed
#download_project(
#	PROJ           asio
#	GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
#	GIT_TAG        22afb86
#	SOURCE_DIR     ${PROJECT_SOURCE_DIR}/other/asio
#	UPDATE_DISCONNECTED 1
#)
#set(asio_INCLUDE_DIR "${asio_SOURCE_DIR}/asio/include")

# CPR
#download_project(
#	PROJ           cpr
#	GIT_REPOSITORY https://github.com/whoshuu/cpr.git
#	GIT_TAG        master
#	SOURCE_DIR     ${PROJECT_SOURCE_DIR}/other/cpr
#	UPDATE_DISCONNECTED 1
#)
#include_directories("${cpr_SOURCE_DIR}")

if(TARGET_OS AND TARGET_OS STREQUAL "mac")
  find_program(CMAKE_OTOOL otool)
  find_program(DMG dmg)
  find_program(HFSPLUS hfsplus)
  find_program(NEWFS_HFS newfs_hfs)
  if(DMG AND HFSPLUS AND NEWFS_HFS)
    set(DMGTOOLS_FOUND ON)
  else()
    set(DMGTOOLS_FOUND OFF)
  endif()

  find_program(HDIUTIL hdiutil)
  if(HDIUTIL)
    set(HDIUTIL_FOUND ON)
  else()
    set(HDIUTIL_FOUND OFF)
  endif()
endif()

message(STATUS "******** Teeworlds ********")
message(STATUS "Target OS: ${TARGET_OS} ${TARGET_BITS}bit")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

message(STATUS "Dependencies:")
function(show_dependency_status OUTPUT_NAME NAME)
  if(${NAME}_FOUND)
    if(${NAME}_BUNDLED)
      message(STATUS " * ${OUTPUT_NAME} not found (using bundled version)")
    else()
      message(STATUS " * ${OUTPUT_NAME} found")
    endif()
  else()
    message(STATUS " * ${OUTPUT_NAME} not found")
  endif()
endfunction()

show_dependency_status("Curl" CURL)
if(TARGET_OS AND TARGET_OS STREQUAL "mac")
  show_dependency_status("Dmg tools" DMGTOOLS)
endif()
show_dependency_status("Freetype" FREETYPE)
if(TARGET_OS AND TARGET_OS STREQUAL "mac")
  show_dependency_status("Hdiutil" HDIUTIL)
endif()
show_dependency_status("OpenSSL Crypto" CRYPTO)
show_dependency_status("Opus" OPUS)
show_dependency_status("Opusfile" OPUSFILE)
show_dependency_status("Pnglite" PNGLITE)
show_dependency_status("PythonInterp" PYTHONINTERP)
show_dependency_status("SDL2" SDL2)
show_dependency_status("Wavpack" WAVPACK)
show_dependency_status("Zlib" ZLIB)

if(CLIENT AND NOT(CURL_FOUND))
  message(SEND_ERROR "You must install Curl to compile Teeworlds")
endif()

if(NOT(PYTHONINTERP_FOUND))
  message(SEND_ERROR "You must install Python to compile Teeworlds")
endif()

if(CLIENT AND NOT(FREETYPE_FOUND))
  message(SEND_ERROR "You must install Freetype to compile the Teeworlds client")
endif()

if(CLIENT AND NOT(OPUS_FOUND))
  message(SEND_ERROR "You must install Opus to compile the Teeworlds client")
endif()

if(CLIENT AND NOT(OPUSFILE_FOUND))
  message(SEND_ERROR "You must install Opusfile to compile the Teeworlds client")
endif()

if(CLIENT AND NOT(SDL2_FOUND))
  message(SEND_ERROR "You must install SDL2 to compile the Teeworlds client")
endif()

if(NOT(GTEST_FOUND))
  if(DOWNLOAD_GTEST)
    if(GIT_FOUND)
      message(STATUS "Automatically downloading GTest to be able to run tests")
    else()
      set(DOWNLOAD_GTEST OFF)
      message(WARNING "To automatically download GTest, you have to install Git")
    endif()
  else()
    message(STATUS "To run the tests, you have to install GTest")
  endif()
endif()

if(TARGET_OS STREQUAL "windows")
  set(PLATFORM_CLIENT)
  set(PLATFORM_CLIENT_LIBS opengl32 glu32 winmm)
  set(PLATFORM_LIBS ws2_32) # Windows sockets
elseif(TARGET_OS STREQUAL "mac")
  find_library(CARBON Carbon)
  find_library(COCOA Cocoa)
  find_library(OPENGL OpenGL)
  set(PLATFORM_CLIENT
    src/osxlaunch/client.h
    src/osxlaunch/client.m
  )
  set(PLATFORM_CLIENT_LIBS ${COCOA} ${OPENGL})
  set(PLATFORM_LIBS ${CARBON})
else()
  set(PLATFORM_CLIENT)
  find_package(OpenGL)
  find_package(X11)
  set(PLATFORM_CLIENT_LIBS ${OPENGL_gl_LIBRARY} ${OPENGL_glu_LIBRARY} ${X11_X11_LIB})
  set(PLATFORM_CLIENT_INCLUDE_DIRS ${OPENGL_INCLUDE_DIR} ${X11_X11_INCLUDE_PATH})
  if(TARGET_OS STREQUAL "linux")
    set(PLATFORM_LIBS rt) # clock_gettime for glibc < 2.17
  else()
    set(PLATFORM_LIBS)
  endif()
endif()

########################################################################
# DOWNLOAD GTEST
########################################################################

if(NOT(GTEST_FOUND) AND DOWNLOAD_GTEST)
  # Change to the 1.9.0 release tag once that works.
  set(DDNET_GTEST_VERSION 7b6561c56e353100aca8458d7bc49c4e0119bae8)
  configure_file(cmake/Download_GTest_CMakeLists.txt.in googletest-download/CMakeLists.txt)
  execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
    RESULT_VARIABLE result
    WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/googletest-download
  )
  if(result)
    message(WARNING "CMake step for googletest failed: ${result}")
    set(DOWNLOAD_GTEST OFF)
  else()
    execute_process(COMMAND ${CMAKE_COMMAND} --build .
      RESULT_VARIABLE result
      WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/googletest-download
    )
    if(result)
      message(WARNING "Build step for googletest failed: ${result}")
      set(DOWNLOAD_GTEST OFF)
    else()
      # Prevent overriding the parent project's compiler/linker settings on Windows
      set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

      # Add googletest directly to our build. This defines the gtest target.
      add_subdirectory(
        ${PROJECT_BINARY_DIR}/googletest-src
        ${PROJECT_BINARY_DIR}/googletest-build
        EXCLUDE_FROM_ALL
      )

      if(MSVC)
        foreach(target gtest)
          # `/w` disables all warnings. This is needed because `gtest` enables
          # `/WX` (equivalent of `-Werror`) for some reason, breaking builds
          # when MSVS adds new warnings.
          target_compile_options(${target} PRIVATE $<$<NOT:${DBG}>:/MT> $<${DBG}:/MTd> /w)
        endforeach()
      endif()

      set(GTEST_LIBRARIES gtest)
      set(GTEST_INCLUDE_DIRS)
      if(CMAKE_VERSION VERSION_LESS 2.8.11)
        set(GTEST_INCLUDE_DIRS "${gtest_SOURCE_DIR}/include")
      endif()
    endif()
  endif()
endif()

########################################################################
# DEPENDENCY COMPILATION
########################################################################

set_src(DEP_JSON_SRC GLOB src/engine/external/json-parser json.c json.h)
add_library(json EXCLUDE_FROM_ALL OBJECT ${DEP_JSON_SRC})

set_src(DEP_MD5_SRC GLOB src/engine/external/md5 md5.c md5.h)
add_library(md5 EXCLUDE_FROM_ALL OBJECT ${DEP_MD5_SRC})

list(APPEND TARGETS_DEP json md5)
set(DEP_JSON $<TARGET_OBJECTS:json>)
set(DEP_MD5)
if(NOT CRYPTO_FOUND)
  set(DEP_MD5 $<TARGET_OBJECTS:md5>)
endif()

########################################################################
# COPY DATA AND DLLS
########################################################################

file(COPY data DESTINATION .)
set(COPY_FILES
  ${CURL_COPY_FILES}
  ${FREETYPE_COPY_FILES}
  ${OPUSFILE_COPY_FILES}
  ${SDL2_COPY_FILES}
  ${FFMPEG_COPY_FILES}
)
file(COPY ${COPY_FILES} DESTINATION .)

########################################################################
# CODE GENERATION
########################################################################

function(chash output_file)
  add_custom_command(OUTPUT ${output_file}
    COMMAND ${PYTHON_EXECUTABLE} scripts/cmd5.py ${ARGN}
      > "${PROJECT_BINARY_DIR}/${output_file}"
    DEPENDS scripts/cmd5.py ${ARGN}
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  )
endfunction()

function(generate_source output_file script_parameter)
  add_custom_command(OUTPUT ${output_file}
    COMMAND ${PYTHON_EXECUTABLE} datasrc/compile.py ${script_parameter}
      > "${PROJECT_BINARY_DIR}/${output_file}"
    DEPENDS
      datasrc/compile.py
      datasrc/content.py
      datasrc/datatypes.py
      datasrc/network.py
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  )
endfunction()

file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/src/generated/")
if(GIT_FOUND)
  execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --git-dir
    ERROR_QUIET
    OUTPUT_VARIABLE PROJECT_GIT_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE PROJECT_GIT_DIR_ERROR
  )
  if(NOT PROJECT_GIT_DIR_ERROR)
    set(GIT_REVISION_EXTRA_DEPS
      ${PROJECT_GIT_DIR}/index
      ${PROJECT_GIT_DIR}/logs/HEAD
    )
  endif()
endif()
add_custom_command(OUTPUT ${PROJECT_BINARY_DIR}/src/generated/git_revision.cpp
  COMMAND ${PYTHON_EXECUTABLE}
    scripts/git_revision.py
    > ${PROJECT_BINARY_DIR}/src/generated/git_revision.cpp
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  DEPENDS
    ${GIT_REVISION_EXTRA_DEPS}
    scripts/git_revision.py
)
chash("src/generated/nethash.cpp"
  "src/engine/shared/protocol.h"
  "src/game/tuning.h"
  "src/game/gamecore.cpp"
  "${PROJECT_BINARY_DIR}/src/generated/protocol.h"
)
generate_source("src/generated/client_data.cpp" "client_content_source")
generate_source("src/generated/client_data.h" "client_content_header")
generate_source("src/generated/protocol.cpp" "network_source")
generate_source("src/generated/protocol.h" "network_header")
generate_source("src/generated/server_data.cpp" "server_content_source")
generate_source("src/generated/server_data.h" "server_content_header")


########################################################################
# SHARED
########################################################################

# Sources
set_src(BASE GLOB_RECURSE src/base
  another/kurhelper.h
  color.h
  detect.h
  hash.c
  hash.h
  hash_bundled.c
  hash_ctxt.h
  hash_libtomcrypt.c
  hash_openssl.c
  math.h
  stringable_enum.h
  system.c
  system.h
  tl/algorithm.h
  tl/allocator.h
  tl/array.h
  tl/base.h
  tl/range.h
  tl/sorted_array.h
  tl/string.h
  tl/threading.h
  threadpool.h
  vmath.h
)
set_src(ENGINE_INTERFACE GLOB src/engine
  client.h
  config.h
  console.h
  contacts.h
  demo.h
  editor.h
  engine.h
  graphics.h
  input.h
  kernel.h
  keys.h
  map.h
  masterserver.h
  message.h
  serverbrowser.h
  sound.h
  storage.h
  textrender.h
  updater.h
)
set_src(ENGINE_SHARED GLOB src/engine/shared
  compression.cpp
  compression.h
  config.cpp
  config.h
  config_variables.h
  console.cpp
  console.h
  datafile.cpp
  datafile.h
  demo.cpp
  demo.h
  econ.cpp
  econ.h
  engine.cpp
  filecollection.cpp
  filecollection.h
  huffman.cpp
  huffman.h
  jobs.cpp
  jobs.h
  kernel.cpp
  linereader.cpp
  linereader.h
  map.cpp
  mapchecker.cpp
  mapchecker.h
  masterserver.cpp
  memheap.cpp
  memheap.h
  message.h
  netban.cpp
  netban.h
  network.cpp
  network.h
  network_client.cpp
  network_conn.cpp
  network_console.cpp
  network_console_conn.cpp
  network_server.cpp
  network_token.cpp
  packer.cpp
  packer.h
  protocol.h
  ringbuffer.cpp
  ringbuffer.h
  snapshot.cpp
  snapshot.h
  storage.cpp
)
set(ENGINE_GENERATED_SHARED src/generated/nethash.cpp src/generated/protocol.cpp src/generated/protocol.h)
set_src(GAME_SHARED GLOB src/game
  collision.cpp
  collision.h
  enum_context.h
  enum_global.h
  gamecore.cpp
  gamecore.h
  layers.cpp
  layers.h
  mapitems.h
  tuning.h
  typedef_struct.h
  variables.h
  version.h
  voting.h
)
set(GAME_GENERATED_SHARED
  src/generated/git_revision.cpp
  src/generated/nethash.cpp
  src/generated/protocol.h
)

set(DEPS ${DEP_JSON} ${DEP_MD5} ${ZLIB_DEP})

# Libraries
set(LIBS ${CMAKE_THREAD_LIBS_INIT} ${ZLIB_LIBRARIES} ${CRYPTO_LIBRARIES} ${PLATFORM_LIBS})


# Targets
add_library(engine-shared EXCLUDE_FROM_ALL OBJECT ${ENGINE_INTERFACE} ${ENGINE_SHARED} ${ENGINE_GENERATED_SHARED} ${BASE})
add_library(game-shared EXCLUDE_FROM_ALL OBJECT ${GAME_SHARED} ${GAME_GENERATED_SHARED})
list(APPEND TARGETS_OWN engine-shared game-shared)

########################################################################
# SLEEPY DISCORD
########################################################################

#file(GLOB ENGINE_SLEEPY "src/teeother/sleepy_discord/*.cpp" "src/teeother/sleepy_discord/*.h")
#add_library(sleepy_discord EXCLUDE_FROM_ALL OBJECT  ${GAME_SHARED} ${GAME_GENERATED_SHARED} ${ENGINE_SLEEPY})

# Includes
#target_include_directories(sleepy_discord PRIVATE
#    ${asio_INCLUDE_DIR}
#    ${CRYPTO_INCLUDE_DIRS}
#    ${CPR_INCLUDE_DIR}
#    ${WEBSOCKETPP_INCLUDE_DIR})

#list(APPEND TARGETS_OWN engine-shared game-shared sleepy_discord)

########################################################################
# CLIENT
########################################################################

if(CLIENT)
  # Sources
  file(GLOB ENGINE_CLIENT "src/engine/client/*.cpp" "src/engine/client/*.h")
  file(GLOB GAME_CLIENT "src/game/client/*.cpp" "src/game/client/*.h" "src/game/client/components/*.cpp" "src/game/client/components/*.h")
  file(GLOB GAME_EDITOR "src/game/editor/*.cpp" "src/game/editor/*.h")
  set(GAME_GENERATED_CLIENT
    src/generated/client_data.cpp
    src/generated/client_data.h
  )
  set(CLIENT_SRC ${PLATFORM_CLIENT} ${ENGINE_CLIENT} ${GAME_CLIENT} ${GAME_EDITOR} ${GAME_GENERATED_CLIENT})

  set(DEPS_CLIENT ${DEPS} ${PNGLITE_DEP} ${WAVPACK_DEP})

  # Libraries
  set(LIBS_CLIENT
    ${LIBS}
    ${CURL_LIBRARIES}
    ${FREETYPE_LIBRARIES}
    ${PNGLITE_LIBRARIES}
    ${SDL2_LIBRARIES}
    ${WAVPACK_LIBRARIES}

    # Order of these three is important.
    ${OPUSFILE_LIBRARIES}
    ${OPUS_LIBRARIES}

    ${PLATFORM_CLIENT_LIBS}
  )

  if(TARGET_OS STREQUAL "windows")
    set(CLIENT_ICON "other/icons/${CLIENT_EXECUTABLE}.rc")
    if(NOT MINGW)
      set(CLIENT_MANIFEST "other/manifest/teeworlds.manifest")
    else()
      set(CLIENT_MANIFEST "other/manifest/teeworlds.rc")
    endif()
  else()
    set(CLIENT_ICON)
  endif()

  # Target
  set(TARGET_CLIENT ${CLIENT_EXECUTABLE})
  add_executable(${TARGET_CLIENT}
    ${CLIENT_SRC}
    ${CLIENT_ICON}
    ${CLIENT_MANIFEST}
    ${DEPS_CLIENT}
    $<TARGET_OBJECTS:engine-shared>
    $<TARGET_OBJECTS:game-shared>
  )
  target_link_libraries(${TARGET_CLIENT} ${LIBS_CLIENT})

  target_include_directories(${TARGET_CLIENT} PRIVATE
	${CURL_INCLUDE_DIRS}
    ${FREETYPE_INCLUDE_DIRS}
    ${OPUSFILE_INCLUDE_DIRS}
    ${OPUS_INCLUDE_DIRS}
    ${PNGLITE_INCLUDE_DIRS}
    ${SDL2_INCLUDE_DIRS}
    ${WAVPACK_INCLUDE_DIRS}
  )

  set(PARAMS "${WAVPACK_INCLUDE_DIRS};${WAVPACK_INCLUDE_DIRS}")
  if(NOT(WAVPACK_OPEN_FILE_INPUT_EX_PARAMS STREQUAL PARAMS))
    unset(WAVPACK_OPEN_FILE_INPUT_EX CACHE)
  endif()
  set(WAVPACK_OPEN_FILE_INPUT_EX_PARAMS "${PARAMS}" CACHE INTERNAL "")

  set(CMAKE_REQUIRED_INCLUDES ${ORIGINAL_CMAKE_REQUIRED_INCLUDES} ${WAVPACK_INCLUDE_DIRS})
  set(CMAKE_REQUIRED_LIBRARIES ${ORIGINAL_CMAKE_REQUIRED_LIBRARIES} ${WAVPACK_LIBRARIES})
  check_symbol_exists(WavpackOpenFileInputEx wavpack.h WAVPACK_OPEN_FILE_INPUT_EX)
  set(CMAKE_REQUIRED_INCLUDES ${ORIGINAL_CMAKE_REQUIRED_INCLUDES})
  set(CMAKE_REQUIRED_LIBRARIES ${ORIGINAL_CMAKE_REQUIRED_LIBRARIES})

  if(WAVPACK_OPEN_FILE_INPUT_EX)
    target_compile_definitions(${TARGET_CLIENT} PRIVATE CONF_WAVPACK_OPEN_FILE_INPUT_EX)
  endif()

  list(APPEND TARGETS_OWN ${TARGET_CLIENT})
  list(APPEND TARGETS_LINK ${TARGET_CLIENT})
endif()

########################################################################
# VARIOUS TARGETS
########################################################################

set_src(MASTERSRV_SRC GLOB src/mastersrv mastersrv.cpp mastersrv.h)
set_src(VERSIONSRV_SRC GLOB src/versionsrv mapversions.h versionsrv.cpp versionsrv.h)
list(APPEND VERSIONSRV_SRC ${PROJECT_BINARY_DIR}/src/generated/nethash.cpp)

set(TARGET_MASTERSRV mastersrv)
set(TARGET_VERSIONSRV versionsrv)

add_executable(${TARGET_MASTERSRV} EXCLUDE_FROM_ALL ${MASTERSRV_SRC} $<TARGET_OBJECTS:engine-shared> ${DEPS})
add_executable(${TARGET_VERSIONSRV} EXCLUDE_FROM_ALL ${VERSIONSRV_SRC} $<TARGET_OBJECTS:engine-shared> ${DEPS})

target_link_libraries(${TARGET_MASTERSRV} ${LIBS})
target_link_libraries(${TARGET_VERSIONSRV} ${LIBS})

list(APPEND TARGETS_OWN ${TARGET_MASTERSRV} ${TARGET_VERSIONSRV})
list(APPEND TARGETS_LINK ${TARGET_MASTERSRV} ${TARGET_VERSIONSRV})

set(TARGETS_TOOLS)
set_src(TOOLS GLOB src/tools
  crapnet.cpp
  fake_server.cpp
  map_resave.cpp
  map_version.cpp
  packetgen.cpp
)
foreach(ABS_T ${TOOLS})
  file(RELATIVE_PATH T "${PROJECT_SOURCE_DIR}/src/tools/" ${ABS_T})
  if(T MATCHES "\\.cpp$")
    string(REGEX REPLACE "\\.cpp$" "" TOOL "${T}")
    add_executable(${TOOL} EXCLUDE_FROM_ALL
      ${DEPS}
      src/tools/${TOOL}.cpp
      ${EXTRA_TOOL_SRC}
      $<TARGET_OBJECTS:engine-shared>
    )
    target_link_libraries(${TOOL} ${LIBS})
    list(APPEND TARGETS_TOOLS ${TOOL})
  endif()
endforeach()

list(APPEND TARGETS_OWN ${TARGETS_TOOLS})
list(APPEND TARGETS_LINK ${TARGETS_TOOLS})

add_custom_target(tools DEPENDS ${TARGETS_TOOLS})
add_custom_target(everything DEPENDS ${TARGETS_OWN})

########################################################################
# TESTS
########################################################################

if(GTEST_FOUND OR DOWNLOAD_GTEST)
  set_src(TESTS GLOB src/test
    fs.cpp
    git_revision.cpp
    hash.cpp
    storage.cpp
    str.cpp
    test.cpp
    test.h
    thread.cpp
  )
  set(TARGET_TESTRUNNER testrunner)
  add_executable(${TARGET_TESTRUNNER} EXCLUDE_FROM_ALL
    ${TESTS}
    $<TARGET_OBJECTS:engine-shared>
    $<TARGET_OBJECTS:game-shared>
    ${DEPS}
  )
  target_link_libraries(${TARGET_TESTRUNNER} ${LIBS} ${GTEST_LIBRARIES})
  target_include_directories(${TARGET_TESTRUNNER} PRIVATE ${GTEST_INCLUDE_DIRS})

  list(APPEND TARGETS_OWN ${TARGET_TESTRUNNER})
  list(APPEND TARGETS_LINK ${TARGET_TESTRUNNER})

  add_custom_target(run_tests
    COMMAND $<TARGET_FILE:${TARGET_TESTRUNNER}> ${TESTRUNNER_ARGS}
    COMMENT Running tests
    DEPENDS ${TARGET_TESTRUNER}
    USES_TERMINAL
  )
endif()

########################################################################
# INSTALLATION
########################################################################

function(escape_regex VAR STRING)
  string(REGEX REPLACE "([][^$.+*?|()\\\\])" "\\\\\\1" ESCAPED "${STRING}")
  set(${VAR} ${ESCAPED} PARENT_SCOPE)
endfunction()

function(escape_backslashes VAR STRING)
  string(REGEX REPLACE "\\\\" "\\\\\\\\" ESCAPED "${STRING}")
  set(${VAR} ${ESCAPED} PARENT_SCOPE)
endfunction()

function(max_length VAR)
  set(MAX_LENGTH 0)
  foreach(str ${ARGN})
    string(LENGTH ${str} LENGTH)
    if(LENGTH GREATER MAX_LENGTH)
      set(MAX_LENGTH ${LENGTH})
    endif()
  endforeach()
  set(${VAR} ${MAX_LENGTH} PARENT_SCOPE)
endfunction()

# Tries to generate a list of regex that matches everything except the given
# parameters.
function(regex_inverted VAR)
  max_length(MAX_LENGTH ${ARGN})
  math(EXPR UPPER_BOUND "${MAX_LENGTH}-1")

  set(REMAINING ${ARGN})
  set(RESULT)

  foreach(i RANGE ${UPPER_BOUND})
    set(TEMP ${REMAINING})
    set(REMAINING)
    foreach(str ${TEMP})
      string(LENGTH ${str} LENGTH)
      if(i LESS LENGTH)
        list(APPEND REMAINING ${str})
      endif()
    endforeach()

    set(ADDITIONAL)
    foreach(outer ${REMAINING})
      string(SUBSTRING ${outer} 0 ${i} OUTER_PREFIX)
      set(CHARS "")
      foreach(inner ${REMAINING})
        string(SUBSTRING ${inner} 0 ${i} INNER_PREFIX)
        if(OUTER_PREFIX STREQUAL INNER_PREFIX)
          string(SUBSTRING ${inner} ${i} 1 INNER_NEXT)
          set(CHARS "${CHARS}${INNER_NEXT}")
        endif()
      endforeach()
      escape_regex(OUTER_PREFIX_ESCAPED "${OUTER_PREFIX}")

      list(APPEND ADDITIONAL "${OUTER_PREFIX_ESCAPED}([^${CHARS}]|$)")
    endforeach()
    list(REMOVE_DUPLICATES ADDITIONAL)
    list(APPEND RESULT ${ADDITIONAL})
  endforeach()
  set(${VAR} ${RESULT} PARENT_SCOPE)
endfunction()

set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_GENERATOR TGZ TXZ)
set(CPACK_ARCHIVE_COMPONENT_INSTALL ON)
set(CPACK_STRIP_FILES TRUE)
set(CPACK_COMPONENTS_ALL portable)
set(CPACK_SOURCE_GENERATOR ZIP TGZ TBZ2 TXZ)
set(CPACK_SYSTEM_NAME ${CMAKE_SYSTEM_NAME})

if(TARGET_OS AND TARGET_BITS)
  if(TARGET_OS STREQUAL "windows")
    set(CPACK_SYSTEM_NAME "win${TARGET_BITS}")
    set(CPACK_GENERATOR ZIP)
  elseif(TARGET_OS STREQUAL "linux")
    # Assuming Intel here.
    if(TARGET_BITS EQUAL 32)
      set(CPACK_SYSTEM_NAME "linux_x86")
    elseif(TARGET_BITS EQUAL 64)
      set(CPACK_SYSTEM_NAME "linux_x86_64")
    endif()
  elseif(TARGET_OS STREQUAL "mac")
    set(CPACK_SYSTEM_NAME "osx")
    set(CPACK_GENERATOR DMG)
  endif()
endif()

set(CPACK_PACKAGE_FILE_NAME ${CPACK_PACKAGE_NAME}-${CPACK_SYSTEM_NAME})
set(CPACK_ARCHIVE_PORTABLE_FILE_NAME ${CPACK_PACKAGE_FILE_NAME})
set(CPACK_SOURCE_PACKAGE_FILE_NAME ${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-src)
set(CPACK_SOURCE_FILES
  CMakeLists.txt
  bam.lua
  cmake/
  configure.lua
  data/
  datasrc/
  license.txt
  other/
  readme.md
  scripts/
  src/
  storage.cfg
)
set(CPACK_SOURCE_IGNORE_FILES
  "\\\\.o$"
  "\\\\.pyc$"
  "/\\\\.git"
  "/__pycache__/"
)

regex_inverted(CPACK_SOURCE_FILES_INVERTED ${CPACK_SOURCE_FILES})
escape_regex(PROJECT_SOURCE_DIR_ESCAPED ${PROJECT_SOURCE_DIR})

foreach(str ${CPACK_SOURCE_FILES_INVERTED})
  escape_backslashes(STR_ESCAPED "${PROJECT_SOURCE_DIR_ESCAPED}/${str}")
  list(APPEND CPACK_SOURCE_IGNORE_FILES "${STR_ESCAPED}")
endforeach()

set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME ${PROJECT_NAME})

set(CPACK_TARGETS
  ${TARGET_CLIENT}
)
set(CPACK_DIRS data)
set(CPACK_FILES
  license.txt
  storage.cfg
  ${COPY_FILES}
)
if(TARGET_OS STREQUAL "windows")
  list(APPEND CPACK_FILES other/config_directory.bat)
endif()

if(NOT DEV)
  install(DIRECTORY data DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME} COMPONENT data)
  install(TARGETS ${TARGET_CLIENT} DESTINATION bin COMPONENT client)
endif()

if(DEV)
  # Don't generate CPack targets.
elseif(CMAKE_VERSION VERSION_LESS 3.6 OR CMAKE_VERSION VERSION_EQUAL 3.6)
  message(WARNING "Cannot create CPack targets, CMake version too old. Use CMake 3.6 or newer.")
else()
  set(EXTRA_ARGS DESTINATION ${CPACK_PACKAGE_FILE_NAME} COMPONENT portable EXCLUDE_FROM_ALL)
  install(TARGETS ${CPACK_TARGETS} ${EXTRA_ARGS})
  install(DIRECTORY ${CPACK_DIRS} ${EXTRA_ARGS})
  install(FILES ${CPACK_FILES} ${EXTRA_ARGS})
endif()

set(PACKAGE_TARGETS)

if(CLIENT AND (DMGTOOLS_FOUND OR HDIUTIL))
  file(MAKE_DIRECTORY bundle/client/)
  configure_file(other/bundle/client/Info.plist.in bundle/client/Info.plist)

  if(HDIUTIL)
    set(DMG_PARAMS --hdiutil ${HDIUTIL})
  elseif(DMGTOOLS_FOUND)
    set(DMG_PARAMS --dmgtools ${DMG} ${HFSPLUS} ${NEWFS_HFS})
  endif()
  set(DMG_TMPDIR pack_${CPACK_PACKAGE_FILE_NAME}_dmg)
  set(DMG_MKDIRS
    ${TARGET_CLIENT}.app
    ${TARGET_CLIENT}.app/Contents
    ${TARGET_CLIENT}.app/Contents/Frameworks
    ${TARGET_CLIENT}.app/Contents/MacOS
    ${TARGET_CLIENT}.app/Contents/Resources
    ${TARGET_CLIENT}.app/Contents/Resources/data
  )
  set(DMG_MKDIR_COMMANDS)
  foreach(dir ${DMG_MKDIRS})
    list(APPEND DMG_MKDIR_COMMANDS COMMAND ${CMAKE_COMMAND} -E make_directory ${DMG_TMPDIR}/${dir})
  endforeach()
  add_custom_command(OUTPUT ${CPACK_PACKAGE_FILE_NAME}.dmg
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${DMG_TMPDIR}
    ${DMG_MKDIR_COMMANDS}

    # CLIENT
    COMMAND ${CMAKE_COMMAND} -E copy_directory data ${DMG_TMPDIR}/${TARGET_CLIENT}.app/Contents/Resources/data
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/other/icons/${TARGET_CLIENT}.icns ${DMG_TMPDIR}/${TARGET_CLIENT}.app/Contents/Resources/
    COMMAND ${CMAKE_COMMAND} -E copy bundle/client/Info.plist ${PROJECT_SOURCE_DIR}/other/bundle/client/PkgInfo ${DMG_TMPDIR}/${TARGET_CLIENT}.app/Contents/
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${TARGET_CLIENT}> ${DMG_TMPDIR}/${TARGET_CLIENT}.app/Contents/MacOS/
    COMMAND ${CMAKE_COMMAND} -E copy ${SDL2_LIBRARY} ${DMG_TMPDIR}/${TARGET_CLIENT}.app/Contents/Frameworks/libSDL2-2.0.0.dylib
    COMMAND ${CMAKE_COMMAND} -E copy ${FREETYPE_LIBRARY} ${DMG_TMPDIR}/${TARGET_CLIENT}.app/Contents/Frameworks/libfreetype.6.dylib
    COMMAND ${PYTHON_EXECUTABLE} ${PROJECT_SOURCE_DIR}/scripts/darwin_change_dylib.py change --tools ${CMAKE_INSTALL_NAME_TOOL} ${CMAKE_OTOOL} ${DMG_TMPDIR}/${TARGET_CLIENT}.app/Contents/MacOS/${TARGET_CLIENT} SDL2 @executable_path/../Frameworks/libSDL2-2.0.0.dylib
    COMMAND ${PYTHON_EXECUTABLE} ${PROJECT_SOURCE_DIR}/scripts/darwin_change_dylib.py change --tools ${CMAKE_INSTALL_NAME_TOOL} ${CMAKE_OTOOL} ${DMG_TMPDIR}/${TARGET_CLIENT}.app/Contents/MacOS/${TARGET_CLIENT} libfreetype @executable_path/../Frameworks/libfreetype.6.dylib

    # DMG
    COMMAND ${PYTHON_EXECUTABLE} ${PROJECT_SOURCE_DIR}/scripts/dmg.py create ${DMG_PARAMS} ${CPACK_PACKAGE_FILE_NAME}.dmg ${CPACK_PACKAGE_FILE_NAME} ${DMG_TMPDIR}

    DEPENDS
      ${TARGET_CLIENT}
      ${PROJECT_BINARY_DIR}/bundle/client/Info.plist
      data
      other/bundle/client/PkgInfo
      other/icons/${TARGET_CLIENT}.icns
      scripts/dmg.py
  )
  add_custom_target(package_dmg DEPENDS ${CPACK_PACKAGE_FILE_NAME}.dmg)
  list(APPEND PACKAGE_TARGETS package_dmg)
endif()

foreach(ext zip tar.gz tar.xz)
  set(TAR_MODE c)
  set(TAR_EXTRA_ARGS)
  string(REPLACE . _ EXT_SLUG ${ext})

  set(TMPDIR pack_${CPACK_PACKAGE_FILE_NAME}_${EXT_SLUG}/${CPACK_PACKAGE_FILE_NAME})

  set(COPY_FILE_COMMANDS)
  set(COPY_DIR_COMMANDS)
  set(COPY_TARGET_COMMANDS)
  set(STRIP_TARGET_COMMANDS)
  foreach(file ${CPACK_FILES})
    list(APPEND COPY_FILE_COMMANDS COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/${file} ${TMPDIR}/)
  endforeach()
  foreach(dir ${CPACK_DIRS})
    get_filename_component(NAME ${dir} NAME)
    list(APPEND COPY_DIR_COMMANDS COMMAND ${CMAKE_COMMAND} -E copy_directory ${dir} ${TMPDIR}/${NAME})
  endforeach()
  foreach(target ${CPACK_TARGETS})
    list(APPEND COPY_TARGET_COMMANDS COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${target}> ${TMPDIR}/)
  endforeach()

  if(ext STREQUAL zip)
    set(TAR_EXTRA_ARGS --format=zip)
  elseif(ext STREQUAL tar.gz)
    set(TAR_MODE cz)
  elseif(ext STREQUAL tar.xz)
    set(TAR_MODE cJ)
  endif()
  add_custom_command(OUTPUT ${CPACK_PACKAGE_FILE_NAME}.${ext}
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${TMPDIR}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${TMPDIR}
    ${COPY_FILE_COMMANDS}
    ${COPY_DIR_COMMANDS}
    ${COPY_TARGET_COMMANDS}
    ${STRIP_TARGET_COMMANDS}
    COMMAND ${CMAKE_COMMAND} -E chdir pack_${CPACK_PACKAGE_FILE_NAME}_${EXT_SLUG} ${CMAKE_COMMAND} -E tar ${TAR_MODE} ../${CPACK_PACKAGE_FILE_NAME}.${ext} ${TAR_EXTRA_ARGS} -- ${CPACK_PACKAGE_FILE_NAME}/
    DEPENDS ${CPACK_TARGETS}
  )
  add_custom_target(package_${EXT_SLUG} DEPENDS ${CPACK_PACKAGE_FILE_NAME}.${ext})
  list(APPEND PACKAGE_TARGETS package_${EXT_SLUG})
endforeach()

set(PACKAGE_DEFAULT tar_xz)
if(TARGET_OS STREQUAL "windows")
  set(PACKAGE_DEFAULT zip)
elseif(TARGET_OS STREQUAL "mac")
  set(PACKAGE_DEFAULT dmg)
endif()
add_custom_target(package_default DEPENDS package_${PACKAGE_DEFAULT})

add_custom_target(package_all DEPENDS ${PACKAGE_TARGETS})

# Unset these variables, they might do something in the future of CPack.
unset(CPACK_SOURCE_FILES)
unset(CPACK_SOURCE_FILES_INVERTED)
unset(CPACK_TARGETS)
unset(CPACK_DIRS)
unset(CPACK_FILES)

include(CPack)

########################################################################
# COMPILER-SPECIFICS
########################################################################

# In the future (CMake 3.8.0+), use source_group(TREE ...)
macro(source_group_tree dir)
  file(GLOB ents RELATIVE ${PROJECT_SOURCE_DIR}/${dir} ${PROJECT_SOURCE_DIR}/${dir}/*)
  foreach(ent ${ents})
    if(IS_DIRECTORY ${PROJECT_SOURCE_DIR}/${dir}/${ent})
      source_group_tree(${dir}/${ent})
    else()
      string(REPLACE "/" "\\" group ${dir})
      source_group(${group} FILES ${PROJECT_SOURCE_DIR}/${dir}/${ent})
    endif()
  endforeach()
endmacro()
source_group_tree(src)

set(TARGETS ${TARGETS_OWN} ${TARGETS_DEP})

foreach(target ${TARGETS})
  if(MSVC)
    if(POLICY CMP0091)
      set_property(TARGET ${target} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<${DBG}:Debug>")
    else()
      target_compile_options(${target} PRIVATE $<$<NOT:${DBG}>:/MT> $<${DBG}:/MTd>)
    endif()
    target_compile_options(${target} PRIVATE /MP) # Use multiple cores
    target_compile_options(${target} PRIVATE /EHsc) # Only catch C++ exceptions with catch.
    target_compile_options(${target} PRIVATE /GS) # Protect the stack pointer.
    target_compile_options(${target} PRIVATE /wd4996) # Use of non-_s functions.
  endif()
  if(OUR_FLAGS)
    target_compile_options(${target} PRIVATE ${OUR_FLAGS})
  endif()
  if(DEFINE_FORTIFY_SOURCE)
    target_compile_definitions(${target} PRIVATE $<$<NOT:$<CONFIG:Debug>>:_FORTIFY_SOURCE=2>) # Detect some buffer overflows.
  endif()
  target_compile_definitions(${target} PRIVATE _GLIBCXX_ASSERTIONS) # Enable run-time bounds-checking for the STL
endforeach()

foreach(target ${TARGETS_LINK})
  if(MSVC)
    set_property(TARGET ${target} APPEND PROPERTY LINK_FLAGS /SAFESEH:NO) # Disable SafeSEH because the shipped libraries don't support it (would cause error LNK2026 otherwise).
  endif()
  if(TARGET_OS STREQUAL "mac")
    target_link_libraries(${target} -stdlib=libc++)
    target_link_libraries(${target} -mmacosx-version-min=10.7)
    # Fixes: Undefined symbols for architecture x86_64:
    #   "___isOSVersionAtLeast", referenced from:
    #       _singleipconnect in libcurl.a(libcurl_la-connect.o)
    target_link_libraries(${target} -Wl,-undefined -Wl,dynamic_lookup)
  endif()
  if((MINGW OR TARGET_OS STREQUAL "linux") AND PREFER_BUNDLED_LIBS)
    # Statically link the standard libraries with on MinGW/Linux so we don't
    # have to ship them as DLLs.
    target_link_libraries(${target} -static-libgcc)
    target_link_libraries(${target} -static-libstdc++)
    if(MINGW)
      # Link pthread library statically instead of dynamically.
      # Solution from https://stackoverflow.com/a/28001261.
      target_link_libraries(${target} -Wl,-Bstatic -lstdc++ -lpthread -Wl,-Bdynamic)
    endif()
  endif()
endforeach()

foreach(target ${TARGETS_OWN})
    if(MSVC)
        target_compile_options(${target} PRIVATE /W3)
        target_compile_options(${target} PRIVATE /wd4244) # Possible loss of data (float -> int, int -> float, etc.).
        target_compile_options(${target} PRIVATE /wd4267) # Possible loss of data (size_t - int on win64).
        target_compile_options(${target} PRIVATE /wd4800) # Implicit conversion of int to bool.
    endif()

    if(OUR_FLAGS_OWN)
        target_compile_options(${target} PRIVATE ${OUR_FLAGS_OWN})
    endif()

    target_include_directories(${target} PRIVATE ${PROJECT_BINARY_DIR}/src)
    target_include_directories(${target} PRIVATE src)
    target_compile_definitions(${target} PRIVATE $<$<CONFIG:Debug>:CONF_DEBUG>)
    target_include_directories(${target} PRIVATE ${ZLIB_INCLUDE_DIRS})
    if(CRYPTO_FOUND)
        target_compile_definitions(${target} PRIVATE CONF_OPENSSL)
        target_include_directories(${target} PRIVATE ${CRYPTO_INCLUDE_DIRS})
    endif()

endforeach()

foreach(target ${TARGETS_DEP})
  if(MSVC)
    target_compile_options(${target} PRIVATE /W0)
  endif()
  if(OUR_FLAGS_DEP)
    target_compile_options(${target} PRIVATE ${OUR_FLAGS_DEP})
  endif()
endforeach()
