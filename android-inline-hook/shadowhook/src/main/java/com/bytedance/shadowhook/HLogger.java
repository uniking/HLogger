package com.bytedance.shadowhook;

public class HLogger {
    private static native void startHook();

    public static void init(){
        ShadowHook.init();
        startHook();
    }
}
