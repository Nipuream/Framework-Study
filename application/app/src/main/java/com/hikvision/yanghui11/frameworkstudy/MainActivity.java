package com.hikvision.yanghui11.frameworkstudy;

import android.os.RemoteException;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import com.hikvision.yanghui11.frameworkstudy.proxy.yanghuiProxy;
import java.io.IOException;

public class MainActivity extends AppCompatActivity {

    yanghuiProxy proxy ;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);


        proxy = new yanghuiProxy();
        try {
            //服务端创建共享内存 大小为 1024 *10
            int fd = proxy.nativeCreateShareMemory();
            Log.d("fd",String.valueOf(fd));
            //0x1 PROT_READ  0x2 PROT_WRITE
            int address = SMOperator.mmap(fd, 1024, 0x1 | 0x2); //映射物理实际内存的首地址
            Log.d("address",String.valueOf(address));
            String value = "Hello Share memory";
            byte[] test = value.getBytes();
            try {
                SMOperator.write(fd,test,0,0,1,address);
            } catch (IOException e) {
                e.printStackTrace();
            }
        } catch (RemoteException e) {
            e.printStackTrace();
        }

    }



}
