#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <android/log.h>
#include <unistd.h>

// 把这个头文件放到你的 jni/include/ 目录下
#include "ANativeWindowCreator.h"
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

    // 1. 获取 EGL Display
    g_EglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (g_EglDisplay == EGL_NO_DISPLAY) {
        LOGE("eglGetDisplay failed");
        return false;
    }

    // 2. 初始化 EGL
    if (!eglInitialize(g_EglDisplay, nullptr, nullptr)) {
        LOGE("eglInitialize failed");
        return false;
    }

    // 3. 选择配置
    const EGLint attribs[] = {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
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

    // 4. 创建窗口表面（现在用的是自己创建的 ANativeWindow）
    LOGI("Creating EGL window surface with native window: %p", g_NativeWindow);
    g_EglSurface = eglCreateWindowSurface(g_EglDisplay, config, g_NativeWindow, nullptr);
    if (g_EglSurface == EGL_NO_SURFACE) {
        LOGE("eglCreateWindowSurface failed");
        return false;
    }

    // 5. 创建上下文
    const EGLint contextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
    g_EglContext = eglCreateContext(g_EglDisplay, config, EGL_NO_CONTEXT, contextAttribs);
    if (g_EglContext == EGL_NO_CONTEXT) {
        LOGE("eglCreateContext failed");
        return false;
    }

    // 6. 激活上下文
    if (!eglMakeCurrent(g_EglDisplay, g_EglSurface, g_EglSurface, g_EglContext)) {
        LOGE("eglMakeCurrent failed");
        return false;
    }

    LOGI("InitEGL: success!");
    return true;
}

void android_main(struct android_app* app) {
    LOGI("android_main started");

    // 1. 直接用 ANativeWindowCreator 创建窗口（不依赖 app->window）
    g_NativeWindow = android::ANativeWindowCreator::Create(
        "ImGuiWindow",  // 窗口名
        1080,           // 宽（传-1会自动获取屏幕尺寸）
        1920,           // 高
        false           // 是否防录屏
    );

    if (!g_NativeWindow) {
        LOGE("Failed to create native window");
        return;
    }
    LOGI("Native window created: %p", g_NativeWindow);

    // 2. 初始化 EGL
    if (!InitEGL()) {
        LOGE("EGL initialization failed");
        android::ANativeWindowCreator::Destroy(g_NativeWindow);
        return;
    }

    // 3. 初始化 ImGui
    LOGI("Initializing ImGui...");
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;

    io.Fonts->AddFontDefault();
    io.Fonts->Build();

    // 4. 初始化 ImGui 后端
    LOGI("Initializing ImGui backends...");
    ImGui_ImplAndroid_Init(g_NativeWindow);
    ImGui_ImplOpenGL3_Init("#version 300 es");

    // 5. 主循环
    LOGI("Entering main loop...");
    bool running = true;
    int frame_count = 0;

    while (running) {
        frame_count++;
        if (frame_count % 60 == 0) {
            LOGI("Main loop iteration %d, FPS: %.1f", frame_count, ImGui::GetIO().Framerate);
        }

        // 处理事件（注意：这里不需要用 app->window）
        int events;
        struct android_poll_source* source;
        while (ALooper_pollAll(0, nullptr, &events, (void**)&source) >= 0) {
            if (source) source->process(app, source);
        }

        // ImGui 新帧
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplAndroid_NewFrame();
        ImGui::NewFrame();

        // 显示官方 Demo 窗口
        ImGui::ShowDemoWindow();

        // 渲染
        ImGui::Render();
        glViewport(0, 0, 1080, 1920);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        eglSwapBuffers(g_EglDisplay, g_EglSurface);
    }

    // 6. 清理
    LOGI("Shutting down...");
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplAndroid_Shutdown();
    ImGui::DestroyContext();

    eglDestroyContext(g_EglDisplay, g_EglContext);
    eglDestroySurface(g_EglDisplay, g_EglSurface);
    eglTerminate(g_EglDisplay);

    android::ANativeWindowCreator::Destroy(g_NativeWindow);
    LOGI("Done");
}
