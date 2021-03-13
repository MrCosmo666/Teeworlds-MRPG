#ifndef BASE_STDAFX_PCH_H
#define BASE_STDAFX_PCH_H

/*
 * I don't know if we'll need a pch.
 * Since the inclusions are mixed in and out.
 * But at least it will be more convenient and we don't have to take care of every switch.
 * And it will only be assembled once and used in all project objects as a precompiled header.
*/

// based teeworlds
#include <base/detect.h>
#include <base/hash.h>
#include <base/hash_ctxt.h>
#include <base/math.h>
#include <base/vmath.h>
#include <base/tl/array.h>
#include <base/tl/sorted_array.h>
#include <base/tl/threading.h>
#include <teeother/tl/nlohmann_json.h>

#if defined(CONF_FAMILY_WINDOWS)
  #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
  #endif
#endif

// based cxx includes
#include <algorithm>
#include <atomic>
#include <cstdarg>
#include <list>
#include <map>
#include <mutex>
#include <thread>
#include <vector>

#ifdef IMAGE_CURSOR
  #undef IMAGE_CURSOR
#endif

#ifdef COLOR_BACKGROUND
  #undef COLOR_BACKGROUND
#endif

#ifdef max
  #undef max
#endif

#endif
