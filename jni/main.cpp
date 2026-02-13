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
    // 立即输出日志
    LOGI("InitEGL: entered function");
    LOGI("InitEGL: window=%p", window);

    // 检查窗口
    if (window == nullptr) {
        LOGE("InitEGL: window is null");
        return false;
    }

    // 获取窗口尺寸测试有效性
    int32_t w = ANativeWindow_getWidth(window);
    int32_t h = ANativeWindow_getHeight(window);
    LOGI("InitEGL: window size=%dx%d", w, h);

    // 尝试获取显示
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
    LOGI("InitEGL: calling eglChooseConfig");
    if (!eglChooseConfig(g_EglDisplay, configAttribs, &config, 1, &numConfigs) || numConfigs == 0) {
        LOGE("InitEGL: eglChooseConfig failed");
        return false;
    }
    LOGI("InitEGL: eglChooseConfig succeeded");

    // 创建表面
    LOGI("InitEGL: calling eglCreateWindowSurface");
    g_EglSurface = eglCreateWindowSurface(g_EglDisplay, config, window, nullptr);
    LOGI("InitEGL: eglCreateWindowSurface returned %p", g_EglSurface);

    if (g_EglSurface == EGL_NO_SURFACE) {
        LOGE("InitEGL: eglCreateWindowSurface failed");
        return false;
    }

    // 创建上下文（使用2.0）
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
    LOGI("android_main started");

    // 等待窗口
    LOGI("Waiting for window...");
    while (app->window == nullptr) {
        int events;
        struct android_poll_source* source;
        ALooper_pollAll(-1, nullptr, &events, (void**)&source);
        if (source) source->process(app, source);
    }
    LOGI("Window obtained, window=%p", app->window);

    // 获取窗口尺寸
    int32_t winWidth = ANativeWindow_getWidth(app->window);
    int32_t winHeight = ANativeWindow_getHeight(app->window);
    LOGI("Window size: %dx%d", winWidth, winHeight);

    // 初始化EGL
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
    if (!ImGui_ImplAndroid_Init(app->window)) {
        LOGE("ImGui_ImplAndroid_Init failed");
        return;
    }
    if (!ImGui_ImplOpenGL3_Init("#version 300 es")) {
        LOGE("ImGui_ImplOpenGL3_Init failed");
        return;
    }
    LOGI("ImGui backends initialized");

    LOGI("Entering main loop...");
    bool running = true;
    while (running) {
        // 处理所有待处理的事件
        int events;
        struct android_poll_source* source;
        while (ALooper_pollAll(0, nullptr, &events, (void**)&source) >= 0) {
            if (source) source->process(app, source);
        }

        // 检查窗口是否仍然有效
        if (app->window == nullptr) {
            LOGI("Window lost, exiting...");
            break;
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplAndroid_NewFrame();
        ImGui::NewFrame();

        {
            ImGui::Begin("✨ 纯净 ELF 菜单", nullptr,
                        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);
            ImGui::Text("这是从零构建的官方 ImGui ELF");
            ImGui::Separator();

            static bool bGodMode = false;
            static bool bAimbot  = false;

            if (ImGui::Button("💀 秒杀", ImVec2(120, 40))) {
                LOGI("秒杀按钮触发");
            }
            ImGui::SameLine();
            ImGui::Checkbox("🛡 无敌", &bGodMode);

            ImGui::Checkbox("🎯 自瞄", &bAimbot);
            if (bAimbot) {
                ImGui::Indent(20);
                static float fSmooth = 1.2f;
                ImGui::SliderFloat("平滑度", &fSmooth, 0.5f, 3.0f, "%.1f");
                ImGui::Unindent(20);
            }

            float fps = ImGui::GetIO().Framerate;
            ImGui::Text("FPS: %.1f", fps);
            ImGui::ProgressBar(fps / 120.0f, ImVec2(200, 0), "");

            ImGui::End();
        }

        ImGui::Render();
        glViewport(0, 0, winWidth, winHeight);
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
