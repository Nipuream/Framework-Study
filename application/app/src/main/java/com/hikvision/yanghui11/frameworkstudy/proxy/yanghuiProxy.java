package com.hikvision.yanghui11.frameworkstudy.proxy;

import android.os.IBinder;
import android.os.Parcel;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.util.Log;

/**
 * Created by yanghui11 on 2018/4/10.
 */

public class yanghuiProxy implements IyanghuiInterface {


    private IBinder mRemote;

    @Override
    public int nativeOpenSocket() throws RemoteException {

        Parcel _data = Parcel.obtain();
        Parcel _reply = Parcel.obtain();
        int result = 0;
        try{
            _data.writeInterfaceToken(DESCRIPTOR);
            mRemote.transact(TRANSACTION_CID_OPEN_SOCKET,_data,_reply,0);
            result = _reply.readInt();

        }finally {
            _reply.recycle();
            _data.recycle();
        }

        return result;
    }

    @Override
    public int nativeCreateShareMemory() throws RemoteException {

        Parcel _data = Parcel.obtain();
        Parcel _reply = Parcel.obtain();
        int result = 0;
        try{

            _data.writeInterfaceToken(DESCRIPTOR);
            mRemote.transact(TRANSACTION_CID_CREATE_SHARE_MEMORY, _data , _reply , 0);
            result = _reply.readInt();

        } finally {

            _reply.recycle();
            _data.recycle();

        }
        return result;
    }

    public yanghuiProxy(){
        mRemote = ServiceManager.getService("yanghui_service");
    }

    public String getInterfaceDescriptor(){
        return DESCRIPTOR;
    }

    @Override
    public IBinder asBinder() {
        return null;
    }
}
