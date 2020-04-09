package ru.securitycode.ipfw

import android.app.Service
import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.net.VpnService
import android.os.ParcelFileDescriptor
import androidx.localbroadcastmanager.content.LocalBroadcastManager
import ru.securitycode.itls.Mitm
import timber.log.Timber
import java.lang.Exception
import java.net.InetAddress


class ServiceSinkhole : VpnService() {


    var alive = true
    var adapter: IpfwTlsAdapter? = null;

    private var vpnInterface: ParcelFileDescriptor? = null


    override fun onStartCommand(intent: Intent?, flags: Int, startId: Int): Int {
        if (intent?.getStringExtra("COMMAND") == "STOP") {
            stopVpn()
        }
        if (intent?.getStringExtra("COMMAND") == "START") {
            setupVpn(intent)
        }
        return Service.START_STICKY
    }



    private  fun setupVpn(intent: Intent) {
        var builder = Builder()
        doAsync {

            val tlsHost = intent.getStringExtra("TLS_HOST")
            println(tlsHost)
            builder.addAddress(TUN_IP_ADDR, 24)

            for (addr in InetAddress.getAllByName(tlsHost))  {
                Timber.d("add route for : " +
                    addr.getHostAddress()
                )
                builder.addRoute(addr.getHostAddress(), 32)
            }
            builder.setSession(tlsHost)

            vpnInterface = builder.establish()

            if(intent.getBooleanExtra("TLS_ANON", false)) {
                startOneSideTls()
            } else {
                startTwoSideTls()
            }

            sendResourceStared(tlsHost);
        }
    }

    fun sendResourceStared(status: String?) {
        val intent = Intent(ACTIVITY_INTENT)
        intent.putExtra(ACTIVITY_RESOURCE_STARTED, status)
        LocalBroadcastManager.getInstance(this).sendBroadcast(intent)
    }

    private fun stopVpn() {
        try {
            adapter!!.stop()
            vpnInterface?.close()
            alive = false
            stopSelf()
            Timber.d("Stopped VPN")
        } catch (e : Exception) {
            Timber.e(e.message)
        }

    }

    private fun startTwoSideTls() {
        Timber.d(">>>  startTwoSideTls ")

        val root_gost = readAssetsFile("tls-mobile.securitycode.ru/ca.cer" , applicationContext)
        val user_cert_gost = readAssetsFile("tls-mobile.securitycode.ru/user.cer" , applicationContext)
        val pkey_gost = readAssetsFile("tls-mobile.securitycode.ru/user.p15" , applicationContext)
        val pkey_gost_passw = "111111"

        val Credentials = Mitm.createCredentials("tls-mobile.securitycode.ru")

        val rsa_cert = Credentials.certs
        val rsa_key =  Credentials.key
        val rsa_key_passw = ""


        adapter = IpfwTlsAdapter( vpnInterface!!.getFd(), pkey_gost_passw,
            user_cert_gost, pkey_gost, root_gost,  rsa_cert, rsa_key, rsa_key_passw,
            { sock: Int -> super.protect(sock) }
        )

        adapter!!.start()
    }

    private fun startOneSideTls() {
        Timber.d("!!! startOneSideTls ")

        val root_gost = readAssetsFile("tls-mobile-anon.securitycode.ru/ca.cer" , applicationContext)

        val Credentials = Mitm.createCredentials("tls-mobile-anon.securitycode.ru")
        val rsa_cert =  Credentials.certs
        val rsa_key = Credentials.key
        val rsa_key_passw = ""

        adapter = IpfwTlsAdapter( vpnInterface!!.getFd(),
            root_gost, rsa_cert, rsa_key, rsa_key_passw,
            { sock: Int -> super.protect(sock) }
        )

        adapter!!.start()
    }
}