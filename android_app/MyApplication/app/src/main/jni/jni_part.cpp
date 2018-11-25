#include <jni.h>
#include <vector>

#include <android/log.h>
#define  LOG_TAG    "clvk-test"
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

using namespace std;

extern "C" {

JNIEXPORT void JNICALL Java_io_github_astarasikov_clvkandroid_clvktests_MainActivity_TestCLVK(
		JNIEnv*,
		jobject
);

JNIEXPORT void JNICALL Java_io_github_astarasikov_clvkandroid_clvktests_MainActivity_TestCLVK(
		JNIEnv*,
		jobject
)
{
	LOGE("CLVK: %s", __func__);

    LOGE("CLVK CLINFO START: %s", __func__);
    extern int clinfo_main(int argc, char *argv[]);
    char *argv_clinfo[] = {
            "clinfo"
    };
    clinfo_main(1, argv_clinfo);
    LOGE("CLVK CLINFO DONE: %s", __func__);

    LOGE("CLVK Intel BitonicSort START: %s", __func__);
    extern int intel_BitonicSort_main (int argc, const char** argv);
    const char *argv_intel_BitonicSort[] = {
            "BitonicSort"
    };
    int intel_rc = intel_BitonicSort_main(1, argv_intel_BitonicSort);
    LOGE("YOLO Intel BitonicSort DONE: %s retcode=%d", __func__, intel_rc);
}

} //extern "C"
