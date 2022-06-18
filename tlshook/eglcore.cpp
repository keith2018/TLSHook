/*
 * TLSHook
 * @author 	: keith@robot9.me
 *
 */
#include "eglcore.h"
#include "common.h"

namespace TLSHook {

bool EGLCore::create(int width, int height, EGLContext sharedContext) {
  if (mEGLDisplay != EGL_NO_DISPLAY) {
    LOGE("EGL already set up");
    return false;
  }
  if (sharedContext == nullptr) {
    sharedContext = EGL_NO_CONTEXT;
  }

  mEGLDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
  if (mEGLDisplay == EGL_NO_DISPLAY) {
    LOGE("unable to get EGL display.");
    return false;
  }

  if (!eglInitialize(mEGLDisplay, nullptr, nullptr)) {
    mEGLDisplay = EGL_NO_DISPLAY;
    LOGE("unable to initialize EGL");
    return false;
  }

  // Try GLES3
  EGLConfig config = getConfig(3);
  if (config != nullptr) {
    int attrib3_list[] = {
        EGL_CONTEXT_CLIENT_VERSION, 3,
        EGL_NONE
    };
    EGLContext context = eglCreateContext(mEGLDisplay, config, sharedContext, attrib3_list);
    if (eglGetError() == EGL_SUCCESS) {
      mEGLConfig = config;
      mEGLContext = context;
    }
  }

  // Back to GLES2
  if (mEGLContext == EGL_NO_CONTEXT) {
    config = getConfig(2);
    int attrib2_list[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };
    EGLContext context = eglCreateContext(mEGLDisplay, config, sharedContext, attrib2_list);
    if (eglGetError() == EGL_SUCCESS) {
      mEGLConfig = config;
      mEGLContext = context;
    }
  }

  int values[1] = {0};
  eglQueryContext(mEGLDisplay, mEGLContext, EGL_CONTEXT_CLIENT_VERSION, values);
  LOGD("EGLContext created, client version %d", values[0]);

  mEGLSurface = createOffscreenSurface(width, height);
  return mEGLSurface != nullptr;
}

void EGLCore::destroy() {
  releaseSurface(mEGLSurface);

  if (mEGLDisplay != EGL_NO_DISPLAY) {
    eglMakeCurrent(mEGLDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(mEGLDisplay, mEGLContext);
  }

  mEGLDisplay = EGL_NO_DISPLAY;
  mEGLContext = EGL_NO_CONTEXT;
  mEGLConfig = nullptr;
}

void EGLCore::makeCurrent() {
  if (mEGLDisplay == EGL_NO_DISPLAY) {
    LOGD("Note: makeCurrent w/o display.");
  }
  if (!eglMakeCurrent(mEGLDisplay, mEGLSurface, mEGLSurface, mEGLContext)) {
    LOGE("Error: makeCurrent w/o display.");
  }
}

EGLConfig EGLCore::getConfig(int version) {
  int renderableType = EGL_OPENGL_ES2_BIT;
  if (version >= 3) {
    renderableType |= EGL_OPENGL_ES3_BIT_KHR;
  }
  int attribList[] = {
      EGL_RED_SIZE, 8,
      EGL_GREEN_SIZE, 8,
      EGL_BLUE_SIZE, 8,
      EGL_ALPHA_SIZE, 8,
      //EGL_DEPTH_SIZE, 16,
      //EGL_STENCIL_SIZE, 8,
      EGL_RENDERABLE_TYPE, renderableType,
      EGL_NONE, 0,
      EGL_NONE
  };

  EGLConfig configs = nullptr;
  int numConfigs;
  if (!eglChooseConfig(mEGLDisplay, attribList, &configs, 1, &numConfigs)) {
    LOGW("unable to find RGB8888 / %d  EGLConfig", version);
  }
  return configs;
}

EGLSurface EGLCore::createOffscreenSurface(int width, int height) {
  int surfaceAttribs[] = {
      EGL_WIDTH, width,
      EGL_HEIGHT, height,
      EGL_NONE
  };
  EGLSurface eglSurface = eglCreatePbufferSurface(mEGLDisplay, mEGLConfig, surfaceAttribs);
  if (eglSurface == nullptr) {
    LOGE("EGLSurface nullptr");
  }
  return eglSurface;
}

void EGLCore::releaseSurface(EGLSurface eglSurface) {
  if (mEGLSurface) {
    eglDestroySurface(mEGLDisplay, eglSurface);
  }
}

}