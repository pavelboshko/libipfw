package ru.securitycode.ipfw

import ru.securitycode.itls.IConnectionMonitor
import ru.securitycode.itls.IP15Monitor
import ru.securitycode.itls.Mitm
import ru.securitycode.itls.TlsProxy
import ru.securitycode.libipfwjni.IpFw
import ru.securitycode.libipfwjni.IpFwSetting
import timber.log.Timber

class IpfwTlsAdapter : IP15Monitor, IConnectionMonitor {

    private val proxyHost: String = "127.0.0.1"
    private lateinit var tls: TlsProxy;
    private lateinit var mitm: Mitm;
    private lateinit var ipFwSetting: IpFwSetting;
    private lateinit var ipfw: IpFw;
    private lateinit var keyPassword: String;
    private lateinit var onSocketProtectedClbk :  (sock: Int) -> Boolean

    private val httpPort: Int = 8080
    private val httpsPort: Int = 8443
    private val httpTimeoutMs: Int = 6000

    constructor( tunFd: Int, root: ByteArray, rsaCert: ByteArray, rsaKey: ByteArray, rsaKeyPassw: String, onSocketProtected: (Int) -> Boolean) {
        tls = TlsProxy(root)
        mitm = Mitm(httpsPort, this,
            this, root, rsaKey, rsaCert, rsaKeyPassw, httpTimeoutMs
        )
        ipFwSetting = IpFwSetting(proxyHost, httpsPort, proxyHost, httpPort, tunFd)
        ipfw = IpFw(ipFwSetting);
        this.keyPassword = String()
        this.onSocketProtectedClbk = onSocketProtected;
    }


    constructor(tunFd: Int, keyPassword: String,
                userCertificate: ByteArray, privateKey: ByteArray, rootCertificate: ByteArray,
                rsaCert: ByteArray, rsaKey: ByteArray, rsaKeyPassw: String, onSocketProtected: (Int) -> Boolean) {

        tls = TlsProxy(userCertificate, privateKey, rootCertificate)

        mitm = Mitm(httpsPort, this,
            this, keyPassword, userCertificate, privateKey, rootCertificate, rsaKey, rsaCert, rsaKeyPassw, httpTimeoutMs
        )

        ipFwSetting = IpFwSetting(proxyHost, httpsPort, proxyHost, httpPort, tunFd)
        ipfw = IpFw(ipFwSetting);
        this.keyPassword = keyPassword
        this.onSocketProtectedClbk = onSocketProtected;

    }

    fun start() {
        tls.startNoAuth(httpPort, keyPassword,  this, this, 0xffff)
        ipfw.start()
        mitm.start()
    }

    fun stop() {
        tls.stop()
        ipfw.stop()
        mitm.stop()
        tls.clean()
    }

    override fun onContainerUpdate(p0: ByteArray?): Boolean {
        return true
    }

    override fun onSocketProtect(sock: Int): Boolean {


        val status = onSocketProtectedClbk(sock)
        Timber.d("!!!!!! onSocketProtect $sock status $status")
        return status;
    }
}