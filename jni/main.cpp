#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <android/log.h>
#include <android_native_app_glue.h>
#include <android/input.h>
#include <unistd.h>

#include "imgui.h"
#include "backends/imgui_impl_android.h"
#include "backends/imgui_impl_opengl3.h"

#define LOG_TAG "PureElf"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

static EGLDisplay  g_EglDisplay     = EGL_NO_DISPLAY;
static EGLSurface  g_EglSurface     = EGL_NO_SURFACE;
static EGLContext  g_EglContext     = EGL_NO_CONTEXT;
static ANativeWindow* g_NativeWindow = nullptr;

static bool InitEGL() {
    LOGI("InitEGL: starting...");

    g_EglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (g_EglDisplay == EGL_NO_DISPLAY) {
        LOGE("eglGetDisplay failed");
        return false;
    }

    if (!eglInitialize(g_EglDisplay, nullptr, nullptr)) {
        LOGE("eglInitialize failed");
        return false;
    }

    const EGLint attribs[] = {
        EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
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
    if (!eglChooseConfig(g_EglDisplay, attribs, &config, 1, &numConfigs) || numConfigs == 0) {
        LOGE("eglChooseConfig failed");
        return false;
    }

    // 用 pbuffer 代替窗口表面
    const EGLint pbufferAttribs[] = {
        EGL_WIDTH, 1080,
        EGL_HEIGHT, 1920,
        EGL_NONE
    };
    g_EglSurface = eglCreatePbufferSurface(g_EglDisplay, config, pbufferAttribs);
    if (g_EglSurface == EGL_NO_SURFACE) {
        LOGE("eglCreatePbufferSurface failed");
        return false;
    }

    const EGLint contextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
    g_EglContext = eglCreateContext(g_EglDisplay, config, EGL_NO_CONTEXT, contextAttribs);
    if (g_EglContext == EGL_NO_CONTEXT) {
        LOGE("eglCreateContext failed");
        return false;
    }

    if (!eglMakeCurrent(g_EglDisplay, g_EglSurface, g_EglSurface, g_EglContext)) {
        LOGE("eglMakeCurrent failed");
        return false;
    }

    LOGI("InitEGL: success!");
    return true;
}

static int32_t handle_input_event(struct android_app* app, AInputEvent* event) {
    if (ImGui_ImplAndroid_HandleInputEvent(event)) {
        return 1;
    }
    return 0;
}

void android_main(struct android_app* app) {
    LOGI("android_main started");
    LOGI("Setting input event callback");
    app->onInputEvent = handle_input_event;

    LOGI("Waiting for window...");
    while (app->window == nullptr) {
        int events;
        struct android_poll_source* source;
        ALooper_pollAll(-1, nullptr, &events, (void**)&source);
        if (source) source->process(app, source);
    }
    LOGI("Window obtained: %p", app->window);

    LOGI("Initializing EGL with pbuffer");
    if (!InitEGL()) {
        LOGE("EGL initialization failed");
        return;
    }

    LOGI("Initializing ImGui...");
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;

    io.Fonts->AddFontDefault();

    LOGI("Initializing ImGui backends...");
    ImGui_ImplAndroid_Init(app->window);
    ImGui_ImplOpenGL3_Init("#version 300 es");

    LOGI("Entering main loop...");
    bool running = true;
    int frame_count = 0;

    while (running) {
        frame_count++;
        if (frame_count % 60 == 0) {
            LOGI("Main loop iteration %d, FPS: %.1f", frame_count, ImGui::GetIO().Framerate);
        }

        int events;
        struct android_poll_source* source;
        while (ALooper_pollAll(0, nullptr, &events, (void**)&source) >= 0) {
            if (source) {
                source->process(app, source);
            }
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplAndroid_NewFrame();
        ImGui::NewFrame();

        ImGui::ShowDemoWindow();

        ImGui::Render();
        glViewport(0, 0, 1080, 1920);
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
