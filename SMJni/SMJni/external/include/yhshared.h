
#ifndef _YH_SHARED_H_
#define _YH_SHARED_H_

#include <utils/RefBase.h>
#include <sys/types.h>
#include <stdint.h>
#include "../include/base.h"
#include <stdint.h>
#include <media/nbaio/roundup.h>
#include <sys/types.h>
#include <linux/futex.h>
#include <sys/syscall.h>
#include <utils/threads.h>


#define CBLK_UNDERRUN 0x01   
#define CBLK_FORCEREADY 0x02 
#define CBLK_INVALID 0x04
#define CBLK_DISABLED 0x08

#define CBLK_LOOP_CYCLE 0x20
#define CBLK_LOOP_FINAL 0x40
#define CBLK_BUFFER_END 0x80
#define CBLK_OVERRUN 0x100
#define CBLK_INTERRUPT 0x200
#define CBLK_STREAM_END_DONE 0x400

#define CBLK_FUTEX_WAKE 1

#define RESULT_OK 0
#define RESULT_FAILED -1


using namespace android;

struct Buffer {

   size_t mFrameCount;
   void* mRaw;
   size_t mNonContig;
 
};


struct yanghui_track_cblk_t {

   yanghui_track_cblk_t();
   ~yanghui_track_cblk_t();

   uint32_t mServer;

   volatile int32_t mFront;
   volatile int32_t mRear;
   volatile int32_t mFlush;

   volatile uint32_t mUnderrunFrames; 
   volatile int32_t mFutex;
   volatile int32_t mFlags;
   
   volatile int32_t mMinimum;

   friend class YhProxy;
   friend class YhClientProxy;
   friend class YhServerProxy;
};


class YhProxy : public RefBase {


  public:
     YhProxy(yanghui_track_cblk_t* cblk, void* buffers, size_t frameCount, size_t frameSize);
	 virtual ~YhProxy()
	 {}

	 virtual size_t obtainBuffer(Buffer* buffer) = 0;
	 virtual void releaseBuffer(Buffer* buffer) = 0;

  protected:
    yanghui_track_cblk_t* const mCblk;
	size_t mFrameSize;
    const size_t mFrameCount;
    const size_t mFrameCountP2; 
    void* mBuffers;
    size_t mUnreleased;
};

class YhClientProxy : public YhProxy {

   public:
   	 YhClientProxy(yanghui_track_cblk_t* cblk, void* buffers, size_t frameCount,size_t frameSize)
	 	:YhProxy(cblk,buffers,frameCount,frameSize){
        LOGI("init YhClientProxy...");
	 }

	 ~YhClientProxy(){
       LOGI("destory yhclient proxy.");
	 }

	 virtual size_t obtainBuffer(Buffer* buffer);
	 virtual void releaseBuffer(Buffer* buffer);
    
     void setMinimum(size_t minimum){

         if(minimum > UINT32_MAX){
             minimum = UINT32_MAX;
         }
         mCblk->mMinimum = (uint32_t) minimum;
     }
    
};


class YhServerProxy : public YhProxy {
    public:
       YhServerProxy(yanghui_track_cblk_t* cblk, void* buffers, size_t frameCount,size_t frameSize)
        :YhProxy(cblk,buffers,frameCount,frameSize){
            LOGI("init YhServerProxy...");
        }

      ~YhServerProxy(){
        LOGI("destroy YhServer proxy.");
      }

      virtual size_t obtainBuffer(Buffer* buffer);
      virtual void releaseBuffer(Buffer* buffer);

    protected:
      size_t mAvailToClient;

};


#endif


