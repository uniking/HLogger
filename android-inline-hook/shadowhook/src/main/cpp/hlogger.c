#include <fcntl.h>
#include <dlfcn.h>
#include <unistd.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "hlogger.h"

//com.bytedance.shadowhook
#define JNI_LOG
//#define JNI_API_DEF(f) Java_com_bytedance_shadowhook_HLogger_##f

static char* TAG = "hlogger";

static void *autohook(void *sym_addr, void *new_addr, void **orig_addr){
    return shadowhook_hook_sym_addr(sym_addr, new_addr, orig_addr);
}
static void *autohook_sym_name(const char *lib_name, const char *sym_name, void *new_addr, void **orig_addr){
    return shadowhook_hook_sym_name(lib_name, sym_name, new_addr, orig_addr);
}
// 将 jstring 转换为 char*
// 将 jstring 转换为 char*
static char* jstringToChar(JNIEnv* env, jstring jstr) {
    char* result = NULL;
    const char* str = (*env)->GetStringUTFChars(env, jstr, NULL);
    if (str != NULL) {
        // 获取字符串的长度
        jsize length = (*env)->GetStringUTFLength(env, jstr);

        // 分配内存并复制字符串
        result = (char*)malloc(((unsigned long) (length + 1)) * sizeof(char)); // 需要额外的字节存储字符串的结束符 '\0'
        strncpy(result, str, (size_t) length);
        result[length] = '\0';

        // 释放字符串的内存
        (*env)->ReleaseStringUTFChars(env, jstr, str);
    }
    return result;
}

//old function
//static jint (*old_RegisterNatives)(jclass clazz, const JNINativeMethod* methods,
//                                   jint nMethods) = NULL;
static jclass (*old_FindClass)(JNIEnv*, const char*)  = NULL;
static jfieldID    (*old_GetFieldID)(JNIEnv*, jclass, const char*, const char*)  = NULL;
static jfieldID    (*old_GetStaticFieldID)(JNIEnv*, jclass, const char*,
                                           const char*) = NULL;
static jmethodID   (*old_GetMethodID)(JNIEnv*, jclass, const char*, const char*) = NULL;
static jmethodID   (*old_GetStaticMethodID)(JNIEnv*, jclass, const char*, const char*) = NULL;

static jstring     (*old_NewStringUTF)(JNIEnv*, const char*) = NULL;
static const char* (*old_GetStringUTFChars)(JNIEnv*, jstring, jboolean*) = NULL;

static jclass      (*old_GetObjectClass)(JNIEnv*, jobject) = NULL;
static jclass      (*old_GetSuperclass)(JNIEnv*, jclass) = NULL;

JNIEXPORT jstring JVM_NativeLoad(JNIEnv* env, jstring javaFilename,
                                   jobject javaLoader, jclass caller);
static JNIEXPORT jstring (*old_JVM_NativeLoad)(JNIEnv* env, jstring javaFilename,
                                 jobject javaLoader, jclass caller) = NULL;

//static int (*old_open)(const char *pathName, int flag, int mode) = NULL;
static int (*old_openat)(int dirFd, const char *pathName, int flag, int mode) = NULL;
static void* (*old_dlopen)(const char* __filename, int __flag) = NULL;
static int (*old_access)(const char* __path, int __mode) = NULL;
static int (*old_kill)(pid_t __pid, int __signal) = NULL;
static FILE* (*old_popen)(const char* __command, const char* __mode) = NULL;

//new function
//static jint new_RegisterNatives(jclass clazz, const JNINativeMethod* methods,
//                                   jint nMethods){
//#ifdef JNI_LOG
//    for(int i=0; i<nMethods; ++i){
//        __android_log_print(ANDROID_LOG_DEBUG, "hlogger", "RegisterNatives fnPtr=%p\n name=%s signature=%s", methods[i].fnPtr, methods[i].name, methods[i].signature);
//    }
//
//#endif
//    return old_RegisterNatives(clazz, methods, nMethods);
//}
static jclass new_FindClass(JNIEnv *env, const char *name)
{
    jclass jc;
    jc = old_FindClass(env, name);
#ifdef JNI_LOG
    __android_log_print(ANDROID_LOG_DEBUG, TAG, "FindClass %s , return clazz=%p\n", name, jc);
#endif
    return jc;
}
static jfieldID    new_GetFieldID(JNIEnv* env, jclass clazz, const char* name, const char* sign)
{
    jfieldID jf;
    jf = old_GetFieldID(env, clazz, name, sign);
#ifdef JNI_LOG
    __android_log_print(ANDROID_LOG_DEBUG, TAG, "GetFieldId clazz=%p name=%s sign=%s\n", clazz, name, sign);
#endif
    return jf;
}
static jfieldID    new_GetStaticFieldID(JNIEnv* env, jclass clazz, const char* name,
                                        const char* sign)
{
#ifdef JNI_LOG
    __android_log_print(ANDROID_LOG_DEBUG, TAG, "GetStaticFieldID clazz=%p name=%s sign=%s\n", clazz, name, sign);
#endif
    return old_GetStaticFieldID(env, clazz, name, sign);
}

static jmethodID   new_GetMethodID(JNIEnv* env, jclass clazz, const char* name, const char* sign)
{
    jmethodID methodid;
#ifdef JNI_LOG
    __android_log_print(ANDROID_LOG_DEBUG, TAG, "GetMethodID clazz=%p name=%s sign=%s\n", clazz, name, sign);
#endif
    methodid = old_GetMethodID(env, clazz, name, sign);
    return methodid;
}

