//
// Created by wzl on 2023/6/27.
//

#ifndef SHADOWHOOK_HLOGGER_H
#define SHADOWHOOK_HLOGGER_H

#include <jni.h>
#include <android/log.h>
#include "include/shadowhook.h"

JNIEXPORT  void Java_com_bytedance_shadowhook_HLogger_startHook(JNIEnv *env, jobject thiz);

#endif //SHADOWHOOK_HLOGGER_H
