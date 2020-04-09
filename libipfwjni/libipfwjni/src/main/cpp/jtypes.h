//
// Created by pbozhko on 1/21/20.
//

#pragma once

#include <string>
#include <vector>
#include <jni.h>
#include <assert.h>

struct IpFwClass {
    jclass classObject;
    jfieldID pointerFieldID;
};

template<class T = char>
static inline
std::vector<T> toVector(JNIEnv *env, jbyteArray array) {
    if(array == NULL) {
        return std::vector<T>();
    }

    jsize length = env->GetArrayLength(array);
    jbyte *elements = env->GetByteArrayElements(array, NULL);
    std::vector<T> result((T *)elements, (T *)(elements + length));
    env->ReleaseByteArrayElements(array, elements, 0);
    return result;
}


static inline
std::string toString(JNIEnv *env, jstring value) {
    if(value == NULL) {
        return std::string();
    }
    const char *data = env->GetStringUTFChars(value, NULL);
    std::string result(data);
    env->ReleaseStringUTFChars(value, data);
    return result;
}

static inline
std::string getStringField(JNIEnv *env, jobject obj, const char *name) {
    jclass clazz = env->GetObjectClass(obj);
    jfieldID strFieldId = env->GetFieldID(clazz, name, "Ljava/lang/String;");
    assert(strFieldId != NULL);
    jstring jstr = (jstring) env->GetObjectField(obj, strFieldId);
    return toString(env, jstr);
}

template<class T = char>
static inline
std::vector<T> getByteArrayField(JNIEnv *env, jobject obj, const char *name) {
    jclass clazz = env->GetObjectClass(obj);
    jfieldID strFieldId = env->GetFieldID(clazz, name, "[B");
    assert(strFieldId != NULL);
    jbyteArray jarr = (jbyteArray) env->GetObjectField(obj, strFieldId);
    return toVector<T>(env, jarr);

}

static inline
int getIntField(JNIEnv *env, jobject obj, const char *name) {
    jclass clazz = env->GetObjectClass(obj);
    jfieldID intFieldId = env->GetFieldID(clazz, name, "I");
    assert(intFieldId != NULL);
    return  env->GetIntField(obj, intFieldId);
}

static inline
int getBooleanField(JNIEnv *env, jobject obj, const char *name) {
    jclass clazz = env->GetObjectClass(obj);
    jfieldID intFieldId = env->GetFieldID(clazz, name, "Z");
    assert(intFieldId != NULL);
    return  env->GetBooleanField(obj, intFieldId);
}

static inline
void throwRuntimeException(JNIEnv *env, const char *msg) {
    jclass c = env->FindClass("java/lang/RuntimeException");
    assert(c!=NULL);
    env->ThrowNew(c, msg);
}

extern  IpFwClass ipFwClass;