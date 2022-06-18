/*
 * TLSHook
 * @author 	: keith@robot9.me
 *
 */
#pragma once

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <EGL/eglplatform.h>

namespace TLSHook {

class EGLCore {
 public:
  bool create(int width, int height, EGLContext sharedContext = nullptr);
  void destroy();
  void makeCurrent();

 private:
  EGLConfig getConfig(int version);
  EGLSurface createOffscreenSurface(int width, int height);
  void releaseSurface(EGLSurface eglSurface);

 private:
  EGLDisplay mEGLDisplay = EGL_NO_DISPLAY;
  EGLConfig mEGLConfig = nullptr;
  EGLContext mEGLContext = EGL_NO_CONTEXT;
  EGLSurface mEGLSurface = nullptr;
};

}
