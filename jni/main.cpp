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

static EGLDisplay  g_EglDisplay     = EGL_NO_DISPLAY;
static EGLSurface  g_EglSurface     = EGL_NO_SURFACE;
static EGLContext  g_EglContext     = EGL_NO_CONTEXT;

static bool InitEGL(ANativeWindow* window) {
    LOGI("InitEGL: ENTERED FUNCTION");
    
    if (window == nullptr) {
        LOGE("InitEGL: window is NULL");
        return false;
    }

    LOGI("InitEGL: calling eglGetDisplay");
    g_EglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (g_EglDisplay == EGL_NO_DISPLAY) {
        LOGE("InitEGL: eglGetDisplay failed");
        return false;
    }

    LOGI("InitEGL: calling eglInitialize");
    if (!eglInitialize(g_EglDisplay, nullptr, nullptr)) {
        LOGE("InitEGL: eglInitialize failed");
        return false;
    }

    const EGLint configAttribs[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_BLUE_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_RED_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 24,
        EGL_STENCIL_SIZE, 8,
        EGL_NONE
    };

    EGLConfig config;
    EGLint numConfigs;
    if (!eglChooseConfig(g_EglDisplay, configAttribs, &config, 1, &numConfigs) || numConfigs == 0) {
        LOGE("InitEGL: eglChooseConfig failed");
        return false;
    }

    // 使用窗口表面（而不是 pbuffer）
    LOGI("InitEGL: creating window surface");
    g_EglSurface = eglCreateWindowSurface(g_EglDisplay, config, window, nullptr);
    if (g_EglSurface == EGL_NO_SURFACE) {
        LOGE("InitEGL: eglCreateWindowSurface failed");
        return false;
    }
    LOGI("InitEGL: window surface created successfully");

    const EGLint contextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
    g_EglContext = eglCreateContext(g_EglDisplay, config, EGL_NO_CONTEXT, contextAttribs);
    if (g_EglContext == EGL_NO_CONTEXT) {
        LOGE("InitEGL: eglCreateContext failed");
        return false;
    }

    if (!eglMakeCurrent(g_EglDisplay, g_EglSurface, g_EglSurface, g_EglContext)) {
        LOGE("InitEGL: eglMakeCurrent failed");
        return false;
    }

    LOGI("InitEGL: success!");
    return true;
}

void android_main(struct android_app* app) {
    LOGI("android_main started");
    
    while (app->window == nullptr) {
        int events;
        struct android_poll_source* source;
        ALooper_pollAll(-1, nullptr, &events, (void**)&source);
        if (source) source->process(app, source);
    }
    LOGI("Window obtained");
    
    if (!InitEGL(app->window)) {
        LOGE("EGL Init Failed!");
        return;
    }
    
    LOGI("Initializing ImGui...");
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;

    io.Fonts->AddFontDefault();
    io.Fonts->Build();

    LOGI("Initializing ImGui backends...");
    ImGui_ImplAndroid_Init(app->window);
    ImGui_ImplOpenGL3_Init("#version 300 es");

    LOGI("Entering main loop...");
    
    bool running = true;
    int frame_count = 0;
    
    while (running) {
        frame_count++;
        if (frame_count % 100 == 0) {
            LOGI("Main loop iteration %d", frame_count);
        }
        
        int events;
        struct android_poll_source* source;
        while (ALooper_pollAll(0, nullptr, &events, (void**)&source) >= 0) {
            if (source) source->process(app, source);
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplAndroid_NewFrame();
        ImGui::NewFrame();

        {
            ImGui::Begin("纯净 ELF 菜单", nullptr);
            ImGui::Text("帧数: %d", frame_count);
            ImGui::Text("窗口表面版");
            ImGui::End();
        }

        ImGui::Render();
        
        // 获取窗口尺寸
        int w = ANativeWindow_getWidth(app->window);
        int h = ANativeWindow_getHeight(app->window);
        glViewport(0, 0, w, h);
        
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        eglSwapBuffers(g_EglDisplay, g_EglSurface);
    }

    LOGI("Shutting down...");
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplAndroid_Shutdown();
    ImGui::DestroyContext();

    eglDestroyContext(g_EglDisplay, g_EglContext);
    eglDestroySurface(g_EglDisplay, g_EglSurface);
    eglTerminate(g_EglDisplay);
    LOGI("Done");
}
