#include "Itestbinder.h"  
#include <binder/Parcel.h>  
#include <binder/IInterface.h>  
namespace android{  
  enum {  
    TEST_INTERFACE,  
  };  
//////////////////客户端  
  class Bptestbinder : public BpInterface<Itestbinder>{  
    public:  
      Bptestbinder(const sp<IBinder>& impl) : BpInterface<Itestbinder>(impl){  
      }  
      virtual int testinterface(int a){  
        LOGD("TK---->>>>>>Itestbinder.cpp>>>>Bptestbinder::testinterface\n");  
        Parcel data,reply;  
        data.writeInt32(a);  
        remote()->transact(TEST_INTERFACE,data,&reply);  
        return reply.readInt32();  
      }  
  };  
  
  IMPLEMENT_META_INTERFACE(testbinder, "android.test.Itestbinder");  
/////////////////服务端  
  status_t Bntestbinder::onTransact(  
      uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags){  
    LOGD("TK---->>>>>>Itestbinder.cpp>>>>Bntestbinder::onTransact\n");  
    switch (code) {  
      case TEST_INTERFACE:{  
        //CHECK_INTERFACE(Itestbinder, data, reply);  
        LOGD("TK---->>>>>>Itestbinder.cpp>>>>Bntestbinder::onTransact>>111\n");  
        reply->writeInt32(testinterface((int) data.readInt32()) );  
        return NO_ERROR;  
      } break;  
      default:{  
        LOGD("TK---->>>>>>Itestbinder.cpp>>>>Bntestbinder::onTransact>>222\n");  
        return BBinder::onTransact(code, data, reply, flags);  
      }  
    }  
  }  
}