static jmethodID   new_GetStaticMethodID(JNIEnv* env, jclass clazz, const char* name, const char* sign)
{
    jmethodID methodid;
#ifdef JNI_LOG
    __android_log_print(ANDROID_LOG_DEBUG, TAG, "GetStaticMethodID clazz=%p, name=%s sign=%s\n", clazz, name, sign);
#endif
    methodid = old_GetStaticMethodID(env, clazz, name, sign);
    return methodid;
}

//jstring     (*NewStringUTF)(JNIEnv*, const char*);
static jstring     new_NewStringUTF(JNIEnv* env, const char* str){
#ifdef JNI_LOG
    __android_log_print(ANDROID_LOG_DEBUG, TAG, "new_NewStringUTF str=%s\n", str);
#endif
    return old_NewStringUTF(env, str);
}
static const char* new_GetStringUTFChars(JNIEnv* env, jstring jstr, jboolean* jb){
    const char * p = old_GetStringUTFChars(env, jstr, jb);
#ifdef JNI_LOG
    __android_log_print(ANDROID_LOG_DEBUG, TAG, "GetStringUTFChars str=%s\n", p);
#endif
    return p;
}
static jclass      new_GetObjectClass(JNIEnv* env, jobject obj){
    jclass jc = old_GetObjectClass(env, obj);
#ifdef JNI_LOG
    __android_log_print(ANDROID_LOG_DEBUG, TAG, "GetObjectClass obj=%p, return jclass=%p\n", obj, jc);
#endif
    return jc;
}
static jclass      new_GetSuperclass(JNIEnv* env, jclass jc){
    jclass jc2 = old_GetSuperclass(env, jc);
#ifdef JNI_LOG
    __android_log_print(ANDROID_LOG_DEBUG, TAG, "GetSuperclass jc=%p, return jc=%p\n", jc, jc2);
#endif
    return jc2;
}
static JNIEXPORT jstring new_JVM_NativeLoad(JNIEnv* env, jstring javaFilename,
                                 jobject javaLoader, jclass caller){
#ifdef JNI_LOG
    char* msg = jstringToChar(env, javaFilename);
    __android_log_print(ANDROID_LOG_DEBUG, TAG, "JVM_NativeLoad javaFilename=%s\n", msg);
    free(msg);
#endif
    return old_JVM_NativeLoad(env, javaFilename, javaLoader, caller);
}


//----------------libc-------------------
static int new_openat(int dirFd, const char *pathName, int flag, int mode){
    #ifdef JNI_LOG
        __android_log_print(ANDROID_LOG_DEBUG, TAG, "openat dirFd=%d pathName=%s flag=%d mode=%d\n", dirFd, pathName, flag, mode);
    #endif
    return old_openat(dirFd, pathName, flag, mode);
}
static void* new_dlopen(const char* filename, int flag){
    #ifdef JNI_LOG
        __android_log_print(ANDROID_LOG_DEBUG, TAG, "dlopen filename=%s flag=%d\n", filename, flag);
    #endif
    return old_dlopen(filename, flag);
}
static int new_access(const char* path, int mode){
#ifdef JNI_LOG
    __android_log_print(ANDROID_LOG_DEBUG, TAG, "access path=%s mode=%d\n", path, mode);
#endif
    return old_access(path, mode);
}
static int new_kill(pid_t pid, int signal){
#ifdef JNI_LOG
    __android_log_print(ANDROID_LOG_DEBUG, TAG, "kill pid=%d signal=%d\n", pid, signal);
#endif
    return old_kill(pid, signal);
}
static FILE* new_popen(const char* command, const char* mode){
#ifdef JNI_LOG
    __android_log_print(ANDROID_LOG_DEBUG, TAG, "popen command=%s mode=%s\n", command, mode);
#endif
    return old_popen(command, mode);
}


static void hookJni(JNIEnv *env){
    //autohook((void*)(*env)->RegisterNatives, (void *) new_RegisterNatives, (void **)&old_RegisterNatives);
    autohook((void*)(*env)->FindClass, (void *) new_FindClass, (void **)&old_FindClass);

    //field
    autohook((void*)(*env)->GetFieldID, (void *) new_GetFieldID, (void **)&old_GetFieldID);
    autohook((void*)(*env)->GetStaticFieldID, (void *) new_GetStaticFieldID, (void **)&old_GetStaticFieldID);

    //method
    autohook((void*)(*env)->GetMethodID, (void *)new_GetMethodID, (void **)&old_GetMethodID);
    autohook((void*)(*env)->GetStaticMethodID, (void *) new_GetStaticMethodID, (void **)&old_GetStaticMethodID);

    //string
    autohook((void*)(*env)->NewStringUTF, (void *) new_NewStringUTF, (void **)&old_NewStringUTF);
    autohook((void*)(*env)->GetStringUTFChars, (void *) new_GetStringUTFChars, (void **)&old_GetStringUTFChars);

    autohook((void*)(*env)->GetObjectClass, (void *) new_GetObjectClass, (void **)&old_GetObjectClass);
    autohook((void*)(*env)->GetSuperclass, (void *) new_GetSuperclass, (void **)&old_GetSuperclass);

    autohook_sym_name("libopenjdkjvm.so", "JVM_NativeLoad", (void *)new_JVM_NativeLoad, (void **)&old_JVM_NativeLoad);
}

static void hookLibc(void){
    autohook((void *)dlopen, (void *)new_dlopen, (void **)&old_dlopen);
    autohook((void *)access, (void *)new_access, (void **)&old_access);
    autohook((void *)kill, (void *)new_kill, (void **)&old_kill);
    autohook((void *)popen, (void *)new_popen, (void **)&old_popen);
    autohook_sym_name("libc.so", "__openat", (void *)new_openat, (void **)&old_openat);
}

JNIEXPORT void Java_com_bytedance_shadowhook_HLogger_startHook(JNIEnv *env, jobject thiz) {
    (void) thiz;
    hookJni(env);
    hookLibc();
}
