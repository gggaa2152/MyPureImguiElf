#include <unistd.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <android/native_window.h>
#include <android_native_app_glue.h>
#include <android/log.h>
#include <GLES3/gl3.h>

#include "imgui.h"
#include "backends/imgui_impl_android.h"
#include "backends/imgui_impl_opengl3.h"

#define LOG_TAG "PureElf"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)

static EGLDisplay  g_EglDisplay     = EGL_NO_DISPLAY;
static EGLSurface  g_EglSurface     = EGL_NO_SURFACE;
static EGLContext  g_EglContext     = EGL_NO_CONTEXT;

static bool InitEGL(ANativeWindow* window) {
    __android_log_print(ANDROID_LOG_INFO, "PureElf", "InitEGL: ENTERED FUNCTION");
    __android_log_print(ANDROID_LOG_INFO, "PureElf", "InitEGL: window = %p", window);

    if (window == nullptr) {
        __android_log_print(ANDROID_LOG_ERROR, "PureElf", "InitEGL: window is NULL");
        return false;
    }

    // 直接使用默认尺寸，绕过ANativeWindow_getWidth/Height
    int w = 1080;
    int h = 1920;
    __android_log_print(ANDROID_LOG_INFO, "PureElf", "InitEGL: using default window size = %dx%d", w, h);

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

    // 配置属性
    const EGLint configAttribs[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_BLUE_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_RED_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
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

    // 创建上下文
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
    __android_log_print(ANDROID_LOG_INFO, "PureElf", "DEBUG: android_main started");
    
    __android_log_print(ANDROID_LOG_INFO, "PureElf", "DEBUG: waiting for window...");
    while (app->window == nullptr) {
        int events;
        struct android_poll_source* source;
        ALooper_pollAll(-1, nullptr, &events, (void**)&source);
        if (source) source->process(app, source);
    }
    __android_log_print(ANDROID_LOG_INFO, "PureElf", "DEBUG: window obtained, window=%p", app->window);
    
    __android_log_print(ANDROID_LOG_INFO, "PureElf", "DEBUG: about to call InitEGL");
    
    bool result = InitEGL(app->window);
    
    __android_log_print(ANDROID_LOG_INFO, "PureElf", "DEBUG: InitEGL returned %d", result);
    
    if (!result) {
        __android_log_print(ANDROID_LOG_ERROR, "PureElf", "DEBUG: EGL Init Failed!");
        return;
    }
    
    __android_log_print(ANDROID_LOG_INFO, "PureElf", "DEBUG: EGL Init succeeded, continuing...");
    
    // 测试完成，先退出
    __android_log_print(ANDROID_LOG_INFO, "PureElf", "DEBUG: test complete, exiting");
    return;
}
