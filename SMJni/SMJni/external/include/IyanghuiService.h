#ifndef _I_YANGHUI_SERVICE_H
#define _I_YANGHUI_SERVICE_H

#include <binder/IInterface.h>
#include "Icallback.h"

namespace android{
	
  class IyanghuiService : public IInterface{
    public:
      DECLARE_META_INTERFACE(yanghuiService);
      virtual status_t openMemory(const char* fileName, status_t size) = 0;
      virtual int setcallback(const sp<Icallback>& callback) = 0;
	  virtual status_t closeMemory() = 0;
  };
  class BnyanghuiService : public BnInterface<IyanghuiService>{
  public:
    virtual status_t    onTransact( uint32_t code,
                                    const Parcel& data,
                                    Parcel* reply,
                                    uint32_t flags = 0);
  };
}

#endif
