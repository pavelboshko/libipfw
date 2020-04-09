//
// Created by pbozhko on 1/21/20.
//

#include "ru_securitycode_libipfwjni_IpFw.h"
#include "jtypes.h"
#include "log.h"
#include "ipfw_tun.h"


ipfw_tun * getIpFw(JNIEnv *env, jobject obj, const IpFwClass & ipFwClass);

JNIEXPORT void JNICALL Java_ru_securitycode_libipfwjni_IpFw_createIpFw
(JNIEnv * env, jobject thiz, jobject param) {

	const std::string fwHttpsHost = getStringField(env, param, "forwardHttpsHost");
	const int fwHttpsPort = getIntField(env, param, "forwardHttpsPort");
	const std::string fwHttpHost = getStringField(env, param, "forwardHttpHost");
	const int fwHttpPort = getIntField(env, param, "forwardHttpsPort");
    const int tunFd = getIntField(env, param, "tunFd");


	ipfw_logger->debug("createIpFw {} {} {} {} {}", fwHttpsHost, fwHttpsPort,
					   fwHttpHost, fwHttpPort, tunFd);
    auto ipfw = new ipfw_tun(fwHttpsHost, fwHttpsPort, fwHttpHost, fwHttpPort, tunFd);

    env->SetLongField(thiz, ipFwClass.pointerFieldID, reinterpret_cast<jlong>(ipfw));
}


JNIEXPORT void JNICALL Java_ru_securitycode_libipfwjni_IpFw_startIpFw
(JNIEnv * env, jobject thiz) {
    auto ipfw = getIpFw(env, thiz, ipFwClass);
    if(!ipfw) {
        throwRuntimeException(env, "ipfw not exist");
        return;
    }
    ipfw->start();
}

JNIEXPORT void JNICALL Java_ru_securitycode_libipfwjni_IpFw_stopIpFw
(JNIEnv * env, jobject thiz) {
    auto ipfw = getIpFw(env, thiz, ipFwClass);
    if(!ipfw) {
        throwRuntimeException(env, "ipfw not exist");
        return;
    }
    ipfw->stop();
    delete ipfw;
    env->SetLongField(thiz, ipFwClass.pointerFieldID, reinterpret_cast<jlong>((jlong*)NULL));
}

ipfw_tun * getIpFw(JNIEnv *env, jobject obj, const IpFwClass & ipFwClass) {
    jlong pointer = env->GetLongField(obj, ipFwClass.pointerFieldID);
    if (!pointer) {
        return NULL;
    }
    return reinterpret_cast<ipfw_tun *>(pointer);
}
