LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := PureImGuiElf

LOCAL_CFLAGS   := -std=c11 -Wall
LOCAL_CPPFLAGS := -std=c++17 -Wall -fexceptions -frtti
LOCAL_CPPFLAGS += -DIMGUI_IMPL_OPENGL_ES3

LOCAL_C_INCLUDES := $(LOCAL_PATH)/imgui
LOCAL_C_INCLUDES += $(LOCAL_PATH)/backends
LOCAL_C_INCLUDES += $(NDK_ROOT)/sources/android/native_app_glue

LOCAL_SRC_FILES := main.cpp
LOCAL_SRC_FILES += imgui/imgui.cpp
LOCAL_SRC_FILES += imgui/imgui_draw.cpp
LOCAL_SRC_FILES += imgui/imgui_tables.cpp
LOCAL_SRC_FILES += imgui/imgui_widgets.cpp
LOCAL_SRC_FILES += imgui/imgui_demo.cpp
LOCAL_SRC_FILES += backends/imgui_impl_android.cpp
LOCAL_SRC_FILES += backends/imgui_impl_opengl3.cpp

# ⚠️ 直接把 native_app_glue.c 当作源文件编译
LOCAL_SRC_FILES += $(NDK_ROOT)/sources/android/native_app_glue/android_native_app_glue.c

LOCAL_LDLIBS := -landroid -lEGL -lGLESv3 -llog

# ❌ 不再需要 LOCAL_STATIC_LIBRARIES
# ✅ 也不需要 $(call import-module, ...)

include $(BUILD_EXECUTABLE)
