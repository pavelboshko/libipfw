package ru.securitycode.libipfwjni;

public class IpFw extends  Loader {
    private long pointer = 0;

    public IpFw(IpFwSetting ipFwSetting) {
        createIpFw(ipFwSetting);
    }

    public void start() {
        startIpFw();
    }

    public void stop() {
        stopIpFw();
    }

    private native void createIpFw(IpFwSetting ipFwSetting) throws RuntimeException;
    private native void startIpFw() throws RuntimeException;
    private native void stopIpFw() throws RuntimeException;
}
