#include "../include/yhshared.h"

using namespace android;


yanghui_track_cblk_t::yanghui_track_cblk_t(){

   LOGI("init yanghui_track_cblk_t.");
   mServer = 0;
   mFront = 0;
   mRear = 0;
   mUnderrunFrames = 0;
   mMinimum = 0;
  
}


YhProxy::YhProxy(yanghui_track_cblk_t* cblk, void* buffers, size_t frameCount, size_t frameSize)
        : mCblk(cblk),mBuffers(buffers),mFrameCount(frameCount),mFrameSize(frameSize),mFrameCountP2(roundup(frameCount))
{
}

size_t YhClientProxy::obtainBuffer(Buffer* buffer){


   yanghui_track_cblk_t* cblk = mCblk;
   size_t status = RESULT_OK;

   for(;;){
 
      int32_t flags = android_atomic_and(~CBLK_INTERRUPT, &cblk->mFlags);
      if(flags & CBLK_INVALID){
        LOGI("cblk invaild");
        status = RESULT_FAILED; // invaild.
        goto end;
      }
     
      int32_t front = android_atomic_acquire_load(&cblk->mFront);
      int32_t rear = cblk->mRear;

      ssize_t filled = rear - front;
	  LOGI("front : %d, rear : %d, filled : %d", front, rear, filled);
      if(!(0 <= filled && (size_t)filled <= mFrameCount)){
          status = RESULT_FAILED; //not init.
          goto end;
      }

      size_t avail = mFrameCount - filled;
	  LOGI("avail : %d", avail);
      if(avail > 0){
   
          size_t part1;
          rear &= mFrameCountP2 - 1;
          part1 = mFrameCountP2 - rear;
       
    
          if(part1 > avail){
            part1 = avail;
          }

         if(part1 > buffer->mFrameCount){
            part1 = buffer->mFrameCount;
         }

         buffer->mFrameCount = part1;
         buffer->mRaw = part1 > 0 ? &((char*)mBuffers)[rear * mFrameSize] : NULL;
         buffer->mNonContig = avail - part1;
         status = RESULT_OK;
         mUnreleased = part1;
         break; 
       }

       //abnormal process.
       int32_t old = android_atomic_and(~CBLK_FUTEX_WAKE, &cblk->mFutex);
       if(!(old & CBLK_FUTEX_WAKE)) {
        
           errno = 0;
           (void) syscall(__NR_futex, &cblk->mFutex, FUTEX_WAIT, old & ~CBLK_FUTEX_WAKE, NULL);
           
           switch(errno){
             case 0:
             case EWOULDBLOCK:
             case EINTR:
             case ETIMEDOUT:
             break;
             default:
               status = errno;
               LOGE("%s unexpected error %s", __func__, strerror(status));
               goto end;
           }
       }
   }


end:
   if(status != RESULT_OK){
      buffer->mFrameCount = 0;
      buffer->mRaw = NULL;
      buffer->mNonContig = 0;
   }
    
   return status;
}

void YhClientProxy::releaseBuffer(Buffer* buffer){

   if(buffer == NULL){
       LOGE("client proxy release buffer is null.");
       return ;
   }

   size_t stepCount = buffer->mFrameCount;
   if(stepCount == 0){
        buffer->mFrameCount = 0;
        buffer->mRaw = NULL;
        buffer->mNonContig = 0;
   }

   if(!(stepCount <= mUnreleased && mUnreleased <= mFrameCount)){
       LOGE("inner error.");
       return;
   }

   mUnreleased -= stepCount;
   yanghui_track_cblk_t* cblk = mCblk;

   int32_t rear = cblk->mRear;
   android_atomic_release_store(stepCount + rear, &cblk->mRear);
   LOGI("cblk mRear : %d", cblk->mRear);
}


size_t YhServerProxy::obtainBuffer(Buffer* buffer){

  size_t status = RESULT_OK;
  if(buffer == NULL || buffer->mFrameCount == 0){

	 buffer->mFrameCount = 0;
     buffer->mRaw = NULL;
     buffer->mNonContig = 0;
     mUnreleased = 0;
     return RESULT_FAILED;
  }

  yanghui_track_cblk_t* cblk = mCblk;

  int32_t rear = android_atomic_acquire_load(&cblk->mRear);
  int32_t front = cblk->mFront;

  ssize_t filled = rear - front;
  if(!(0 <= filled && (size_t)filled <= mFrameCount)){
     LOGE(" shared memory control block is corrupt (filled = %zd); shutting down", filled);
     buffer->mFrameCount = 0;
     buffer->mRaw = NULL;
     buffer->mNonContig = 0;
     mUnreleased = 0;
     return RESULT_FAILED;
  }

  size_t availToServer;
  availToServer = filled;
  mAvailToClient = mFrameCount - filled;
  
  size_t part1;
  front &= mFrameCountP2 - 1;
  part1 = mFrameCountP2 - front;

  if(part1 > availToServer){
     part1 = availToServer;
  }

  size_t ask = buffer->mFrameCount;
  if(part1 > ask){
     part1 = ask;
  }
  
  buffer->mFrameCount = part1;
  buffer->mRaw = part1 > 0 ? &((char*)mBuffers)[front * mFrameSize] : NULL;
  buffer->mNonContig = availToServer - part1;

  mUnreleased = part1;

  status = (part1 > 0) ? RESULT_OK : RESULT_FAILED;
 
  return status;
}

void YhServerProxy::releaseBuffer(Buffer* buffer){

   if(buffer == NULL){
      LOGE("release buffer is null.");
      return ;
   }

   size_t stepCount = buffer->mFrameCount;
   if(stepCount == 0){
      buffer->mFrameCount = 0;
      buffer->mRaw = NULL;
      buffer->mNonContig = 0;
      return ;
   }

   if(!(stepCount <= mUnreleased && mUnreleased <= mFrameCount)){
      LOGE("release inner error.");
      return ;
   }

   mUnreleased -= stepCount;
   yanghui_track_cblk_t* cblk = mCblk;
   int32_t front = cblk->mFront;
   android_atomic_release_store(stepCount + front, &cblk->mFront);

   cblk->mServer += stepCount;

   size_t half = mFrameCount / 2;
   if(half == 0){
      half = 1;
   }

   size_t minimum = (size_t) cblk->mMinimum;
   if(minimum == 0){
      minimum = half;
   } else if(minimum > half){
      minimum = half;
   }

   //wake client.
   int32_t old = android_atomic_or(CBLK_FUTEX_WAKE, &cblk->mFutex);
   if(mAvailToClient + stepCount >= minimum){
      if(!(old & CBLK_FUTEX_WAKE)){
         (void) syscall(__NR_futex, &cblk->mFutex, FUTEX_WAKE, 1);
      }
   }
   
   buffer->mFrameCount = 0;
   buffer->mRaw = NULL;
   buffer->mNonContig = 0;

}



