package ru.securitycode.ipfw


import android.app.Activity
import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.net.VpnService
import android.os.Bundle
import android.view.View
import androidx.appcompat.app.AppCompatActivity
import androidx.localbroadcastmanager.content.LocalBroadcastManager
import kotlinx.android.synthetic.main.activity_main.*

import timber.log.Timber
import java.lang.RuntimeException

class MainActivity : AppCompatActivity() {


    lateinit var localBroadcastManager: LocalBroadcastManager
    var tlsHost : String = ""
    var tlsAnon : Boolean = false

    private val broadcastReceiver: BroadcastReceiver = object : BroadcastReceiver() {
        override fun onReceive(context: Context, intent: Intent) {
            if (intent.action == ACTIVITY_INTENT) {

                if (intent.getStringExtra(ACTIVITY_RESOURCE_STARTED) != null) {
                    Timber.d("ACTIVITY_RESOURCE_STARTED")
                    tls_client_stop.setEnabled(true)
                    resource_status.setText("Enable " + tlsHost)
                }
            }
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        Timber.plant( Timber.DebugTree());
        initBReciver()

        tls_client_one_side.setOnClickListener(View.OnClickListener {
            tlsHost = "tls-mobile-anon.securitycode.ru"
            tls_client_one_side.setEnabled(false)
            tls_client_two_side.setEnabled(false)
            tls_client_stop.setEnabled(false)
            tlsAnon = true
            resource_status.setText("Starting " + tlsHost)
            startVpn()
        })

        tls_client_two_side.setOnClickListener(View.OnClickListener {
            tlsHost = "tls-mobile.securitycode.ru"
            tls_client_one_side.setEnabled(false)
            tls_client_two_side.setEnabled(false)
            tls_client_stop.setEnabled(false)
            tlsAnon = false
            resource_status.setText("Starting " + tlsHost)
            startVpn()
        })

        tls_client_stop.setOnClickListener(View.OnClickListener {
            stopVpn()
            tls_client_one_side.setEnabled(true)
            tls_client_two_side.setEnabled(true)
            resource_status.setText("")
        })
    }

    private fun initBReciver() {
        localBroadcastManager = LocalBroadcastManager.getInstance(this)
        val intentFilter = IntentFilter()
        intentFilter.addAction(ACTIVITY_INTENT)
        localBroadcastManager.registerReceiver(broadcastReceiver, intentFilter)
    }

    override fun onDestroy() {
        stopVpn()
        localBroadcastManager.unregisterReceiver(broadcastReceiver)
        super.onDestroy()
    }


    fun stopVpn(){
        val intent = Intent(this, ServiceSinkhole::class.java)
        intent.putExtra("COMMAND", "STOP")
        startService(intent)
    }

    fun startVpn(){
        val intent= VpnService.prepare(this)
        if (intent!=null){
            startActivityForResult(intent, VPN_REQUEST_CODE);
        }else{
            onActivityResult(VPN_REQUEST_CODE, Activity.RESULT_OK, null);
        }
    }

    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        super.onActivityResult(requestCode, resultCode, data)
        if (requestCode == VPN_REQUEST_CODE && resultCode== Activity.RESULT_OK){
            val intent = Intent(this, ServiceSinkhole::class.java)


            intent.putExtra("COMMAND", "START")
            intent.putExtra("TLS_HOST", tlsHost)
            intent.putExtra("TLS_ANON", tlsAnon)
            startService(intent)
        }
    }
}
