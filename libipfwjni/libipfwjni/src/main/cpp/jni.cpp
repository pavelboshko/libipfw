//
// Created by pbozhko on 1/21/20.
//

#include <jni.h>
#include <android/log.h>
#include <assert.h>
#include "jtypes.h"
#include "log.h"

IpFwClass ipFwClass;
class Sink;
static const char * TAG = "libipfwjni_log";

static void init_log();
static void setupIpFwClass(JNIEnv *env, IpFwClass & ipFwClass);

jint JNI_OnLoad(JavaVM* vm, void* reserved) {


    JNIEnv *env = NULL;

    if (vm->GetEnv((void **) &env, JNI_VERSION_1_6) != JNI_OK) {
        __android_log_write(ANDROID_LOG_ERROR, TAG, "can`t get JNIEnv");
        return -1;
    }

    init_log();
    setupIpFwClass(env, ipFwClass);
    return JNI_VERSION_1_6;
}

JNIEXPORT void
JNICALL JNI_OnUnload(JavaVM *vm, void *reserved) {

}

static void init_log() {
    ipfw_logger = std::make_shared<spdlog::logger>(TAG, std::make_shared<Sink>());
//    ipfw_logger->set_level(spdlog::level::debug);
}

void setupIpFwClass(JNIEnv *env, IpFwClass & ipFwClass) {
    jclass clazz = env->FindClass("ru/securitycode/libipfwjni/IpFw");
    assert(clazz != NULL);
    ipFwClass.pointerFieldID = env->GetFieldID(clazz, "pointer", "J");
    assert(ipFwClass.pointerFieldID != NULL);
    ipFwClass.classObject = clazz;
}

class Sink : public spdlog::sinks::sink {
public:
    void log(const spdlog::details::log_msg &msg) override {
        __android_log_write(ANDROID_LOG_DEBUG, TAG, msg.formatted.str().c_str());
    }
    void flush() override { ; }
};
