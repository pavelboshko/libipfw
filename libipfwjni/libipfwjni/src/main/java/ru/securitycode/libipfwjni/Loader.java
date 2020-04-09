package ru.securitycode.libipfwjni;

public class Loader {
    static {
        System.setProperty("java.library.path", "libs");
        System.setProperty("jni.library.path", "libs");
        System.loadLibrary("ipfw");
        System.loadLibrary("ipfwjni");
    }
}
