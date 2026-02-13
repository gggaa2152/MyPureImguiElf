LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := PureImGuiElf

LOCAL_CFLAGS   := -std=c11 -Wall
LOCAL_CPPFLAGS := -std=c++17 -Wall -fexceptions -frtti
LOCAL_CPPFLAGS += -DIMGUI_IMPL_OPENGL_ES3

# ===== 头文件路径 =====
LOCAL_C_INCLUDES := $(LOCAL_PATH)/imgui
LOCAL_C_INCLUDES += $(LOCAL_PATH)/backends
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include          # ← 新加的这一行！
LOCAL_C_INCLUDES += $(NDK_ROOT)/sources/android/native_app_glue

LOCAL_SRC_FILES := main.cpp
LOCAL_SRC_FILES += imgui/imgui.cpp
LOCAL_SRC_FILES += imgui/imgui_draw.cpp
LOCAL_SRC_FILES += imgui/imgui_tables.cpp
LOCAL_SRC_FILES += imgui/imgui_widgets.cpp
LOCAL_SRC_FILES += imgui/imgui_demo.cpp
LOCAL_SRC_FILES += backends/imgui_impl_android.cpp
LOCAL_SRC_FILES += backends/imgui_impl_opengl3.cpp
LOCAL_SRC_FILES += $(NDK_ROOT)/sources/android/native_app_glue/android_native_app_glue.c

LOCAL_LDLIBS := -landroid -lEGL -lGLESv3 -llog

# 告诉链接器用 android_main 而不是 main
LOCAL_LDFLAGS += -u ANativeActivity_onCreate

include $(BUILD_EXECUTABLE)
