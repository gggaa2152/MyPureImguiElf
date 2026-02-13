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

    // 可选：短暂等待让窗口稳定
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
