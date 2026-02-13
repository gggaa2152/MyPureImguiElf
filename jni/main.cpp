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

    int w = 1080;
    int h = 1920;
    LOGI("InitEGL: using default window size = %dx%d", w, h);

    LOGI("InitEGL: calling eglGetDisplay");
    g_EglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    LOGI("InitEGL: eglGetDisplay returned %p", g_EglDisplay);

    if (g_EglDisplay == EGL_NO_DISPLAY) {
        LOGE("InitEGL: eglGetDisplay failed");
        return false;
    }

    LOGI("InitEGL: calling eglInitialize");
    if (!eglInitialize(g_EglDisplay, nullptr, nullptr)) {
        LOGE("InitEGL: eglInitialize failed");
        return false;
    }
    LOGI("InitEGL: eglInitialize succeeded");

    const EGLint configAttribs[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT | EGL_PBUFFER_BIT,
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
    LOGI("InitEGL: calling eglChooseConfig");
    if (!eglChooseConfig(g_EglDisplay, configAttribs, &config, 1, &numConfigs) || numConfigs == 0) {
        LOGE("InitEGL: eglChooseConfig failed");
        return false;
    }
    LOGI("InitEGL: eglChooseConfig succeeded");

    LOGI("InitEGL: using pbuffer surface");
    const EGLint pbufferAttribs[] = {
        EGL_WIDTH, w,
        EGL_HEIGHT, h,
        EGL_NONE
    };

    g_EglSurface = eglCreatePbufferSurface(g_EglDisplay, config, pbufferAttribs);
    EGLint surfaceError = eglGetError();
    LOGI("InitEGL: eglCreatePbufferSurface returned %p, error=0x%x", g_EglSurface, surfaceError);

    if (g_EglSurface == EGL_NO_SURFACE) {
        LOGE("InitEGL: eglCreatePbufferSurface failed with error 0x%x", surfaceError);
        return false;
    }
    LOGI("InitEGL: pbuffer surface created successfully");

    const EGLint contextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
    LOGI("InitEGL: calling eglCreateContext");
    g_EglContext = eglCreateContext(g_EglDisplay, config, EGL_NO_CONTEXT, contextAttribs);
    LOGI("InitEGL: eglCreateContext returned %p", g_EglContext);

    if (g_EglContext == EGL_NO_CONTEXT) {
        LOGE("InitEGL: eglCreateContext failed");
        return false;
    }

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
    
    if (!InitEGL(app->window)) {
        LOGE("DEBUG: EGL Init Failed!");
        return;
    }
    
    LOGI("DEBUG: EGL Init succeeded, initializing ImGui...");
    
    // 初始化 ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;

    io.Fonts->AddFontDefault();
    io.Fonts->Build();

    LOGI("DEBUG: Initializing ImGui backends...");
    ImGui_ImplAndroid_Init(app->window);
    ImGui_ImplOpenGL3_Init("#version 300 es");

    LOGI("DEBUG: Entering main loop...");
    
    bool running = true;
    while (running) {
        // 处理事件
        int events;
        struct android_poll_source* source;
        while (ALooper_pollAll(0, nullptr, &events, (void**)&source) >= 0) {
            if (source) source->process(app, source);
        }

        // 开始新帧
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplAndroid_NewFrame();
        ImGui::NewFrame();

        // 绘制菜单
        {
            ImGui::Begin("✨ 纯净 ELF 菜单", nullptr,
                        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);
            ImGui::Text("终于成功了！");
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

        // 渲染
        ImGui::Render();
        glViewport(0, 0, 1080, 1920);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        eglSwapBuffers(g_EglDisplay, g_EglSurface);
    }

    LOGI("DEBUG: Shutting down...");
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplAndroid_Shutdown();
    ImGui::DestroyContext();

    eglDestroyContext(g_EglDisplay, g_EglContext);
    eglDestroySurface(g_EglDisplay, g_EglSurface);
    eglTerminate(g_EglDisplay);
    LOGI("DEBUG: Done");
}
