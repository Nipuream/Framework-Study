#include "IyanghuiService.h"
#include <utils/Errors.h>
#include <utils/RefBase.h>
#include <binder/Parcel.h>
#include "base_log.h"


namespace yanghui{

	class BpyanghuiService : public BpInterface<IyanghuiService>{

	public:
		BpyanghuiService(const sp<IBinder>& impl):
			BpInterface<IyanghuiService>(impl){}

		virtual status_t native_opensocket(){
			Parcel data, reply;
			data.writeInterfaceToken(IyanghuiService::getInterfaceDescriptor());
			remote()->transact(BnyanghuiService::CID_OPEN_SOCKET, data, &reply);
			return reply.readInt32();
		}


		virtual sp<IMemory> native_createShareMemory(){
			Parcel data, reply;
			sp<IMemory> cblk;
			data.writeInterfaceToken(IyanghuiService::getInterfaceDescriptor());
			status_t status = remote()->transact(BnyanghuiService::CID_CREATE_SHARE_MEMORY, data, &reply);
			if (status == NO_ERROR) {
				cblk = interface_cast<IMemory>(reply.readStrongBinder());
			}
			return cblk;
		}

		virtual char* native_readMemory(size_t len){
			Parcel data, reply;
			data.writeInterfaceToken(IyanghuiService::getInterfaceDescriptor());
			data.writeInt32(len);
			remote()->transact(BnyanghuiService::CID_READ_MEMORY, data, &reply);
			const char *result = reply.readCString();
			LOGD("trans result : %s", result);
			return const_cast<char *>(result);
		}

		/*
		virtual status_t native_createShareMemory(){
			Parcel data, reply;
			data.writeInterfaceToken(IyanghuiService::getInterfaceDescriptor());
			remote()->transact(BnyanghuiService::CID_CREATE_SHARE_MEMORY, data, &reply);
			return reply.readInt32();
		}
		*/
	};

	IMPLEMENT_META_INTERFACE(yanghuiService, "yanghui.service");

	status_t BnyanghuiService::onTransact(uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags){
	
		switch (code){
		case CID_OPEN_SOCKET:{
			CHECK_INTERFACE(IyanghuiService, data, reply);
			reply->writeInt32(native_opensocket());
			return NO_ERROR;
		}

		case CID_CREATE_SHARE_MEMORY:{
			CHECK_INTERFACE(IyanghuiService, data, reply);
			reply->writeStrongBinder(native_createShareMemory()->asBinder());
			return NO_ERROR;
		}

		case CID_READ_MEMORY:{
			CHECK_INTERFACE(IyanghuiService, data, reply);
			int len = data.readInt32();
			char *result = native_readMemory(len);
			LOGI("CID_READ_MEMORY result : %s", result);
			reply->write(result, len);
			return NO_ERROR;
		}

							 /*
		case CID_CREATE_SHARE_MEMORY:{
			CHECK_INTERFACE(IyanghuiService, data, reply);
			reply->writeInt32(native_createShareMemory());
			return NO_ERROR;
		}
		*/
		default:
			return BBinder::onTransact(code, data, reply, flags);
		}
	

	}
}