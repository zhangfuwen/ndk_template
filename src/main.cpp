#include <android/log.h>
#include <android/trace.h>
#include <sys/prctl.h>

#define LOG_TAG "NdkBinderExample"
#define ALOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define ALOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

int main() {

   prctl(PR_SET_NAME,"my_ndk_example_thread",0,0,0);
   ATrace_beginSection("my_section");
    __android_log_buf_print(LOG_ID_EVENTS, ANDROID_LOG_INFO, LOG_TAG, "print start");
    ALOGE("NDK Binder Example");
    __android_log_buf_print(LOG_ID_EVENTS, ANDROID_LOG_INFO, LOG_TAG, "print end");
    ATrace_endSection();
    
    return 0;
}
