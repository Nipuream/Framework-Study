#ifndef _IYANGHUISERVICE_H_
#define _IYANGHUISERVICE_H_


#include <utils/Errors.h>
#include <utils/RefBase.h>
#include <binder/IInterface.h>
#include <binder/IMemory.h>

using namespace android;

namespace yanghui{

	class IyanghuiService : public IInterface {

	public:
		DECLARE_META_INTERFACE(yanghuiService);

		virtual status_t native_opensocket() = 0;
		virtual sp<IMemory> native_createShareMemory() = 0;
		virtual char* native_readMemory(size_t len) = 0;
		//virtual status_t native_createShareMemory() = 0;
	};

	class BnyanghuiService : public BnInterface<IyanghuiService>{

	public:
		enum {
			CID_OPEN_SOCKET = IBinder::FIRST_CALL_TRANSACTION,
			CID_CREATE_SHARE_MEMORY ,
			CID_READ_MEMORY,
		};
		virtual status_t onTransact(uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags = 0);
	};

}





#endif