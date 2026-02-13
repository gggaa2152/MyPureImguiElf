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
    LOGI("InitEGL: getting display...");
    g_EglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (g_EglDisplay == EGL_NO_DISPLAY) {
        LOGE("eglGetDisplay failed, error=0x%x", eglGetError());
        return false;
    }

    LOGI("InitEGL: initializing display...");
    if (!eglInitialize(g_EglDisplay, nullptr, nullptr)) {
        LOGE("eglInitialize failed, error=0x%x", eglGetError());
        return false;
    }

    LOGI("InitEGL: choosing config...");
    const EGLint configAttribs[] = {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT_KHR,
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_BLUE_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_RED_SIZE, 8, EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 24,
        EGL_STENCIL_SIZE, 8,
        EGL_NONE
    };
    EGLConfig config;
    EGLint numConfigs;
    if (!eglChooseConfig(g_EglDisplay, configAttribs, &config, 1, &numConfigs) || numConfigs == 0) {
        LOGE("eglChooseConfig failed, error=0x%x", eglGetError());
        return false;
    }

    // ---------- 修复：确保窗口仍然有效 ----------
    LOGI("InitEGL: checking window validity...");
    if (window == nullptr) {
        LOGE("window is null!");
        return false;
    }

    // 尝试获取窗口宽度来验证有效性
    int32_t width = ANativeWindow_getWidth(window);
    if (width <= 0) {
        LOGE("window invalid (width=%d)", width);
        return false;
    }
    LOGI("InitEGL: window valid, width=%d", width);

    // 等待窗口稳定
    LOGI("InitEGL: waiting 50ms for window to stabilize...");
    usleep(50000);

    LOGI("InitEGL: creating surface with window=%p", window);
    g_EglSurface = eglCreateWindowSurface(g_EglDisplay, config, window, nullptr);
    LOGI("InitEGL: eglCreateWindowSurface returned %p", g_EglSurface);

    if (g_EglSurface == EGL_NO_SURFACE) {
        LOGE("eglCreateWindowSurface failed, error=0x%x", eglGetError());
        return false;
    }
    LOGI("InitEGL: surface created successfully");

    LOGI("InitEGL: creating context...");
    const EGLint contextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE };
    g_EglContext = eglCreateContext(g_EglDisplay, config, EGL_NO_CONTEXT, contextAttribs);
    if (g_EglContext == EGL_NO_CONTEXT) {
        LOGE("eglCreateContext failed, error=0x%x", eglGetError());
        return false;
    }
    LOGI("InitEGL: context created successfully");

    LOGI("InitEGL: making current...");
    if (!eglMakeCurrent(g_EglDisplay, g_EglSurface, g_EglSurface, g_EglContext)) {
        LOGE("eglMakeCurrent failed, error=0x%x", eglGetError());
        return false;
    }
    LOGI("InitEGL: success!");

    return true;
}

void android_main(struct android_app* app) {
    LOGI("android_main started");

    // 处理应用命令的回调
    // 可以添加一个简单的命令处理函数来处理窗口事件
    // 但为了简化，我们直接等待窗口可用

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
