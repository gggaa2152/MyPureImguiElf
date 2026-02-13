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
    LOGI("InitEGL: ENTERED FUNCTION");
    LOGI("InitEGL: window = %p", window);

    if (window == nullptr) {
        LOGE("InitEGL: window is NULL");
        return false;
    }

    // 直接使用默认尺寸
    int w = 1080;
    int h = 1920;
    LOGI("InitEGL: using default window size = %dx%d", w, h);

    // 获取显示
    LOGI("InitEGL: calling eglGetDisplay");
    g_EglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    LOGI("InitEGL: eglGetDisplay returned %p", g_EglDisplay);

    if (g_EglDisplay == EGL_NO_DISPLAY) {
        LOGE("InitEGL: eglGetDisplay failed");
        return false;
    }

    // 初始化显示
    LOGI("InitEGL: calling eglInitialize");
    if (!eglInitialize(g_EglDisplay, nullptr, nullptr)) {
        LOGE("InitEGL: eglInitialize failed");
        return false;
    }
    LOGI("InitEGL: eglInitialize succeeded");

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
    LOGI("InitEGL: calling eglChooseConfig");
    if (!eglChooseConfig(g_EglDisplay, configAttribs, &config, 1, &numConfigs) || numConfigs == 0) {
        LOGE("InitEGL: eglChooseConfig failed");
        return false;
    }
    LOGI("InitEGL: eglChooseConfig succeeded");

    // 创建表面（带详细错误信息）
    LOGI("InitEGL: calling eglCreateWindowSurface with window=%p", window);
    g_EglSurface = eglCreateWindowSurface(g_EglDisplay, config, window, nullptr);
    EGLint surfaceError = eglGetError();
    LOGI("InitEGL: eglCreateWindowSurface returned %p, error=0x%x", g_EglSurface, surfaceError);

    if (g_EglSurface == EGL_NO_SURFACE) {
        LOGE("InitEGL: eglCreateWindowSurface failed with error 0x%x", surfaceError);
        return false;
    }
    LOGI("InitEGL: surface created successfully");

    // 创建上下文
    const EGLint contextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
    LOGI("InitEGL: calling eglCreateContext");
    g_EglContext = eglCreateContext(g_EglDisplay, config, EGL_NO_CONTEXT, contextAttribs);
    LOGI("InitEGL: eglCreateContext returned %p", g_EglContext);

    if (g_EglContext == EGL_NO_CONTEXT) {
        LOGE("InitEGL: eglCreateContext failed");
        return false;
    }

    // 激活上下文
    LOGI("InitEGL: calling eglMakeCurrent");
    if (!eglMakeCurrent(g_EglDisplay, g_EglSurface, g_EglSurface, g_EglContext)) {
        LOGE("InitEGL: eglMakeCurrent failed");
        return false;
    }
    LOGI("InitEGL: success!");

    return true;
}

void android_main(struct android_app* app) {
    LOGI("DEBUG: android_main started");
    
    LOGI("DEBUG: waiting for window...");
    while (app->window == nullptr) {
        int events;
        struct android_poll_source* source;
        ALooper_pollAll(-1, nullptr, &events, (void**)&source);
        if (source) source->process(app, source);
    }
    LOGI("DEBUG: window obtained, window=%p", app->window);
    
    LOGI("DEBUG: about to call InitEGL");
    
    bool result = InitEGL(app->window);
    
    LOGI("DEBUG: InitEGL returned %d", result);
    
    if (!result) {
        LOGE("DEBUG: EGL Init Failed!");
        return;
    }
    
    LOGI("DEBUG: EGL Init succeeded, continuing...");
    
    // 测试完成，先退出
    LOGI("DEBUG: test complete, exiting");
    return;
}
