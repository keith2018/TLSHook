/*
 * TLSHook
 * @author 	: keith@robot9.me
 *
 */
#include <jni.h>
#include <string>
#include <EGL/egl.h>
#include <GLES2/gl2.h>

#include "common.h"
#include "tlshook.h"

// origin function
PFNGLCLEARCOLORPROC cb_glClearColor = nullptr;

EGLContext cb_eglContext = nullptr;

// new function
void hook_glClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) {
  if (cb_glClearColor) {
    EGLContext eglContext = eglGetCurrentContext();
      if (eglContext == cb_eglContext) {
        LOGD("hook call glClearColor: (%f, %f, %f, %f)", red, green, blue, alpha);
        cb_glClearColor(0.f, 0.f, 1.f, 1.f);
      } else {
        LOGD("ignore call glClearColor: (%f, %f, %f, %f)", red, green, blue, alpha);
        cb_glClearColor(red, green, blue, alpha);
      }
  }
}

extern "C" JNIEXPORT jboolean JNICALL
Java_me_robot9_tlshook_MainActivity_hookTestStart(JNIEnv *env,
                                                  jobject /* this */) {
  bool success = TLSHook::tls_hook_init();
  if (success) {
    success = TLSHook::tls_hook_func("glClearColor",
                                     (void *) hook_glClearColor,
                                     (void **) &cb_glClearColor);
  }
  LOGD("hookTestStart: %s", success ? "true" : "false");
  return success ? JNI_TRUE : JNI_FALSE;
}

extern "C"
JNIEXPORT void JNICALL
Java_me_robot9_tlshook_MainActivity_hookEGLContext(JNIEnv *env, jobject thiz) {
  cb_eglContext = eglGetCurrentContext();
}

extern "C" JNIEXPORT void JNICALL
Java_me_robot9_tlshook_MainActivity_hookTestStop(
    JNIEnv *env,
    jobject /* this */) {
  cb_eglContext = nullptr;
  TLSHook::tls_hook_clear();
  LOGD("hookTestStop");
}
