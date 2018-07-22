package com.hikvision.yanghui11.frameworkstudy.proxy;

import android.os.IBinder;
import android.os.IInterface;
import android.os.RemoteException;

/**
 * Created by yanghui11 on 2018/4/10.
 */

public interface IyanghuiInterface extends IInterface{

    public static final String DESCRIPTOR = "yanghui.service";


    public int nativeOpenSocket() throws RemoteException;
    public int nativeCreateShareMemory() throws RemoteException;

    public static final int TRANSACTION_CID_OPEN_SOCKET = IBinder.FIRST_CALL_TRANSACTION;
    public static final int TRANSACTION_CID_CREATE_SHARE_MEMORY = IBinder.FIRST_CALL_TRANSACTION + 1;




}
