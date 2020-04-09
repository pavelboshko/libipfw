package ru.securitycode.libipfwjni;

import androidx.test.ext.junit.runners.AndroidJUnit4;

import org.junit.Test;
import org.junit.runner.RunWith;

@RunWith(AndroidJUnit4.class)
public class JniWrapperTest {
    final IpFwSetting ipFwSetting = new IpFwSetting("127.0.0.1", 8080, 0);

    @Test
    public void createFw() {
        IpFw ipFw = new IpFw(ipFwSetting);
        ipFw.start();
        ipFw.stop();
    }

    @Test(expected= RuntimeException.class)
    public void createFwException() {
        IpFw ipFw = new IpFw(ipFwSetting);
        ipFw.start();
        ipFw.stop();
        ipFw.stop();
    }
}
