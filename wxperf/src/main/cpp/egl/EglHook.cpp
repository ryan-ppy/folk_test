//
// Created by 邓沛堆 on 2020-05-06.
//

#include "EglHook.h"
#include "Log.h"
#include <EGL/egl.h>

#define ORIGINAL_LIB "libEGL.so"
#define TAG "EGL"

const size_t BUF_SIZE = 1024;

jstring charTojstring(JNIEnv *env, const char *pat) {

//    jclass strClass = (env)->FindClass("java/lang/String");
//    jmethodID ctorID = (env)->GetMethodID(strClass, "<init>", "([BLjava/lang/String;)V");
//    jbyteArray bytes = (env)->NewByteArray(strlen(pat));
//    (env)->SetByteArrayRegion(bytes, 0, strlen(pat), (jbyte *) pat);
//    jstring encoding = (env)->NewStringUTF("GB2312");
//    jstring result = (jstring) (env)->NewObject(strClass, ctorID, bytes, encoding);

    jstring result = (env)->NewStringUTF(pat);

//    env->DeleteLocalRef(strClass);

    return result;
}

void store_stack_info(uint64_t egl_resource, jmethodID methodId, char *java_stack,
                      uint64_t native_stack_hash) {
    JNIEnv *env = NULL;
    if (m_java_vm->GetEnv((void **) &env, JNI_VERSION_1_6) != JNI_OK) {
        if (m_java_vm->AttachCurrentThread(&env, NULL) == JNI_OK) {
        } else {
            LOGD("Cc1over-debug", "AttachCurrentThread != JNI_OK");
            return;
        }
    }

    if (env != NULL) {
        jstring js = charTojstring(env, java_stack);
        LOGD("Cc1over-debug", "start call back jni");
        env->CallStaticVoidMethod(m_class_EglHook,
                                  methodId,
                                  egl_resource, native_stack_hash, js);
        env->DeleteLocalRef(js);
        if (java_stack) {
            free(java_stack);
        }
        LOGD("Cc1over-debug", "end call back jni");
    }
}

void release_egl_resource(jmethodID methodId, uint64_t egl_resource) {
    JNIEnv *env = NULL;
    if (m_java_vm->GetEnv((void **) &env, JNI_VERSION_1_6) != JNI_OK) {
        if (m_java_vm->AttachCurrentThread(&env, NULL) == JNI_OK) {
        } else {
            LOGD("Cc1over-debug", "AttachCurrentThread != JNI_OK");
            return;
        }
    }

    if (env != NULL) {
        LOGD("Cc1over-debug", "start call back jni");
        env->CallStaticVoidMethod(m_class_EglHook,
                                  methodId,
                                  egl_resource);
        LOGD("Cc1over-debug", "end call back jni");
    }
}

uint64_t get_native_stack() {
//    auto ptr_stack_frames = new std::vector<unwindstack::FrameData>;
//    unwindstack::do_unwind(*ptr_stack_frames);
//    return hash_stack_frames(*ptr_stack_frames);
    return 0;
}

char *get_java_stack() {
    char *buf = static_cast<char *>(malloc(BUF_SIZE));
    if (buf) {
        get_java_stacktrace(buf, BUF_SIZE);
    }
    return buf;
}

DEFINE_HOOK_FUN(EGLContext, eglCreateContext, EGLDisplay dpy, EGLConfig config,
                EGLContext share_context, const EGLint *attrib_list) {

    CALL_ORIGIN_FUNC_RET(EGLContext, ret, eglCreateContext, dpy, config, share_context,
                         attrib_list);

    LOGD("Cc1over-debug", "call into eglCreateContext");

    store_stack_info((uint64_t) ret, m_method_egl_create_context, get_java_stack(),
                     get_native_stack());

    return ret;
}

DEFINE_HOOK_FUN(EGLBoolean, eglDestroyContext, EGLDisplay dpy, EGLContext ctx) {

    release_egl_resource(m_method_egl_destroy_context, (uint64_t) ctx);

    LOGD("Cc1over-debug", "call into eglDestroyContext");

    CALL_ORIGIN_FUNC_RET(EGLBoolean, ret, eglDestroyContext, dpy, ctx);

    return ret;
}


DEFINE_HOOK_FUN(EGLSurface, eglCreatePbufferSurface, EGLDisplay dpy, EGLContext ctx,
                const EGLint *attrib_list, int offset) {

    CALL_ORIGIN_FUNC_RET(EGLContext, ret, eglCreatePbufferSurface, dpy, ctx, attrib_list,
                         offset);

    LOGD("Cc1over-debug", "call into eglCreatePbufferSurface");

    store_stack_info((uint64_t) ret, m_method_egl_create_pbuffer_surface, get_java_stack(),
                     get_native_stack());

    return ret;

}

typedef EGLBoolean (*EGL_DESTORY_SURFACE)(EGLDisplay, EGLSurface);

DEFINE_HOOK_FUN(EGLBoolean, eglDestorySurface, EGLDisplay dpy, EGLSurface surface) {

    release_egl_resource(m_method_egl_destroy_surface, (uint64_t) surface);

    LOGD("Cc1over-debug", "call into eglDestorySurface");

    void *handle = dlopen(ORIGINAL_LIB, RTLD_LAZY);

    EGL_DESTORY_SURFACE eglDestorySurface = (EGL_DESTORY_SURFACE) (dlsym(handle,
                                                                         "eglDestroySurface"));

    return eglDestorySurface(dpy, surface);

}

DEFINE_HOOK_FUN(EGLSurface, eglCreateWindowSurface, EGLDisplay dpy, EGLConfig config,
                EGLNativeWindowType window, const EGLint *attrib_list) {

    CALL_ORIGIN_FUNC_RET(EGLContext, ret, eglCreateWindowSurface, dpy, config, window,
                         attrib_list);

    LOGD("Cc1over-debug", "call into eglCreateWindowSurface");

    store_stack_info((uint64_t) ret, m_method_egl_create_window_surface, get_java_stack(),
                     get_native_stack());

    return ret;
}

