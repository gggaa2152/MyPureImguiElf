#include <unistd.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <android_native_app_glue.h>
#include <android/log.h>
#include <GLES3/gl3.h>

#include "imgui.h"
#include "backends/imgui_impl_android.h"
#include "backends/imgui_impl_opengl3.h"

#define LOG_TAG "PureElf"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

static EGLDisplay  g_EglDisplay     = EGL_NO_DISPLAY;
static EGLSurface  g_EglSurface     = EGL_NO_SURFACE;
static EGLContext  g_EglContext     = EGL_NO_CONTEXT;

static bool InitEGL(ANativeWindow* window) {
    // 最原始的日志，确保能看到
    __android_log_print(ANDROID_LOG_INFO, "PureElf", "InitEGL: ENTERED FUNCTION");
    __android_log_print(ANDROID_LOG_INFO, "PureElf", "InitEGL: window = %p", window);

    if (window == nullptr) {
        __android_log_print(ANDROID_LOG_ERROR, "PureElf", "InitEGL: window is NULL");
        return false;
    }

    int w = ANativeWindow_getWidth(window);
    int h = ANativeWindow_getHeight(window);
    __android_log_print(ANDROID_LOG_INFO, "PureElf", "InitEGL: window size = %dx%d", w, h);

    // 获取显示
    __android_log_print(ANDROID_LOG_INFO, "PureElf", "InitEGL: calling eglGetDisplay");
    g_EglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    __android_log_print(ANDROID_LOG_INFO, "PureElf", "InitEGL: eglGetDisplay returned %p", g_EglDisplay);

    if (g_EglDisplay == EGL_NO_DISPLAY) {
        __android_log_print(ANDROID_LOG_ERROR, "PureElf", "InitEGL: eglGetDisplay failed");
        return false;
    }

    // 初始化显示
    __android_log_print(ANDROID_LOG_INFO, "PureElf", "InitEGL: calling eglInitialize");
    if (!eglInitialize(g_EglDisplay, nullptr, nullptr)) {
        __android_log_print(ANDROID_LOG_ERROR, "PureElf", "InitEGL: eglInitialize failed");
        return false;
    }
    __android_log_print(ANDROID_LOG_INFO, "PureElf", "InitEGL: eglInitialize succeeded");

    // 配置属性（使用兼容性更好的2.0）
    const EGLint configAttribs[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_BLUE_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_RED_SIZE, 8,
        EGL_NONE
    };

    EGLConfig config;
    EGLint numConfigs;
    __android_log_print(ANDROID_LOG_INFO, "PureElf", "InitEGL: calling eglChooseConfig");
    if (!eglChooseConfig(g_EglDisplay, configAttribs, &config, 1, &numConfigs) || numConfigs == 0) {
        __android_log_print(ANDROID_LOG_ERROR, "PureElf", "InitEGL: eglChooseConfig failed");
        return false;
    }
    __android_log_print(ANDROID_LOG_INFO, "PureElf", "InitEGL: eglChooseConfig succeeded");

    // 创建表面
    __android_log_print(ANDROID_LOG_INFO, "PureElf", "InitEGL: calling eglCreateWindowSurface");
    g_EglSurface = eglCreateWindowSurface(g_EglDisplay, config, window, nullptr);
    __android_log_print(ANDROID_LOG_INFO, "PureElf", "InitEGL: eglCreateWindowSurface returned %p", g_EglSurface);

    if (g_EglSurface == EGL_NO_SURFACE) {
        __android_log_print(ANDROID_LOG_ERROR, "PureElf", "InitEGL: eglCreateWindowSurface failed");
        return false;
    }

    // 创建上下文（使用2.0）
    const EGLint contextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
    __android_log_print(ANDROID_LOG_INFO, "PureElf", "InitEGL: calling eglCreateContext");
    g_EglContext = eglCreateContext(g_EglDisplay, config, EGL_NO_CONTEXT, contextAttribs);
    __android_log_print(ANDROID_LOG_INFO, "PureElf", "InitEGL: eglCreateContext returned %p", g_EglContext);

    if (g_EglContext == EGL_NO_CONTEXT) {
        __android_log_print(ANDROID_LOG_ERROR, "PureElf", "InitEGL: eglCreateContext failed");
        return false;
    }

    // 激活上下文
    __android_log_print(ANDROID_LOG_INFO, "PureElf", "InitEGL: calling eglMakeCurrent");
    if (!eglMakeCurrent(g_EglDisplay, g_EglSurface, g_EglSurface, g_EglContext)) {
        __android_log_print(ANDROID_LOG_ERROR, "PureElf", "InitEGL: eglMakeCurrent failed");
        return false;
    }
    __android_log_print(ANDROID_LOG_INFO, "PureElf", "InitEGL: success!");

    return true;
}

void android_main(struct android_app* app) {
    LOGI("android_main started");

    LOGI("Waiting for window...");
    while (app->window == nullptr) {
        int events;
        struct android_poll_source* source;
        ALooper_pollAll(-1, nullptr, &events, (void**)&source);
        if (source) source->process(app, source);
    }
    LOGI("Window obtained, window=%p", app->window);

    // 调用 InitEGL 之前加日志
    LOGI("Calling InitEGL with window=%p", app->window);

    if (!InitEGL(app->window)) {
        LOGE("EGL Init Failed!");
        return;
    }

    // ... 后续代码不变
    // 为了简洁，省略了 ImGui 初始化和主循环，你可以保留原来的
}
