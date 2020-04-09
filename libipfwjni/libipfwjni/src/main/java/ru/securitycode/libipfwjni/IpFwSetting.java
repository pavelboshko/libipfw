package ru.securitycode.libipfwjni;

public class IpFwSetting {
    public IpFwSetting(String forwardHttpsHost, int forwardHttpsPort,
                       String forwardHttpHost, int forwardHttpPort, int tunFd){
        this.forwardHttpsHost = forwardHttpsHost;
        this.forwardHttpsPort = forwardHttpsPort;
        this.forwardHttpHost = forwardHttpHost;
        this.forwardHttpPort = forwardHttpPort;
        this.tunFd = tunFd;
    }


    final public String forwardHttpsHost;
    final public int forwardHttpsPort;
    final public String forwardHttpHost;
    final public int forwardHttpPort;
    final public int tunFd;
}
