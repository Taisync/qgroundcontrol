package org.mavlink.qgroundcontrol;

import android.content.Context;

import android.net.nsd.NsdManager;
import android.net.nsd.NsdServiceInfo;
import android.util.Log;

import java.net.InetAddress;
import java.net.UnknownHostException;


public class MDNSManager {
    private static final String TAG = "MDNS";

    public static QGCActivity activity = null;

    private NsdManager mNsdManager = null;
    private NsdServiceInfo mServiceInfo = null;
    private NsdManager.RegistrationListener mListener = null;

    private static  MDNSManager INSTANCE = null;
    private MDNSManager() {
        try {
            Log.i(TAG, "init mNsdManager");
            if (activity == null) {
                Log.e(TAG, "activity is null");
            }
            mNsdManager = (NsdManager) activity.getSystemService(Context.NSD_SERVICE);
            if (mNsdManager == null) {
                Log.e(TAG, "mNsdManager is null");
            }
        } catch (Exception e) {
            Log.e(TAG, "can not get NSD_SERVICE");
        }
    }
    private static MDNSManager getInstance() {
        if (INSTANCE == null) {
            INSTANCE = new MDNSManager();
        }
        return INSTANCE;
    }

    public static void stopBroadcast() {
        MDNSManager.getInstance()._stopBroadcast();
    }

    // start
    public static void startBroadcast(String serviceName,
                                      String serviceType,
                                      int port) {
        MDNSManager.getInstance()._startBroadcast(serviceName, serviceType, port);
    }

    // stop
    private void _stopBroadcast() {
        if (mListener == null) {
            Log.i(TAG, "no listener, do not need stop broadcast");
            return;
        }
        Log.i(TAG, "stop broadcast");
        mNsdManager.unregisterService(mListener);
        mServiceInfo = null;
        mListener = null;
    }

    // check if has same service broadingcasting
    private boolean _isSameBroadcasting(String serviceName, String serviceType, int port) {
        if (mServiceInfo == null) return false;
        return mServiceInfo.getServiceName().equals(serviceName)
                && mServiceInfo.getServiceType().equals(serviceType)
                && mServiceInfo.getPort() == port;
    }

    private void _startBroadcast(String serviceName,
                                 String serviceType,
                                 int port) {
        if (_isSameBroadcasting(serviceName, serviceType, port)) {
            Log.w(TAG, String.format("Already broadcast:%s, type:%s, port:%d", serviceName, serviceType, port));
            return;
        }

        // only support single service
        _stopBroadcast();

        Log.i(TAG, String.format("start broadcast:%s, type:%s, port:%d", serviceName, serviceType, port));

        mServiceInfo = new NsdServiceInfo();
        mServiceInfo.setServiceName(serviceName);
        mServiceInfo.setServiceType(serviceType);
        mServiceInfo.setPort(port);

        mListener = new NsdManager.RegistrationListener() {
            @Override
            public void onRegistrationFailed(NsdServiceInfo nsdServiceInfo, int i) {
                Log.i(TAG, "onRegistrationFailed");
            }

            @Override
            public void onUnregistrationFailed(NsdServiceInfo nsdServiceInfo, int i) {
                Log.i(TAG, "onUnregistrationFailed");
            }

            @Override
            public void onServiceRegistered(NsdServiceInfo nsdServiceInfo) {
                Log.i(TAG, "onServiceRegistered");
            }

            @Override
            public void onServiceUnregistered(NsdServiceInfo nsdServiceInfo) {
                Log.i(TAG, "onServiceUnregistered");
            }
        };
        mNsdManager.registerService(mServiceInfo, NsdManager.PROTOCOL_DNS_SD, mListener);
    }
}
