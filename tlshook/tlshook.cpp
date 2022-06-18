/*
 * TLSHook
 * @author 	: keith@robot9.me
 *
 */
#include "tlshook.h"

#include <sys/system_properties.h>
#include <unordered_map>
#include <string>

#include "common.h"
#include "tls.h"
#include "eglcore.h"


/** entries.in commit logs
8.0-28 1381b18 Update EGL and GLES headers. by Krzysztof Kosiński · 4 years, 1 month ago

7.0-24 4690754 opengl: Update headers and registry and regenerate *api.in by Pablo Ceballos · 6 years ago

5.0-21 fca1b54 opengl: Regenerate code with ES 3.1 and new extensions by Jesse Hall · 8 years ago
3703f7f opengl: Update GLES headers and generate *.in from registry XML by Jesse Hall · 8 years ago
0b96e6c Revert "opengl: Generate *.in from registry XML" by Jesse Hall · 8 years ago
4a73962 opengl: Generate *.in from registry XML by Jesse Hall · 8 years ago

4.3-18 4774338 Add ES3 support to libGLESv2 and tracing tools by Jesse Hall · 9 years ago
d58974c Regenerate API/trace files for constness change by Jesse Hall · 9 years ago
300ad09 Fix return type of glGetAttribLocation and glGetUniformLocation by Andrew Hsieh · 9 years ago

4.1-16 3127260 update GLES headers and add support for corresponding new extensions. by Mathias Agopian · 10 years ago
11cca92 update GL ES stub libraries with the new GL ES headers by Mathias Agopian · 12 years ago
618fa10 fix [2187212] add support for GLESv2 dispatch based on TLS by Mathias Agopian · 13 years ago
 */

namespace TLSHook {

#define TLS_SLOT_OPENGL_API_LOWER  3
#define TLS_SLOT_OPENGL_API_29     4

struct HookFuncEntry {
  std::string name;
  int idx = -1;
  size_t *origin = nullptr;
  size_t *hooked = nullptr;
};

static size_t *tlsPtr = nullptr;
static std::unordered_map<std::string, int> hookMap;
static std::unordered_map<std::string, HookFuncEntry> hookFunctions;

void *volatile *get_tls_hooks() {
  volatile void *tls_base = __get_tls();
  void *volatile *tls_hooks = reinterpret_cast<void *volatile *>(tls_base);
  return tls_hooks;
}

void *getGlThreadSpecific(int idx) {
  void *volatile *tls_hooks = get_tls_hooks();
  void *hooks = tls_hooks[idx];
  return hooks;
}

bool tls_hook_init() {
  if (tlsPtr == nullptr) {
    EGLCore egl_core;
    bool needCreateEGL = (eglGetCurrentContext() == EGL_NO_CONTEXT);
    if (needCreateEGL) {
      LOGI("Init egl ...");
      // init egl
      if (!egl_core.create(1, 1)) {
        LOGE("Init egl failed");
      }
      egl_core.makeCurrent();
      LOGI("Init egl done");
    }

    char sdk[128] = "0";
    __system_property_get("ro.build.version.sdk", sdk);
    int api_level = atoi(sdk);
    LOGI("ro.build.version.sdk: %s", sdk);
    int tlsIdx = api_level >= 29 ? TLS_SLOT_OPENGL_API_29 : TLS_SLOT_OPENGL_API_LOWER;

    // get tls ptr
    tlsPtr = static_cast<size_t *>(getGlThreadSpecific(tlsIdx));
    if (tlsPtr == nullptr) {
      LOGE("Error getGlThreadSpecific nullptr");
      return false;
    }

    int idx = 0;

#define GL_ENTRY(_r, _api, ...) hookMap[#_api] = idx++;
    if (api_level >= 28) {
#include "entry/entries.28.in"
    } else if (api_level >= 24) {
#include "entry/entries.24.in"
    } else if (api_level >= 21) {
#include "entry/entries.21.in"
    } else if (api_level >= 18) {
#include "entry/entries.18.in"
    } else if (api_level >= 16) {
#include "entry/entries.16.in"
    }

    if (needCreateEGL) {
      egl_core.destroy();
      LOGI("Destroy egl");
    }
  }

  LOGI("hook init success");
  return true;
}

bool tls_hook_func(const char *symbol, void *new_func, void **old_func) {
  HookFuncEntry entry;
  entry.name = symbol;
  auto it = hookMap.find(entry.name);
  if (it == hookMap.end()) {
    LOGE("Hook failed: symbol not found: %s", symbol);
    return false;
  }
  entry.idx = it->second;
  size_t *slot = tlsPtr + entry.idx;

  *old_func = entry.origin = reinterpret_cast<size_t *>(*slot);
  entry.hooked = static_cast<size_t *>(new_func);
  *slot = reinterpret_cast<size_t>(new_func);
  hookFunctions[entry.name] = entry;

  LOGI("Hook success: symbol: %s", symbol);
  return true;
}

void tls_hook_clear() {
  for (auto &it : hookFunctions) {
    auto &entry = it.second;
    size_t *slot = tlsPtr + entry.idx;
    *slot = reinterpret_cast<size_t>(entry.origin);
  }
  hookFunctions.clear();
  LOGI("hook clear");
}

}