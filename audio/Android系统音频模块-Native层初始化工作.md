# Android系统音频模块-Native层初始化工作

在前面一篇的文章中，我们知道了音频模块Java层所做的一些事情，总的来说还是比较简单的，下面我们继续学习和探索Native层中系统做了什么工作，首先先简单介绍下，Native层采用了C/S的架构方式，AudioTrack是属于Client端的，AudioFlinger和AudioPolicyService是属于服务端的，AudioFlinger是唯一可以调用HAL层接口的地方，而AudioPolicyService主要是对一些音频策略的选择和一些逻辑调用的中转，下面先来看看Client端AudioTrack都做了什么。

> 首先需要注意的是，我们本篇文章只描述Native层代码的初始化工作，因为整个Native层的代码和逻辑是整个音频模块的核心，想要在一篇文章中讲述完整那是不可能的，其中的很多地方的知识点都可以单独拿出来写一篇文章。

## 1. AudioTrack的初始化工作

在Java层JNI方法中，我们得知不仅通过AudioTrack的无参构造方法生成AudioTrack，还调用了它的set()方法，下面是一些重要的代码片段，它所在的路径是：/framework/av/media/libmedia .

```c++
//AudioTrack的无参构造方法
AudioTrack::AudioTrack()
    : mStatus(NO_INIT),
      mIsTimed(false),
      mPreviousPriority(ANDROID_PRIORITY_NORMAL),
      mPreviousSchedulingGroup(SP_DEFAULT),
      mPausedPosition(0),
      mSelectedDeviceId(AUDIO_PORT_HANDLE_NONE)
{
    mAttributes.content_type = AUDIO_CONTENT_TYPE_UNKNOWN;
    mAttributes.usage = AUDIO_USAGE_UNKNOWN;
    mAttributes.flags = 0x0;
    strcpy(mAttributes.tags, "");
}

status_t AudioTrack::set(
        audio_stream_type_t streamType,
        uint32_t sampleRate,
        audio_format_t format,
        audio_channel_mask_t channelMask,
        size_t frameCount,
        audio_output_flags_t flags,
        callback_t cbf,
        void* user,
        uint32_t notificationFrames,
        const sp<IMemory>& sharedBuffer,
        bool threadCanCallJava,
        int sessionId,
        transfer_type transferType,
        const audio_offload_info_t *offloadInfo,
        int uid,
        pid_t pid,
        const audio_attributes_t* pAttributes,
        bool doNotReconnect)
{


   //校验数据...(省略)
  
   //对参数的处理和初始化工作...(省略)
   
    // validate parameters
    if (!audio_is_valid_format(format)) {
        ALOGE("Invalid format %#x", format);
        return BAD_VALUE;
    }
    mFormat = format;

    if (!audio_is_output_channel(channelMask)) {
        ALOGE("Invalid channel mask %#x", channelMask);
        return BAD_VALUE;
    }
  
    //确定声道数
    mChannelMask = channelMask;
    uint32_t channelCount = audio_channel_count_from_out_mask(channelMask);
    mChannelCount = channelCount;

     //根据flags 确定framesize 大小
    if (flags & AUDIO_OUTPUT_FLAG_DIRECT) {
        if (audio_is_linear_pcm(format)) {
            mFrameSize = channelCount * audio_bytes_per_sample(format);
        } else {
            mFrameSize = sizeof(uint8_t);
        }
    } else {
        ALOG_ASSERT(audio_is_linear_pcm(format));
        mFrameSize = channelCount * audio_bytes_per_sample(format);
        // createTrack will return an error if PCM format is not supported by server,
        // so no need to check for specific PCM formats here
    }

    // sampling rate must be specified for direct outputs
    if (sampleRate == 0 && (flags & AUDIO_OUTPUT_FLAG_DIRECT) != 0) {
        return BAD_VALUE;
    }
    mSampleRate = sampleRate;
    mOriginalSampleRate = sampleRate;
    mPlaybackRate = AUDIO_PLAYBACK_RATE_DEFAULT;

    mVolume[AUDIO_INTERLEAVE_LEFT] = 1.0f;
    mVolume[AUDIO_INTERLEAVE_RIGHT] = 1.0f;
    mSendLevel = 0.0f;
    // mFrameCount is initialized in createTrack_l
    mReqFrameCount = frameCount;
    mNotificationFramesReq = notificationFrames;
    mNotificationFramesAct = 0;
    // ...
    mAuxEffectId = 0;
    mFlags = flags;
    mCbf = cbf;

    //1. 如果Java传入的音频状态回调对象不为空，则开启线程处理回调
    if (cbf != NULL) {
        mAudioTrackThread = new AudioTrackThread(*this, threadCanCallJava);
        mAudioTrackThread->run("AudioTrack", ANDROID_PRIORITY_AUDIO, 0 /*stack*/);
        // thread begins in paused state, and will not reference us until start()
    }

    //2. 创建 sp<IAudioTrack> 对象，主要和服务端进行通信
    status_t status = createTrack_l();


    //... 参数的初始化，赋值工作

    return NO_ERROR;
}
```

这个方法实在太长了，有些不重要的我删除了，还对一些语句做了备注，最为主要的我用序列标出来了，下面就详细学习下这两步到底做了什么。

### 1.1. Native层状态回调

在我们AudioTrack调用set()方法中，创建了一条名叫AudioTrackThread线程，我们来看下它的数据结构：

```c++
    /* a small internal class to handle the callback */
    class AudioTrackThread : public Thread
    {
    public:
        AudioTrackThread(AudioTrack& receiver, bool bCanCallJava = false);

        // Do not call Thread::requestExitAndWait() without first calling requestExit().
        // Thread::requestExitAndWait() is not virtual, and the implementation doesn't do enough.
        virtual void        requestExit();

                void        pause();    // suspend thread from execution at next loop boundary
                void        resume();   // allow thread to execute, if not requested to exit
                void        wake();     // wake to handle changed notification conditions.

    private:
                void        pauseInternal(nsecs_t ns = 0LL);
                                        // like pause(), but only used internally within thread

        friend class AudioTrack;
        virtual bool        threadLoop();
        AudioTrack&         mReceiver;
        virtual ~AudioTrackThread();
        Mutex               mMyLock;    // Thread::mLock is private
        Condition           mMyCond;    // Thread::mThreadExitedCondition is private
        bool                mPaused;    // whether thread is requested to pause at next loop entry
        bool                mPausedInt; // whether thread internally requests pause
        nsecs_t             mPausedNs;  // if mPausedInt then associated timeout, otherwise ignored
        bool                mIgnoreNextPausedInt;   // skip any internal pause and go immediately
                                        // to processAudioBuffer() as state may have changed
                                        // since pause time calculated.
    };
```

我们可以看到它继承于Native层的Thread，其实在调用AudioTrackThread->run()的时候，最终会调用到它的threadLooper()，我们可以看下Thread.h的实现类Threads.cpp，代码位于：

**./system/core/libutils/Threads.cpp**

```c++
status_t Thread::run(const char* name, int32_t priority, size_t stack)
{
    Mutex::Autolock _l(mLock);
    
    //...
    bool res;
    //mCanCallJava表示是否需要attach虚拟机，这里是false.
    if (mCanCallJava) {
        res = createThreadEtc(_threadLoop,
                this, name, priority, stack, &mThread);
    } else {
        res = androidCreateRawThreadEtc(_threadLoop,
                this, name, priority, stack, &mThread);
    }
    //...
}

int androidCreateRawThreadEtc(android_thread_func_t entryFunction,
                               void *userData,
                               const char* threadName __android_unused,
                               int32_t threadPriority,
                               size_t threadStackSize,
                               android_thread_id_t *threadId)
{
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    //...

    errno = 0;
    pthread_t thread;
    //创建 unix thread. 可以看出 entryFunction就是上面方法传下的 _threadLoop
    int result = pthread_create(&thread, &attr,
                    (android_pthread_entry)entryFunction, userData);
    pthread_attr_destroy(&attr);
    if (result != 0) {
        ALOGE("androidCreateRawThreadEtc failed (entry=%p, res=%d, errno=%d)\n"
             "(android threadPriority=%d)",
            entryFunction, result, errno, threadPriority);
        return 0;
    }

    // Note that *threadID is directly available to the parent only, as it is
    // assigned after the child starts.  Use memory barrier / lock if the child
    // or other threads also need access.
    if (threadId != NULL) {
        *threadId = (android_thread_id_t)thread; // XXX: this is not portable
    }
    return 1;
}

int Thread::_threadLoop(void* user)
{
    Thread* const self = static_cast<Thread*>(user);
    
    //...

    bool first = true;

    //可以从下面代码看出，Thread线程会一直调用 threadloop 方法，当然这个循环也会阻塞，用于
    //和其他线程之间的协调工作.
    do {
        bool result;
        if (first) {
            first = false;
            self->mStatus = self->readyToRun();
            result = (self->mStatus == NO_ERROR);

            if (result && !self->exitPending()) {
                // Binder threads (and maybe others) rely on threadLoop
                // running at least once after a successful ::readyToRun()
                // (unless, of course, the thread has already been asked to exit
                // at that point).
                // This is because threads are essentially used like this:
                //   (new ThreadSubclass())->run();
                // The caller therefore does not retain a strong reference to
                // the thread and the thread would simply disappear after the
                // successful ::readyToRun() call instead of entering the
                // threadLoop at least once.
                result = self->threadLoop();
            }
        } else {
            result = self->threadLoop();
        }

        // establish a scope for mLock
        {
        Mutex::Autolock _l(self->mLock);
        if (result == false || self->mExitPending) {
            self->mExitPending = true;
            self->mRunning = false;
            // clear thread ID so that requestExitAndWait() does not exit if
            // called by a new thread using the same thread ID as this one.
            self->mThread = thread_id_t(-1);
            // note that interested observers blocked in requestExitAndWait are
            // awoken by broadcast, but blocked on mLock until break exits scope
            self->mThreadExitedCondition.broadcast();
            break;
        }
        }

        // Release our strong reference, to let a chance to the thread
        // to die a peaceful death.
        strong.clear();
        // And immediately, re-acquire a strong reference for the next loop
        strong = weak.promote();
    } while(strong != 0);

    return 0;
}
```

从上面一大片代码的分析，得出最后我们只要去分析 threadloop() 方法的实现即可，那么继续分析回调AudioTrack.cpp中对AudioTrackThread的threadloop()的实现：

```c++
bool AudioTrack::AudioTrackThread::threadLoop()
{
    {
        AutoMutex _l(mMyLock);
        if (mPaused) {
            mMyCond.wait(mMyLock);
            // caller will check for exitPending()
            return true;
        }
        if (mIgnoreNextPausedInt) {
            mIgnoreNextPausedInt = false;
            mPausedInt = false;
        }
        if (mPausedInt) {
            if (mPausedNs > 0) {
                (void) mMyCond.waitRelative(mMyLock, mPausedNs);
            } else {
                mMyCond.wait(mMyLock);
            }
            mPausedInt = false;
            return true;
        }
    }
    if (exitPending()) {
        return false;
    }
    nsecs_t ns = mReceiver.processAudioBuffer();
    switch (ns) {
    case 0:
        return true;
    case NS_INACTIVE:
        pauseInternal();
        return true;
    case NS_NEVER:
        return false;
    case NS_WHENEVER:
        // Event driven: call wake() when callback notifications conditions change.
        ns = INT64_MAX;
        // fall through
    default:
        LOG_ALWAYS_FATAL_IF(ns < 0, "processAudioBuffer() returned %" PRId64, ns);
        pauseInternal(ns);
        return true;
    }
}
```

其中很重要的一句mReceiver.processAudioBuffer()，其中的mReceiver就是AudioTrack本身，所以，go  on ...

```c++
nsecs_t AudioTrack::processAudioBuffer()
{
    //检测 mCblk 是否为空，mCblk 对应的是Java空间的回调方法
    LOG_ALWAYS_FATAL_IF(mCblk == NULL);

    mLock.lock();

    //...
  
    // Can only reference mCblk while locked
    int32_t flags = android_atomic_and(
        ~(CBLK_UNDERRUN | CBLK_LOOP_CYCLE | CBLK_LOOP_FINAL | CBLK_BUFFER_END), &mCblk->mFlags);

    // Check for track invalidation
    if (flags & CBLK_INVALID) {
        // for offloaded tracks restoreTrack_l() will just update the sequence and clear
        // AudioSystem cache. We should not exit here but after calling the callback so
        // that the upper layers can recreate the track
        if (!isOffloadedOrDirect_l() || (mSequence == mObservedSequence)) {
            status_t status __unused = restoreTrack_l("processAudioBuffer");
            // FIXME unused status
            // after restoration, continue below to make sure that the loop and buffer events
            // are notified because they have been cleared from mCblk->mFlags above.
        }
    }

    //查看 音频服务端(消费者)是否是低负荷状态
    bool waitStreamEnd = mState == STATE_STOPPING;
    bool active = mState == STATE_ACTIVE;

    // Manage underrun callback, must be done under lock to avoid race with releaseBuffer()
    bool newUnderrun = false;
    if (flags & CBLK_UNDERRUN) {
#if 0
        // Currently in shared buffer mode, when the server reaches the end of buffer,
        // the track stays active in continuous underrun state.  It's up to the application
        // to pause or stop the track, or set the position to a new offset within buffer.
        // This was some experimental code to auto-pause on underrun.   Keeping it here
        // in "if 0" so we can re-visit this if we add a real sequencer for shared memory content.
        if (mTransfer == TRANSFER_SHARED) {
            mState = STATE_PAUSED;
            active = false;
        }
#endif
        if (!mInUnderrun) {
            mInUnderrun = true;
            newUnderrun = true;
        }
    }

    //获取服务端的写指针
    size_t position = updateAndGetPosition_l();

    //检测是否达到警戒标志位
    bool markerReached = false;
    size_t markerPosition = mMarkerPosition;
    // FIXME fails for wraparound, need 64 bits
    if (!mMarkerReached && (markerPosition > 0) && (position >= markerPosition)) {
        mMarkerReached = markerReached = true;
    }

    // Determine number of new position callback(s) that will be needed, while locked
    size_t newPosCount = 0;
    size_t newPosition = mNewPosition;
    size_t updatePeriod = mUpdatePeriod;
    // FIXME fails for wraparound, need 64 bits
    if (updatePeriod > 0 && position >= newPosition) {
        newPosCount = ((position - newPosition) / updatePeriod) + 1;
        mNewPosition += updatePeriod * newPosCount;
    }

    // Cache other fields that will be needed soon
    uint32_t sampleRate = mSampleRate;
    float speed = mPlaybackRate.mSpeed;
    const uint32_t notificationFrames = mNotificationFramesAct;
    if (mRefreshRemaining) {
        mRefreshRemaining = false;
        mRemainingFrames = notificationFrames;
        mRetryOnPartialBuffer = false;
    }
    size_t misalignment = mProxy->getMisalignment();
    uint32_t sequence = mSequence;
    sp<AudioTrackClientProxy> proxy = mProxy;

    // Determine the number of new loop callback(s) that will be needed, while locked.
    int loopCountNotifications = 0;
    uint32_t loopPeriod = 0; // time in frames for next EVENT_LOOP_END or EVENT_BUFFER_END

    //获取循环次数
    if (mLoopCount > 0) {
        int loopCount;
        size_t bufferPosition;
        mStaticProxy->getBufferPositionAndLoopCount(&bufferPosition, &loopCount);
        loopPeriod = ((loopCount > 0) ? mLoopEnd : mFrameCount) - bufferPosition;
        loopCountNotifications = min(mLoopCountNotified - loopCount, kMaxLoopCountNotifications);
        mLoopCountNotified = loopCount; // discard any excess notifications
    } else if (mLoopCount < 0) {
        // FIXME: We're not accurate with notification count and position with infinite looping
        // since loopCount from server side will always return -1 (we could decrement it).
        size_t bufferPosition = mStaticProxy->getBufferPosition();
        loopCountNotifications = int((flags & (CBLK_LOOP_CYCLE | CBLK_LOOP_FINAL)) != 0);
        loopPeriod = mLoopEnd - bufferPosition;
    } else if (/* mLoopCount == 0 && */ mSharedBuffer != 0) {
        size_t bufferPosition = mStaticProxy->getBufferPosition();
        loopPeriod = mFrameCount - bufferPosition;
    }

    mLock.unlock();

    // get anchor time to account for callbacks.
    const nsecs_t timeBeforeCallbacks = systemTime();

    /**
     * 这里检测 音频流是否播放完，播放状态回调Java层
     */
    if (waitStreamEnd) {
        // FIXME:  Instead of blocking in proxy->waitStreamEndDone(), Callback thread
        // should wait on proxy futex and handle CBLK_STREAM_END_DONE within this function
        // (and make sure we don't callback for more data while we're stopping).
        // This helps with position, marker notifications, and track invalidation.
        struct timespec timeout;
        timeout.tv_sec = WAIT_STREAM_END_TIMEOUT_SEC;
        timeout.tv_nsec = 0;

        status_t status = proxy->waitStreamEndDone(&timeout);
        switch (status) {
        case NO_ERROR:
        case DEAD_OBJECT:
        case TIMED_OUT:
            if (status != DEAD_OBJECT) {
                //回调Java层 EVENT_STREAM_END 状态
                mCbf(EVENT_STREAM_END, mUserData, NULL);
            }
            {
                AutoMutex lock(mLock);
                // The previously assigned value of waitStreamEnd is no longer valid,
                // since the mutex has been unlocked and either the callback handler
                // or another thread could have re-started the AudioTrack during that time.
                waitStreamEnd = mState == STATE_STOPPING;
                if (waitStreamEnd) {
                    mState = STATE_STOPPED;
                    mReleased = 0;
                }
            }
            if (waitStreamEnd && status != DEAD_OBJECT) {
               return NS_INACTIVE;
            }
            break;
        }
        return 0;
    }

   //回调之前判断的状态...
  
    if (newUnderrun) {
        mCbf(EVENT_UNDERRUN, mUserData, NULL);
    }
  
    //循环播放回调
    while (loopCountNotifications > 0) {
        mCbf(EVENT_LOOP_END, mUserData, NULL);
        --loopCountNotifications;
    }
    if (flags & CBLK_BUFFER_END) {
        mCbf(EVENT_BUFFER_END, mUserData, NULL);
    }
    if (markerReached) {
        mCbf(EVENT_MARKER, mUserData, &markerPosition);
    }
  
    //播放多少音频帧回调一次
    while (newPosCount > 0) {
        size_t temp = newPosition;
        mCbf(EVENT_NEW_POS, mUserData, &temp);
        newPosition += updatePeriod;
        newPosCount--;
    }

    if (mObservedSequence != sequence) {
        mObservedSequence = sequence;
        mCbf(EVENT_NEW_IAUDIOTRACK, mUserData, NULL);
        // for offloaded tracks, just wait for the upper layers to recreate the track
        if (isOffloadedOrDirect()) {
            return NS_INACTIVE;
        }
    }

    // if inactive, then don't run me again until re-started
    if (!active) {
        return NS_INACTIVE;
    }

    //...
  
    // EVENT_MORE_DATA callback handling.
    // Timing for linear pcm audio data formats can be derived directly from the
    // buffer fill level.
    // Timing for compressed data is not directly available from the buffer fill level,
    // rather indirectly from waiting for blocking mode callbacks or waiting for obtain()
    // to return a certain fill level.

    struct timespec timeout;
    const struct timespec *requested = &ClientProxy::kForever;
    if (ns != NS_WHENEVER) {
        timeout.tv_sec = ns / 1000000000LL;
        timeout.tv_nsec = ns % 1000000000LL;
        ALOGV("timeout %ld.%03d", timeout.tv_sec, (int) timeout.tv_nsec / 1000000);
        requested = &timeout;
    }

    //如果还有音频流没有写完
    while (mRemainingFrames > 0) {

        Buffer audioBuffer;
        audioBuffer.frameCount = mRemainingFrames;
        size_t nonContig;
        //向共享内存块申请 一块可用的 buffer.
        status_t err = obtainBuffer(&audioBuffer, requested, NULL, &nonContig);
        LOG_ALWAYS_FATAL_IF((err != NO_ERROR) != (audioBuffer.frameCount == 0),
                "obtainBuffer() err=%d frameCount=%zu", err, audioBuffer.frameCount);
        requested = &ClientProxy::kNonBlocking;
        size_t avail = audioBuffer.frameCount + nonContig;
        ALOGV("obtainBuffer(%u) returned %zu = %zu + %zu err %d",
                mRemainingFrames, avail, audioBuffer.frameCount, nonContig, err);
        if (err != NO_ERROR) {
            if (err == TIMED_OUT || err == WOULD_BLOCK || err == -EINTR ||
                    (isOffloaded() && (err == DEAD_OBJECT))) {
                // FIXME bug 25195759
                return 1000000;
            }
            ALOGE("Error %d obtaining an audio buffer, giving up.", err);
            return NS_NEVER;
        }

        if (mRetryOnPartialBuffer && audio_is_linear_pcm(mFormat)) {
            mRetryOnPartialBuffer = false;
            if (avail < mRemainingFrames) {
                if (ns > 0) { // account for obtain time
                    const nsecs_t timeNow = systemTime();
                    ns = max((nsecs_t)0, ns - (timeNow - timeAfterCallbacks));
                }
                nsecs_t myns = framesToNanoseconds(mRemainingFrames - avail, sampleRate, speed);
                if (ns < 0 /* NS_WHENEVER */ || myns < ns) {
                    ns = myns;
                }
                return ns;
            }
        }

       //回调状态，告诉应用，我还需要更多的buffer.
        size_t reqSize = audioBuffer.size;
        mCbf(EVENT_MORE_DATA, mUserData, &audioBuffer);
        size_t writtenSize = audioBuffer.size;

        // Sanity check on returned size
        if (ssize_t(writtenSize) < 0 || writtenSize > reqSize) {
            ALOGE("EVENT_MORE_DATA requested %zu bytes but callback returned %zd bytes",
                    reqSize, ssize_t(writtenSize));
            return NS_NEVER;
        }

            //...省略一大串代码...主要做以下一些事情.
            // The callback is done filling buffers
            // Keep this thread going to handle timed events and
            // still try to get more data in intervals of WAIT_PERIOD_MS
            // but don't just loop and block the CPU, so wait

            // mCbf(EVENT_MORE_DATA, ...) might either
            // (1) Block until it can fill the buffer, returning 0 size on EOS.
            // (2) Block until it can fill the buffer, returning 0 data (silence) on EOS.
            // (3) Return 0 size when no data is available, does not wait for more data.
            //
            // (1) and (2) occurs with AudioPlayer/AwesomePlayer; (3) occurs with NuPlayer.
            // We try to compute the wait time to avoid a tight sleep-wait cycle,
            // especially for case (3).
            //
            // The decision to support (1) and (2) affect the sizing of mRemainingFrames
            // and this loop; whereas for case (3) we could simply check once with the full
            // buffer size and skip the loop entirely.


        //释放buffer，更新读指针
        releaseBuffer(&audioBuffer);

        // FIXME here is where we would repeat EVENT_MORE_DATA again on same advanced buffer
        // if callback doesn't like to accept the full chunk
        if (writtenSize < reqSize) {
            continue;
        }

        // There could be enough non-contiguous frames available to satisfy the remaining request
        if (mRemainingFrames <= nonContig) {
            continue;
        }
         
         //......
      
    return 0;
}
```
这段代码非常的长，其实主要就是根据共享内存的读写指针的位置，判断当前处于什么样的状态，其中通过mCbf对象回调的状态值定义在 /frameworks/av/include/media/AudioTrack.h 的枚举event_type中：

```c++
    /* Events used by AudioTrack callback function (callback_t).
     * Keep in sync with frameworks/base/media/java/android/media/AudioTrack.java NATIVE_EVENT_*.
     */
    enum event_type {
        EVENT_MORE_DATA = 0,        //需要更多的数据

        EVENT_UNDERRUN = 1,         //处于低负荷状态，目前不能提供更多的数据

        EVENT_LOOP_END = 2,         //循环结束
                                  
        EVENT_MARKER = 3,           //播放音频流达到警戒线
   
        EVENT_NEW_POS = 4,          // Playback head is at a new position
                                    // (See setPositionUpdatePeriod()).
        EVENT_BUFFER_END = 5,       // Playback has completed for a static track.
        EVENT_NEW_IAUDIOTRACK = 6,  // IAudioTrack was re-created, either due to re-routing and
                                    // voluntary invalidation by mediaserver, or mediaserver crash.
        EVENT_STREAM_END = 7,       //播放结束
                                    // back (after stop is called) for an offloaded track.
#if 0   // FIXME not yet implemented
        EVENT_NEW_TIMESTAMP = 8,    // Delivered periodically and when there's a significant change
                                    // in the mapping from frame position to presentation time.
                                    // See AudioTimestamp for the information included with event.
#endif
    };
```

终于分析完了，好了，我们再来看看它如何将数据回调给Java空间的。首先mCbf这个对象在AudioTrack.h定义的类型是 callback_t ，我们只要找到 callback_t 的定义就行了。

​    ***typedef void (callback_t)(int event, void user, void *info);***

可以看出其实 callback_t 就是一个指针函数，它其实在JNI调用AudioTrack对象的set()方法传入的，那么我们再回顾下之前JNI的实现：

```c++
    AudioTrackJniStorage* lpJniStorage = new AudioTrackJniStorage();
    //重点在这里
    lpJniStorage->mCallbackData.audioTrack_class = (jclass)env->NewGlobalRef(clazz);
    // we use a weak reference so the AudioTrack object can be garbage collected.
    lpJniStorage->mCallbackData.audioTrack_ref = env->NewGlobalRef(weak_this);
    lpJniStorage->mCallbackData.busy = false;

    // initialize the native AudioTrack object
    status_t status = NO_ERROR;
    switch (memoryMode) {
    case MODE_STREAM:

        status = lpTrack->set(
                AUDIO_STREAM_DEFAULT,// stream type, but more info conveyed in paa (last argument)
                sampleRateInHertz,
                format,// word length, PCM
                nativeChannelMask,
                frameCount,
                AUDIO_OUTPUT_FLAG_NONE,
                audioCallback, &(lpJniStorage->mCallbackData),//callback, callback data (user)
                0,// notificationFrames == 0 since not using EVENT_MORE_DATA to feed the AudioTrack
                0,// shared mem
                true,// thread can call Java
                sessionId,// audio session ID
                AudioTrack::TRANSFER_SYNC,
                NULL,                         // default offloadInfo
                -1, -1,                       // default uid, pid values
                paa);
        break;

        //Java 空间对应的全局变量
  
// field names found in android/media/AudioTrack.java
#define JAVA_POSTEVENT_CALLBACK_NAME                    "postEventFromNative"
#define JAVA_NATIVETRACKINJAVAOBJ_FIELD_NAME            "mNativeTrackInJavaObj"
#define JAVA_JNIDATA_FIELD_NAME                         "mJniData"
#define JAVA_STREAMTYPE_FIELD_NAME                      "mStreamType"
```

Java对Native层回调的处理在这里：

```java
    //---------------------------------------------------------
    // Java methods called from the native side
    //--------------------
    @SuppressWarnings("unused")
    private static void postEventFromNative(Object audiotrack_ref,
            int what, int arg1, int arg2, Object obj) {
        //logd("Event posted from the native side: event="+ what + " args="+ arg1+" "+arg2);
        AudioTrack track = (AudioTrack)((WeakReference)audiotrack_ref).get();
        if (track == null) {
            return;
        }

        if (what == AudioSystem.NATIVE_EVENT_ROUTING_CHANGE) {
            track.broadcastRoutingChange();
            return;
        }
        NativePositionEventHandlerDelegate delegate = track.mEventHandlerDelegate;
        if (delegate != null) {
            Handler handler = delegate.getHandler();
            if (handler != null) {
                Message m = handler.obtainMessage(what, arg1, arg2, obj);
                handler.sendMessage(m);
            }
        }
    }

```

具体做了哪些事情，我们就不继续分析了，不然真的没完没了了 ^v^ ~.

### 1.2. 创建客户端代理对象

我们继续了解下之前AudioTrack对象初始化所做的第二个很重要的事情，来回顾下代码：

***status_t status = createTrack_l();***

这个方法是非常重要的方法，它不仅创建了客户端与服务端之间通信的桥梁，也相当于告诉服务端我Client端已经准备差不多了，你Server端也要开始准备准备了， 那么我们继续分析代码，代码中比较重要的地方我会用注释的方式体现。

```c++
// must be called with mLock held
status_t AudioTrack::createTrack_l()
{
    //通过AudioSystem来获取 audioFlinger的代理对象，这样就可以通过它来实现业务需求
    const sp<IAudioFlinger>& audioFlinger = AudioSystem::get_audio_flinger();
    if (audioFlinger == 0) {
        ALOGE("Could not get audioflinger");
        return NO_INIT;
    }

    if (mDeviceCallback != 0 && mOutput != AUDIO_IO_HANDLE_NONE) {
        AudioSystem::removeAudioDeviceCallback(mDeviceCallback, mOutput);
    }
  
    //这是一个很重要的变量，output相当于服务端线程的一个索引，
    //客户端会根据这个来找到相应的音频输出线程
    audio_io_handle_t output;
    audio_stream_type_t streamType = mStreamType;
    audio_attributes_t *attr = (mStreamType == AUDIO_STREAM_DEFAULT) ? &mAttributes : NULL;

    //1. 根据JNI传来的各种参数来确定output值，其实这里调用到服务端去了
    status_t status;
    status = AudioSystem::getOutputForAttr(attr, &output,
                                           (audio_session_t)mSessionId, &streamType, mClientUid,
                                           mSampleRate, mFormat, mChannelMask,
                                           mFlags, mSelectedDeviceId, mOffloadInfo);

    //... 获取服务端 输出线程处理此类音频流 各种音频参数是多少...

    // Client decides whether the track is TIMED (see below), but can only express a preference
    // for FAST.  Server will perform additional tests.
    if ((mFlags & AUDIO_OUTPUT_FLAG_FAST) && !((
            // either of these use cases:
            // use case 1: shared buffer
            (mSharedBuffer != 0) ||
            // use case 2: callback transfer mode
            (mTransfer == TRANSFER_CALLBACK) ||
            // use case 3: obtain/release mode
            (mTransfer == TRANSFER_OBTAIN)) &&
            // matching sample rate
            (mSampleRate == mAfSampleRate))) {
        ALOGW("AUDIO_OUTPUT_FLAG_FAST denied by client; transfer %d, track %u Hz, output %u Hz",
                mTransfer, mSampleRate, mAfSampleRate);
        // once denied, do not request again if IAudioTrack is re-created
        mFlags = (audio_output_flags_t) (mFlags & ~AUDIO_OUTPUT_FLAG_FAST);
    }

    // The client's AudioTrack buffer is divided into n parts for purpose of wakeup by server, where
    //  n = 1   fast track with single buffering; nBuffering is ignored
    //  n = 2   fast track with double buffering
    //  n = 2   normal track, (including those with sample rate conversion)
    //  n >= 3  very high latency or very small notification interval (unused).
    const uint32_t nBuffering = 2;

    mNotificationFramesAct = mNotificationFramesReq;

    //frameCount的计算 ...

    // trackFlags 的设置 .... 省略

    size_t temp = frameCount;   // temp may be replaced by a revised value of frameCount,
                                // but we will still need the original value also
    int originalSessionId = mSessionId;
    //2. 调用服务端 createTrack，返回 IAudioTrack 接口，下面继续分析这个接口
    sp<IAudioTrack> track = audioFlinger->createTrack(streamType,
                                                      mSampleRate,
                                                      mFormat,
                                                      mChannelMask,
                                                      &temp,
                                                      &trackFlags,
                                                      mSharedBuffer,
                                                      output,
                                                      tid,
                                                      &mSessionId,
                                                      mClientUid,
                                                      &status);
    ALOGE_IF(originalSessionId != AUDIO_SESSION_ALLOCATE && mSessionId != originalSessionId,
            "session ID changed from %d to %d", originalSessionId, mSessionId);

    if (status != NO_ERROR) {
        ALOGE("AudioFlinger could not create track, status: %d", status);
        goto release;
    }
    ALOG_ASSERT(track != 0);

    // AudioFlinger now owns the reference to the I/O handle,
    // so we are no longer responsible for releasing it.

    //获取音频流控制块，IMemory是一个对共享内存操作的跨进程操作接口
    sp<IMemory> iMem = track->getCblk();
    if (iMem == 0) {
        ALOGE("Could not get control block");
        return NO_INIT;
    }
    void *iMemPointer = iMem->pointer();
    if (iMemPointer == NULL) {
        ALOGE("Could not get control block pointer");
        return NO_INIT;
    }
    // invariant that mAudioTrack != 0 is true only after set() returns successfully
    if (mAudioTrack != 0) {
        IInterface::asBinder(mAudioTrack)->unlinkToDeath(mDeathNotifier, this);
        mDeathNotifier.clear();
    }
    mAudioTrack = track;
    mCblkMemory = iMem;
    IPCThreadState::self()->flushCommands();

    //将首地址强转为 audio_track_cblk_t ，这个对象以后的文章在分析，这里只要知道这个对象是对共     //享内存进行管理的结构体
    audio_track_cblk_t* cblk = static_cast<audio_track_cblk_t*>(iMemPointer);
    mCblk = cblk;
    // note that temp is the (possibly revised) value of frameCount
    if (temp < frameCount || (frameCount == 0 && temp == 0)) {
        // In current design, AudioTrack client checks and ensures frame count validity before
        // passing it to AudioFlinger so AudioFlinger should not return a different value except
        // for fast track as it uses a special method of assigning frame count.
        ALOGW("Requested frameCount %zu but received frameCount %zu", frameCount, temp);
    }
    frameCount = temp;

    mAwaitBoost = false;
    if (mFlags & AUDIO_OUTPUT_FLAG_FAST) {
        if (trackFlags & IAudioFlinger::TRACK_FAST) {
            ALOGV("AUDIO_OUTPUT_FLAG_FAST successful; frameCount %zu", frameCount);
            mAwaitBoost = true;
        } else {
            ALOGV("AUDIO_OUTPUT_FLAG_FAST denied by server; frameCount %zu", frameCount);
            // once denied, do not request again if IAudioTrack is re-created
            mFlags = (audio_output_flags_t) (mFlags & ~AUDIO_OUTPUT_FLAG_FAST);
        }
    }
    if (mFlags & AUDIO_OUTPUT_FLAG_COMPRESS_OFFLOAD) {
        if (trackFlags & IAudioFlinger::TRACK_OFFLOAD) {
            ALOGV("AUDIO_OUTPUT_FLAG_OFFLOAD successful");
        } else {
            ALOGW("AUDIO_OUTPUT_FLAG_OFFLOAD denied by server");
            mFlags = (audio_output_flags_t) (mFlags & ~AUDIO_OUTPUT_FLAG_COMPRESS_OFFLOAD);
            // FIXME This is a warning, not an error, so don't return error status
            //return NO_INIT;
        }
    }
    if (mFlags & AUDIO_OUTPUT_FLAG_DIRECT) {
        if (trackFlags & IAudioFlinger::TRACK_DIRECT) {
            ALOGV("AUDIO_OUTPUT_FLAG_DIRECT successful");
        } else {
            ALOGW("AUDIO_OUTPUT_FLAG_DIRECT denied by server");
            mFlags = (audio_output_flags_t) (mFlags & ~AUDIO_OUTPUT_FLAG_DIRECT);
            // FIXME This is a warning, not an error, so don't return error status
            //return NO_INIT;
        }
    }
    // Make sure that application is notified with sufficient margin before underrun
    if (mSharedBuffer == 0 && audio_is_linear_pcm(mFormat)) {
        // Theoretically double-buffering is not required for fast tracks,
        // due to tighter scheduling.  But in practice, to accommodate kernels with
        // scheduling jitter, and apps with computation jitter, we use double-buffering
        // for fast tracks just like normal streaming tracks.
        if (mNotificationFramesAct == 0 || mNotificationFramesAct > frameCount / nBuffering) {
            mNotificationFramesAct = frameCount / nBuffering;
        }
    }

    // We retain a copy of the I/O handle, but don't own the reference
    mOutput = output;
    mRefreshRemaining = true;

    // Starting address of buffers in shared memory.  If there is a shared buffer, buffers
    // is the value of pointer() for the shared buffer, otherwise buffers points
    // immediately after the control block.  This address is for the mapping within client
    // address space.  AudioFlinger::TrackBase::mBuffer is for the server address space.
    void* buffers;
    if (mSharedBuffer == 0) {
        buffers = cblk + 1;
    } else {
        buffers = mSharedBuffer->pointer();
        if (buffers == NULL) {
            ALOGE("Could not get buffer pointer");
            return NO_INIT;
        }
    }

    mAudioTrack->attachAuxEffect(mAuxEffectId);
    // FIXME doesn't take into account speed or future sample rate changes (until restoreTrack)
    // FIXME don't believe this lie
    mLatency = mAfLatency + (1000*frameCount) / mSampleRate;

    mFrameCount = frameCount;
    // If IAudioTrack is re-created, don't let the requested frameCount
    // decrease.  This can confuse clients that cache frameCount().
    if (frameCount > mReqFrameCount) {
        mReqFrameCount = frameCount;
    }

    // reset server position to 0 as we have new cblk.
    mServer = 0;

    // 记住当 mSharedBuffer == 0 时时Stream模式
    if (mSharedBuffer == 0) {
        mStaticProxy.clear();
        mProxy = new AudioTrackClientProxy(cblk, buffers, frameCount, mFrameSize);
    } else {
        mStaticProxy = new StaticAudioTrackClientProxy(cblk, buffers, frameCount, mFrameSize);
        mProxy = mStaticProxy;
    }

    //通过mProxy设置音频的相关参数的初始值

    return status;
}
```

上面代码有两个地方比较重要，一个是通过AudioSystem如何获取到output的值的，另一个就是audioFlinger->createTrack()到底做了什么。

```c++
status_t AudioSystem::getOutputForAttr(const audio_attributes_t *attr,
                                        audio_io_handle_t *output,
                                        audio_session_t session,
                                        audio_stream_type_t *stream,
                                        uid_t uid,
                                        uint32_t samplingRate,
                                        audio_format_t format,
                                        audio_channel_mask_t channelMask,
                                        audio_output_flags_t flags,
                                        audio_port_handle_t selectedDeviceId,
                                        const audio_offload_info_t *offloadInfo)
{
    const sp<IAudioPolicyService>& aps = AudioSystem::get_audio_policy_service();
    if (aps == 0) return NO_INIT;
    return aps->getOutputForAttr(attr, output, session, stream, uid,
                                 samplingRate, format, channelMask,
                                 flags, selectedDeviceId, offloadInfo);
}
```

其实AudioSystem是通过binder的IPC操作，调用服务端 AudioPolicyService的接口函数，这里我们先不做过多的分析，等分析到Server端，我们优先来阅读这块的逻辑，到底做了什么。并且还有个疑问，为什么JNI传来的音频参数就能确定服务端音频播放线程的索引？我们先带着对服务端的逻辑的疑问，先继续分析代码。再来看看第二部，其实这里还是调用了服务端AudioFlinger的远程接口函数，我们先不分析，先来看看它返回的IAudioTrack究竟有哪些函数。

```c++
class IAudioTrack : public IInterface
{
public:
    DECLARE_META_INTERFACE(AudioTrack);

    /* Get this track's control block */
    virtual sp<IMemory> getCblk() const = 0;

    /* After it's created the track is not active. Call start() to
     * make it active.
     */
    virtual status_t    start() = 0;

    /* Stop a track. If set, the callback will cease being called and
     * obtainBuffer will return an error. Buffers that are already released
     * will continue to be processed, unless/until flush() is called.
     */
    virtual void        stop() = 0;

    /* Flush a stopped or paused track. All pending/released buffers are discarded.
     * This function has no effect if the track is not stopped or paused.
     */
    virtual void        flush() = 0;

    /* Pause a track. If set, the callback will cease being called and
     * obtainBuffer will return an error. Buffers that are already released
     * will continue to be processed, unless/until flush() is called.
     */
    virtual void        pause() = 0;

    /* Attach track auxiliary output to specified effect. Use effectId = 0
     * to detach track from effect.
     */
    virtual status_t    attachAuxEffect(int effectId) = 0;


    /* Allocate a shared memory buffer suitable for holding timed audio
       samples */
    virtual status_t    allocateTimedBuffer(size_t size,
                                            sp<IMemory>* buffer) = 0;

    /* Queue a buffer obtained via allocateTimedBuffer for playback at the given
       timestamp */
    virtual status_t    queueTimedBuffer(const sp<IMemory>& buffer,
                                         int64_t pts) = 0;

    /* Define the linear transform that will be applied to the timestamps
       given to queueTimedBuffer (which are expressed in media time).
       Target specifies whether this transform converts media time to local time
       or Tungsten time. The values for target are defined in AudioTrack.h */
    virtual status_t    setMediaTimeTransform(const LinearTransform& xform,
                                              int target) = 0;

    /* Send parameters to the audio hardware */
    virtual status_t    setParameters(const String8& keyValuePairs) = 0;

    /* Return NO_ERROR if timestamp is valid.  timestamp is undefined otherwise. */
    virtual status_t    getTimestamp(AudioTimestamp& timestamp) = 0;

    /* Signal the playback thread for a change in control block */
    virtual void        signal() = 0;
};

// ----------------------------------------------------------------------------

class BnAudioTrack : public BnInterface<IAudioTrack>
{
public:
    virtual status_t    onTransact( uint32_t code,
                                    const Parcel& data,
                                    Parcel* reply,
                                    uint32_t flags = 0);
};
```

说实话，确实不太懂。似乎方法挺杂的，从注释上来看，前面的一些方法对Track对象进行的一些操作，后面有对HAL层的调用，有对线程的控制，那么Track又是什么呢？一头雾水，看来只有分析到Server端，我们才会解开这些谜团了。

## 2. Server端的初始化工作

Server端最核心的类无非就是AudioFlinger.cpp 和 AudioPolicyService.cpp了，上文也提及过，那么他们所在源码的路径在：

***./frameworks/av/services/audioflinger/AudioFlinger.cpp***

***./frameworks/av/services/audiopolicy/service/AudioPolicyService.cpp***

AudioFlinger和AudioPolicyService是运行于media_server进程中，用ps命令就可以看到。我们看看服务端的最初始的地方：

```c++
int main(int argc __unused, char** argv)
{
        //... 不相关的代码
        InitializeIcuOrDie();
        sp<ProcessState> proc(ProcessState::self());
        //获取到 ServiceManager 的 proxy
        sp<IServiceManager> sm = defaultServiceManager();
        ALOGI("ServiceManager: %p", sm.get());
        //初始化 AudioFlinger
        AudioFlinger::instantiate();
        MediaPlayerService::instantiate();
        ResourceManagerService::instantiate();
        CameraService::instantiate();
        //初始化 AudioPolicyService
        AudioPolicyService::instantiate();
        SoundTriggerHwService::instantiate();
        RadioService::instantiate();
        registerExtensions();
        //启动服务端的线程池
        ProcessState::self()->startThreadPool();
        IPCThreadState::self()->joinThreadPool();
    }
}
```

这些代码都是Native层很普遍的写法，没什么好讲的，就是初始化服务，然后添加到ServiceManager中去，这些都不是核心的地方，我们下面来看看AudioFlinger的初始化流程。

### 2.1. AudioFlinger的初始化工作

先从AudioFlinger.cpp开始，从第一节得知客户端会调用到AudioFlinger的createTrack()方法，并且返回了IAudioTrack对象，我们就来看看源码：

```c++
//AudioFlinger 的构造方法
AudioFlinger::AudioFlinger()
    : BnAudioFlinger(),
      mPrimaryHardwareDev(NULL),
      mAudioHwDevs(NULL),
      mHardwareStatus(AUDIO_HW_IDLE),
      mMasterVolume(1.0f),
      mMasterMute(false),
      mNextUniqueId(1),
      mMode(AUDIO_MODE_INVALID),
      mBtNrecIsOff(false),
      mIsLowRamDevice(true),
      mIsDeviceTypeKnown(false),
      mGlobalEffectEnableTime(0),
      mSystemReady(false)
{
    getpid_cached = getpid();
    char value[PROPERTY_VALUE_MAX];
    bool doLog = (property_get("ro.test_harness", value, "0") > 0) && (atoi(value) == 1);
    if (doLog) {
        mLogMemoryDealer = new MemoryDealer(kLogMemorySize, "LogWriters",
                MemoryHeapBase::READ_ONLY);
    }

#ifdef TEE_SINK
    (void) property_get("ro.debuggable", value, "0");
    int debuggable = atoi(value);
    int teeEnabled = 0;
    if (debuggable) {
        (void) property_get("af.tee", value, "0");
        teeEnabled = atoi(value);
    }
    // FIXME symbolic constants here
    if (teeEnabled & 1) {
        mTeeSinkInputEnabled = true;
    }
    if (teeEnabled & 2) {
        mTeeSinkOutputEnabled = true;
    }
    if (teeEnabled & 4) {
        mTeeSinkTrackEnabled = true;
    }
#endif
}

//执行 sp<AudioFlinger> 的时候调用这个方法
void AudioFlinger::onFirstRef()
{
    int rc = 0;

    Mutex::Autolock _l(mLock);

    /* TODO: move all this work into an Init() function */
    char val_str[PROPERTY_VALUE_MAX] = { 0 };
    if (property_get("ro.audio.flinger_standbytime_ms", val_str, NULL) >= 0) {
        uint32_t int_val;
        if (1 == sscanf(val_str, "%u", &int_val)) {
            mStandbyTimeInNsecs = milliseconds(int_val);
            ALOGI("Using %u mSec as standby time.", int_val);
        } else {
            mStandbyTimeInNsecs = kDefaultStandbyTimeInNsecs;
            ALOGI("Using default %u mSec as standby time.",
                    (uint32_t)(mStandbyTimeInNsecs / 1000000));
        }
    }

    mPatchPanel = new PatchPanel(this);

    mMode = AUDIO_MODE_NORMAL;
}

//客户端调用到这里.
sp<IAudioTrack> AudioFlinger::createTrack(
        audio_stream_type_t streamType,
        uint32_t sampleRate,
        audio_format_t format,
        audio_channel_mask_t channelMask,
        size_t *frameCount,
        IAudioFlinger::track_flags_t *flags,
        const sp<IMemory>& sharedBuffer,
        audio_io_handle_t output,
        pid_t tid,
        int *sessionId,
        int clientUid,
        status_t *status)
{
    sp<PlaybackThread::Track> track;
    sp<TrackHandle> trackHandle;
    sp<Client> client;
    status_t lStatus;
    int lSessionId;

   // ... 对一些逻辑的判断 ... 省略

    {
        Mutex::Autolock _l(mLock);
        //1. 根据output获取到 PlaybackThread
        PlaybackThread *thread = checkPlaybackThread_l(output);
        if (thread == NULL) {
            ALOGE("no playback thread found for output handle %d", output);
            lStatus = BAD_VALUE;
            goto Exit;
        }

        pid_t pid = IPCThreadState::self()->getCallingPid();
        client = registerPid(pid);

        //...

        //2. 调用 PlaybackThread 的 createTrack_l() 方法，返回了Track对象
        track = thread->createTrack_l(client, streamType, sampleRate, format,
                channelMask, frameCount, sharedBuffer, lSessionId, flags, tid, clientUid, &lStatus);
        LOG_ALWAYS_FATAL_IF((lStatus == NO_ERROR) && (track == 0));
        // we don't abort yet if lStatus != NO_ERROR; there is still work to be done regardless

        // move effect chain to this output thread if an effect on same session was waiting
        // for a track to be created
        if (lStatus == NO_ERROR && effectThread != NULL) {
            // no risk of deadlock because AudioFlinger::mLock is held
            Mutex::Autolock _dl(thread->mLock);
            Mutex::Autolock _sl(effectThread->mLock);
            moveEffectChain_l(lSessionId, effectThread, thread, true);
        }

    //...

    if (lStatus != NO_ERROR) {
        // remove local strong reference to Client before deleting the Track so that the
        // Client destructor is called by the TrackBase destructor with mClientLock held
        // Don't hold mClientLock when releasing the reference on the track as the
        // destructor will acquire it.
        {
            Mutex::Autolock _cl(mClientLock);
            client.clear();
        }
        track.clear();
        goto Exit;
    }

    // 3. 返回的TrackHandle是对Track对象基于Binder的封装，让其拥有跨进程的传输的能力
    trackHandle = new TrackHandle(track);

Exit:
    *status = lStatus;
    return trackHandle;
}
```

从上面的代码看到，AudioFlinger初始化流程中，最重要的事情都是在createTrack()方法中，我们一个问题一个问题来分析，首先如何根据output获取到PlaybackThread：

```c++
// checkPlaybackThread_l() must be called with AudioFlinger::mLock held
AudioFlinger::PlaybackThread *AudioFlinger::checkPlaybackThread_l(audio_io_handle_t output) const
{
    return mPlaybackThreads.valueFor(output).get();
}

//在AudioFlinger.h 中找到对mPlaybackThreads的定义
DefaultKeyedVector< audio_io_handle_t, sp<PlaybackThread> >  mPlaybackThreads;
```

现在我们知道是从mPlaybackThreads对象中获取到播放线程，那么什么时候创建这些线程，什么时候添加进去的呢？还是得继续往后看。我们再来分析第二步，调用thread->createTrack_l()方法，继续看：

```c++
// PlaybackThread::createTrack_l() must be called with AudioFlinger::mLock held
sp<AudioFlinger::PlaybackThread::Track> AudioFlinger::PlaybackThread::createTrack_l(
        const sp<AudioFlinger::Client>& client,
        audio_stream_type_t streamType,
        uint32_t sampleRate,
        audio_format_t format,
        audio_channel_mask_t channelMask,
        size_t *pFrameCount,
        const sp<IMemory>& sharedBuffer,
        int sessionId,
        IAudioFlinger::track_flags_t *flags,
        pid_t tid,
        int uid,
        status_t *status)
{
    size_t frameCount = *pFrameCount;
    sp<Track> track;
    status_t lStatus;

    bool isTimed = (*flags & IAudioFlinger::TRACK_TIMED) != 0;

    //... 省略一大串代码 ... 这里就不过多分析了，有兴趣可自行研究

    lStatus = initCheck();
    if (lStatus != NO_ERROR) {
        ALOGE("createTrack_l() audio driver not initialized");
        goto Exit;
    }

    { // scope for mLock
        Mutex::Autolock _l(mLock);

        // all tracks in same audio session must share the same routing strategy otherwise
        // conflicts will happen when tracks are moved from one output to another by audio policy
  
        if (!isTimed) {
            track = new Track(this, client, streamType, sampleRate, format,
                              channelMask, frameCount, NULL, sharedBuffer,
                              sessionId, uid, *flags, TrackBase::TYPE_DEFAULT);
        } else {
            track = TimedTrack::create(this, client, streamType, sampleRate, format,
                    channelMask, frameCount, sharedBuffer, sessionId, uid);
        }

        //...
     
        mTracks.add(track);

        //...

    return track;
}
```

这里的代码很多，细节也同样很多，我们只注重核心代码走向。最重要的就是创建了Track对象，并添加到mTracks数组中去。所以，这个Track对象非常的重要，那么它究竟做了什么呢？

```c++
// TrackBase constructor must be called with AudioFlinger::mLock held
AudioFlinger::ThreadBase::TrackBase::TrackBase(
            ThreadBase *thread,
            const sp<Client>& client,
            uint32_t sampleRate,
            audio_format_t format,
            audio_channel_mask_t channelMask,
            size_t frameCount,
            void *buffer,
            int sessionId,
            int clientUid,
            IAudioFlinger::track_flags_t flags,
            bool isOut,
            alloc_type alloc,
            track_type type)
    :   RefBase(),
        mThread(thread),
        mClient(client),
        mCblk(NULL),
        // mBuffer
        mState(IDLE),
        mSampleRate(sampleRate),
        mFormat(format),
        mChannelMask(channelMask),
        mChannelCount(isOut ?
                audio_channel_count_from_out_mask(channelMask) :
                audio_channel_count_from_in_mask(channelMask)),
        mFrameSize(audio_is_linear_pcm(format) ?
                mChannelCount * audio_bytes_per_sample(format) : sizeof(int8_t)),
        mFrameCount(frameCount),
        mSessionId(sessionId),
        mFlags(flags),
        mIsOut(isOut),
        mServerProxy(NULL),
        mId(android_atomic_inc(&nextTrackId)),
        mTerminated(false),
        mType(type),
        mThreadIoHandle(thread->id())
{
    //...

    // ALOGD("Creating track with %d buffers @ %d bytes", bufferCount, bufferSize);
    size_t size = sizeof(audio_track_cblk_t);
    size_t bufferSize = (buffer == NULL ? roundup(frameCount) : frameCount) * mFrameSize;
    //计算共享内存的大小
    if (buffer == NULL && alloc == ALLOC_CBLK) {
        size += bufferSize;
    }

    if (client != 0) {
        mCblkMemory = client->heap()->allocate(size);
        if (mCblkMemory == 0 ||
                (mCblk = static_cast<audio_track_cblk_t *>(mCblkMemory->pointer())) == NULL) {
            ALOGE("not enough memory for AudioTrack size=%u", size);
            client->heap()->dump("AudioTrack");
            mCblkMemory.clear();
            return;
        }
    } else {
        // Stream 模式走的是这里
        mCblk = (audio_track_cblk_t *) new uint8_t[size];
        // assume mCblk != NULL
    }

    // construct the shared structure in-place.
    if (mCblk != NULL) {
        new(mCblk) audio_track_cblk_t();
        switch (alloc) {
        case ALLOC_READONLY: {
            const sp<MemoryDealer> roHeap(thread->readOnlyHeap());
            if (roHeap == 0 ||
                    (mBufferMemory = roHeap->allocate(bufferSize)) == 0 ||
                    (mBuffer = mBufferMemory->pointer()) == NULL) {
                ALOGE("not enough memory for read-only buffer size=%zu", bufferSize);
                if (roHeap != 0) {
                    roHeap->dump("buffer");
                }
                mCblkMemory.clear();
                mBufferMemory.clear();
                return;
            }
            memset(mBuffer, 0, bufferSize);
            } break;
        case ALLOC_PIPE:
            mBufferMemory = thread->pipeMemory();
            // mBuffer is the virtual address as seen from current process (mediaserver),
            // and should normally be coming from mBufferMemory->pointer().
            // However in this case the TrackBase does not reference the buffer directly.
            // It should references the buffer via the pipe.
            // Therefore, to detect incorrect usage of the buffer, we set mBuffer to NULL.
            mBuffer = NULL;
            break;
        case ALLOC_CBLK:
            // clear all buffers
            if (buffer == NULL) {
                //我们看这里，计算出audio_track_cblk_t的大小，然后初始化内存
                mBuffer = (char*)mCblk + sizeof(audio_track_cblk_t);
                memset(mBuffer, 0, bufferSize);
            } else {
                mBuffer = buffer;
#if 0
                mCblk->mFlags = CBLK_FORCEREADY;    // FIXME hack, need to fix the track ready logic
#endif
            }
            break;
        case ALLOC_LOCAL:
            mBuffer = calloc(1, bufferSize);
            break;
        case ALLOC_NONE:
            mBuffer = buffer;
            break;
        }

      //...

    }
}
```

 首先要注意的是这里是Track的父类TrackBase的构造方法，Track的构造方法倒没有做非常重要的事情。我们之前学习到播放音频有两种模式，一个STATIC模式，一种STREAM模式，STATIC创建共享内存是在JNI中，而STREAM模式下，创建共享内存是在服务端完成的，并且是流程是在AudioTrack->createTrack()，一直到服务端AudioFlinger创建Track对象的时候创建的。 对AudioFlinger的createTrack()方法中，第三步就是将生成的Track对象进一步封装，生成TrackHandle对象，下面来看看它到底是什么：

```c++
    // server side of the client's IAudioTrack
    class TrackHandle : public android::BnAudioTrack {
    public:
                            TrackHandle(const sp<PlaybackThread::Track>& track);
        virtual             ~TrackHandle();
        virtual sp<IMemory> getCblk() const;
        virtual status_t    start();
        virtual void        stop();
        virtual void        flush();
        virtual void        pause();
        virtual status_t    attachAuxEffect(int effectId);
        virtual status_t    allocateTimedBuffer(size_t size,
                                                sp<IMemory>* buffer);
        virtual status_t    queueTimedBuffer(const sp<IMemory>& buffer,
                                             int64_t pts);
        virtual status_t    setMediaTimeTransform(const LinearTransform& xform,
                                                  int target);
        virtual status_t    setParameters(const String8& keyValuePairs);
        virtual status_t    getTimestamp(AudioTimestamp& timestamp);
        virtual void        signal(); // signal playback thread for a change in control block

        virtual status_t onTransact(
            uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags);

    private:
        const sp<PlaybackThread::Track> mTrack;
    };

```

我相信只要看到Google工程师对这个类的注释，你也会恍然大悟吧。你是否还记得在AudioTrack最后调用createTrack()方法，生成了 IAudioTrack 对象，没错，TrackHandle 就是它服务端的代理对象，继承了BnAudioTrack，我相信这些方法都是对Track对象的控制和管理吧。下面我们来小结下AudioFlinger的初始化工作，首先最重要的是根据Client端传来的output找到了适合它自己的播放线程，然后这个线程调用createTrack_l()方法生成了Track对象，在这个对象构造方法，构建了STREAM模式下的共享内存，然后封装成TrackHandle对象返回给Client端的AudioTrack，而这个TrackHandle对象和Client端的IAudioTrack对象就是对这个Track对象和播放线程的控制和管理的作用。

### 2.2.  AudioPolicyService的初始化工作

AudioPolicyService也是音频服务端非常重要的一个成员，从它的名字来看就知道是对音频策略相关的选择。那么，先不管别的，先从它的初始化方法看看究竟做了什么。

```c++
AudioPolicyService::AudioPolicyService()
    : BnAudioPolicyService(), mpAudioPolicyDev(NULL), mpAudioPolicy(NULL),
      mAudioPolicyManager(NULL), mAudioPolicyClient(NULL), mPhoneState(AUDIO_MODE_INVALID)
{
}

//第一次被 sp<AudioPolicyService> 的时候被执行
void AudioPolicyService::onFirstRef()
{
    char value[PROPERTY_VALUE_MAX];
    const struct hw_module_t *module;
    int forced_val;
    int rc;

    {
        Mutex::Autolock _l(mLock);

        //3. 创建了三条线程
        // start tone playback thread
        mTonePlaybackThread = new AudioCommandThread(String8("ApmTone"), this);
        // start audio commands thread
        mAudioCommandThread = new AudioCommandThread(String8("ApmAudio"), this);
        // start output activity command thread
        mOutputCommandThread = new AudioCommandThread(String8("ApmOutput"), this);

#ifdef USE_LEGACY_AUDIO_POLICY
        //...
#else
        // 没有定义上面的宏，所以我们默认走这里
        ALOGI("AudioPolicyService CSTOR in new mode");
        //1. 创建了AudioPolicyClient对象
        mAudioPolicyClient = new AudioPolicyClient(this);
        //2. 创建了AudioPolicyManager对象
        mAudioPolicyManager = createAudioPolicyManager(mAudioPolicyClient);
#endif
    }
    // load audio processing modules
    sp<AudioPolicyEffects>audioPolicyEffects = new AudioPolicyEffects();
    {
        Mutex::Autolock _l(mLock);
        mAudioPolicyEffects = audioPolicyEffects;
    }
}
```

我们先来看看AudioPolicyClient的定义，看看它有哪些函数，先不看它的实现：

```c++
    class AudioPolicyClient : public AudioPolicyClientInterface
    {
     public:
        AudioPolicyClient(AudioPolicyService *service) : mAudioPolicyService(service) {}
        virtual ~AudioPolicyClient() {}

        //
        // Audio HW module functions
        //

        // loads a HW module.
        virtual audio_module_handle_t loadHwModule(const char *name);

        //
        // Audio output Control functions
        //

        // opens an audio output with the requested parameters. The parameter values can indicate to use the default values
        // in case the audio policy manager has no specific requirements for the output being opened.
        // When the function returns, the parameter values reflect the actual values used by the audio hardware output stream.
        // The audio policy manager can check if the proposed parameters are suitable or not and act accordingly.
        virtual status_t openOutput(audio_module_handle_t module,
                                    audio_io_handle_t *output,
                                    audio_config_t *config,
                                    audio_devices_t *devices,
                                    const String8& address,
                                    uint32_t *latencyMs,
                                    audio_output_flags_t flags);
        // creates a special output that is duplicated to the two outputs passed as arguments. The duplication is performed by
        // a special mixer thread in the AudioFlinger.
        virtual audio_io_handle_t openDuplicateOutput(audio_io_handle_t output1, audio_io_handle_t output2);
        // closes the output stream
        virtual status_t closeOutput(audio_io_handle_t output);
        // suspends the output. When an output is suspended, the corresponding audio hardware output stream is placed in
        // standby and the AudioTracks attached to the mixer thread are still processed but the output mix is discarded.
        virtual status_t suspendOutput(audio_io_handle_t output);
        // restores a suspended output.
        virtual status_t restoreOutput(audio_io_handle_t output);

        //
        // Audio input Control functions
        //

        // opens an audio input
        virtual audio_io_handle_t openInput(audio_module_handle_t module,
                                            audio_io_handle_t *input,
                                            audio_config_t *config,
                                            audio_devices_t *devices,
                                            const String8& address,
                                            audio_source_t source,
                                            audio_input_flags_t flags);
        // closes an audio input
        virtual status_t closeInput(audio_io_handle_t input);
        //
        // misc control functions
        //

        // set a stream volume for a particular output. For the same user setting, a given stream type can have different volumes
        // for each output (destination device) it is attached to.
        virtual status_t setStreamVolume(audio_stream_type_t stream, float volume, audio_io_handle_t output, int delayMs = 0);

        // invalidate a stream type, causing a reroute to an unspecified new output
        virtual status_t invalidateStream(audio_stream_type_t stream);

        // function enabling to send proprietary informations directly from audio policy manager to audio hardware interface.
        virtual void setParameters(audio_io_handle_t ioHandle, const String8& keyValuePairs, int delayMs = 0);
        // function enabling to receive proprietary informations directly from audio hardware interface to audio policy manager.
        virtual String8 getParameters(audio_io_handle_t ioHandle, const String8& keys);

        // request the playback of a tone on the specified stream: used for instance to replace notification sounds when playing
        // over a telephony device during a phone call.
        virtual status_t startTone(audio_policy_tone_t tone, audio_stream_type_t stream);
        virtual status_t stopTone();

        // set down link audio volume.
        virtual status_t setVoiceVolume(float volume, int delayMs = 0);

        //... 很多函数 ... 省略


     private:
        AudioPolicyService *mAudioPolicyService;
    };

```

首先要注意的是它实现了AudioPolicyClientInterface接口，然后它有个AudioPolicyService的私有成员变量。看它的函数，有和HAL层交互的，loadHwModule()方法，注释也写的很清楚了，加载HAL层的module，了解HAL层的小伙伴们肯定知道，HAL层的对象最外层的结构体就是以Module的形式提供的，例如：

```c
struct audio_module HAL_MODULE_INFO_SYM = {
    .common = {
         .tag = HARDWARE_MODULE_TAG,
         .module_api_version = AUDIO_MODULE_API_VERSION_0_1,
         .hal_api_version = HARDWARE_HAL_API_VERSION,
         .id = AUDIO_HARDWARE_MODULE_ID,
         .name = "Audio HW HAL",
         .author = "The Android Open Source Project",
          //hal层所有的方法就在这里了
         .methods = &hal_module_methods,
    },  
};
```

 从其他的方法来看，也蛮符合它的名字的，为客户端提供的一些接口，例如openOutput()，setParams()，setVolume() 等等，它的实现类路径在：

***./frameworks/av/services/audiopolicy/service/AudioPolicyClientImpl.cpp***

我们再来看看它创建的AudioPolicyManager到底做了什么事情。

```c++
// AudioPolicyFactory.cpp.... 
extern "C" AudioPolicyInterface* createAudioPolicyManager(
        AudioPolicyClientInterface *clientInterface)
{
    return new AudioPolicyManager(clientInterface);
}

// ... AudioPolicyManager ...
AudioPolicyManager::AudioPolicyManager(AudioPolicyClientInterface *clientInterface)
    :
#ifdef AUDIO_POLICY_TEST
    Thread(false),
#endif //AUDIO_POLICY_TEST
    mLimitRingtoneVolume(false), mLastVoiceVolume(-1.0f),
    mA2dpSuspended(false),
    mSpeakerDrcEnabled(false),
    mAudioPortGeneration(1),
    mBeaconMuteRefCount(0),
    mBeaconPlayingRefCount(0),
    mBeaconMuted(false),
    mTtsOutputAvailable(false)
{
    // 1. Engine 是和音频策略相关的类，先简单看下它的结构，以后详细学习它所发挥的功能
    audio_policy::EngineInstance *engineInstance = audio_policy::EngineInstance::getInstance();
    if (!engineInstance) {
        ALOGE("%s:  Could not get an instance of policy engine", __FUNCTION__);
        return;
    }
    // Retrieve the Policy Manager Interface
    mEngine = engineInstance->queryInterface<AudioPolicyManagerInterface>();
    if (mEngine == NULL) {
        ALOGE("%s: Failed to get Policy Engine Interface", __FUNCTION__);
        return;
    }
    mEngine->setObserver(this);
    status_t status = mEngine->initCheck();
    ALOG_ASSERT(status == NO_ERROR, "Policy engine not initialized(err=%d)", status);

    mUidCached = getuid();
    mpClientInterface = clientInterface;

    mDefaultOutputDevice = new DeviceDescriptor(AUDIO_DEVICE_OUT_SPEAKER);
    //2. 加载音频配置文件，我们看到先去加载厂商的音频配置文件，如果没有就加载Android默认的配置文件，
    //我们这里就直接看Android默认的配置文件
    if (ConfigParsingUtils::loadAudioPolicyConfig(AUDIO_POLICY_VENDOR_CONFIG_FILE,
                 mHwModules, mAvailableInputDevices, mAvailableOutputDevices,
                 mDefaultOutputDevice, mSpeakerDrcEnabled) != NO_ERROR) {
        if (ConfigParsingUtils::loadAudioPolicyConfig(AUDIO_POLICY_CONFIG_FILE,
                                  mHwModules, mAvailableInputDevices, mAvailableOutputDevices,
                                  mDefaultOutputDevice, mSpeakerDrcEnabled) != NO_ERROR) {
            ALOGE("could not load audio policy configuration file, setting defaults");
            //3. 看到底做了什么
            defaultAudioPolicyConfig();
        }
    }
    // mAvailableOutputDevices and mAvailableInputDevices now contain all attached devices

    // must be done after reading the policy (since conditionned by Speaker Drc Enabling)
    mEngine->initializeVolumeCurves(mSpeakerDrcEnabled);

    // open all output streams needed to access attached devices
    audio_devices_t outputDeviceTypes = mAvailableOutputDevices.types();
    audio_devices_t inputDeviceTypes = mAvailableInputDevices.types() & ~AUDIO_DEVICE_BIT_IN;
        
    //4.调用 AudioPolicyClientInterface的loadHwModule()方法，它的实现类刚才也说了，就是
    // AudioPolicyClientImpl.cpp 文件
    for (size_t i = 0; i < mHwModules.size(); i++) {
        mHwModules[i]->mHandle = mpClientInterface->loadHwModule(mHwModules[i]->mName);
        if (mHwModules[i]->mHandle == 0) {
            ALOGW("could not open HW module %s", mHwModules[i]->mName);
            continue;
        }
        // open all output streams needed to access attached devices
        // except for direct output streams that are only opened when they are actually
        // required by an app.
        // This also validates mAvailableOutputDevices list
        for (size_t j = 0; j < mHwModules[i]->mOutputProfiles.size(); j++)
        {
            const sp<IOProfile> outProfile = mHwModules[i]->mOutputProfiles[j];

            if (outProfile->mSupportedDevices.isEmpty()) {
                ALOGW("Output profile contains no device on module %s", mHwModules[i]->mName);
                continue;
            }
            if ((outProfile->mFlags & AUDIO_OUTPUT_FLAG_TTS) != 0) {
                mTtsOutputAvailable = true;
            }

            if ((outProfile->mFlags & AUDIO_OUTPUT_FLAG_DIRECT) != 0) {
                continue;
            }
            audio_devices_t profileType = outProfile->mSupportedDevices.types();
            if ((profileType & mDefaultOutputDevice->type()) != AUDIO_DEVICE_NONE) {
                profileType = mDefaultOutputDevice->type();
            } else {
                // chose first device present in mSupportedDevices also part of
                // outputDeviceTypes
                for (size_t k = 0; k  < outProfile->mSupportedDevices.size(); k++) {
                    profileType = outProfile->mSupportedDevices[k]->type();
                    if ((profileType & outputDeviceTypes) != 0) {
                        break;
                    }
                }
            }
            if ((profileType & outputDeviceTypes) == 0) {
                continue;
            }
            sp<SwAudioOutputDescriptor> outputDesc = new SwAudioOutputDescriptor(outProfile,
                                                                                 mpClientInterface);

            outputDesc->mDevice = profileType;
            audio_config_t config = AUDIO_CONFIG_INITIALIZER;
            config.sample_rate = outputDesc->mSamplingRate;
            config.channel_mask = outputDesc->mChannelMask;
            config.format = outputDesc->mFormat;
            audio_io_handle_t output = AUDIO_IO_HANDLE_NONE;
            //5. openOutput 是否是创建输出线程？
            status_t status = mpClientInterface->openOutput(outProfile->getModuleHandle(),
                                                            &output,
                                                            &config,
                                                            &outputDesc->mDevice,
                                                            String8(""),
                                                            &outputDesc->mLatency,
                                                            outputDesc->mFlags);

            if (status != NO_ERROR) {
                ALOGW("Cannot open output stream for device %08x on hw module %s",
                      outputDesc->mDevice,
                      mHwModules[i]->mName);
            } else {
                outputDesc->mSamplingRate = config.sample_rate;
                outputDesc->mChannelMask = config.channel_mask;
                outputDesc->mFormat = config.format;

                for (size_t k = 0; k  < outProfile->mSupportedDevices.size(); k++) {
                    audio_devices_t type = outProfile->mSupportedDevices[k]->type();
                    ssize_t index =
                            mAvailableOutputDevices.indexOf(outProfile->mSupportedDevices[k]);
                    // give a valid ID to an attached device once confirmed it is reachable
                    if (index >= 0 && !mAvailableOutputDevices[index]->isAttached()) {
                        mAvailableOutputDevices[index]->attach(mHwModules[i]);
                    }
                }
                if (mPrimaryOutput == 0 &&
                        outProfile->mFlags & AUDIO_OUTPUT_FLAG_PRIMARY) {
                    mPrimaryOutput = outputDesc;
                }
                addOutput(output, outputDesc);
                setOutputDevice(outputDesc,
                                outputDesc->mDevice,
                                true);
            }
        }
     
        //... 省略大部分代码，和音频input相关的，这里先不去学习了。
        
        
    // make sure all attached devices have been allocated a unique ID
    for (size_t i = 0; i  < mAvailableOutputDevices.size();) {
        if (!mAvailableOutputDevices[i]->isAttached()) {
            ALOGW("Input device %08x unreachable", mAvailableOutputDevices[i]->type());
            mAvailableOutputDevices.remove(mAvailableOutputDevices[i]);
            continue;
        }
        // The device is now validated and can be appended to the available devices of the engine
        mEngine->setDeviceConnectionState(mAvailableOutputDevices[i],
                                          AUDIO_POLICY_DEVICE_STATE_AVAILABLE);
        i++;
    }
   
    // ... 和input相关的
        
    // make sure default device is reachable
    if (mAvailableOutputDevices.indexOf(mDefaultOutputDevice) < 0) {
        ALOGE("Default device %08x is unreachable", mDefaultOutputDevice->type());
    }

    ALOGE_IF((mPrimaryOutput == 0), "Failed to open primary output");

    //6. 更新输出设备
    updateDevicesAndOutputs();
}

```

这个函数的内部做了很多事情，我们逐步的去学习，先来了解下第一步到底做了什么：

```c++
Engine *EngineInstance::getEngine() const
{
    static Engine engine;
    return &engine;
}

template <>
AudioPolicyManagerInterface *EngineInstance::queryInterface() const
{
    return getEngine()->queryInterface<AudioPolicyManagerInterface>();
}

//下面是Engine的结构体
class Engine
{
public:
    Engine();
    virtual ~Engine();

    template <class RequestedInterface>
    RequestedInterface *queryInterface();

private:
    /// Interface members
    class ManagerInterfaceImpl : public AudioPolicyManagerInterface
    {
    public:
        ManagerInterfaceImpl(Engine *policyEngine)
            : mPolicyEngine(policyEngine) {}
        
        virtual audio_devices_t getDeviceForStrategy(routing_strategy stategy) const;
        virtual routing_strategy getStrategyForStream(audio_stream_type_t stream)
        {
            return mPolicyEngine->getPropertyForKey<routing_strategy, audio_stream_type_t>(stream);
        }
        virtual routing_strategy getStrategyForUsage(audio_usage_t usage);

        //... 省略很多个函数 ...
    private:
        Engine *mPolicyEngine;
    } mManagerInterface;

```

我们先不去看它的实现，后面会单独的去学习这块，不然篇幅就太长了。我们这里可以简单的看下，都是和策略相关的，根据 stream_type 获取策略 routing_strategy ，然后又根据 routing_strategy 去获取 audio_devices_t，这个对象就是播放音频的硬件设备。好了，我们大概了解了这个类的作用，看来看看第二步干了啥，我们直接去看它加载的音频配置文件是什么，在路径

***./frameworks/av/services/audiopolicy/common/managerdefinitons/include/audio_policy_conf.h***

下，定义了加载文件的宏定义：

```c++
#define AUDIO_POLICY_CONFIG_FILE "/system/etc/audio_policy.conf"
#define AUDIO_POLICY_VENDOR_CONFIG_FILE "/vendor/etc/audio_policy.conf"
```

我们直接去查看 /system/etc/audio_policy.conf，这个文件的位置位于

***./framework/av/services/audiopolicy/audio_policy.conf***

我们来看下它的结构：

```
audio_hw_modules {
  // primary 代表了一个 HwModule
  primary {
    //当前系统支持的音频输入输出设备以及默认的输入输出设备
    global_configuration {
      attached_output_devices AUDIO_DEVICE_OUT_SPEAKER
      default_output_device AUDIO_DEVICE_OUT_SPEAKER
      attached_input_devices AUDIO_DEVICE_IN_BUILTIN_MIC
      audio_hal_version 3.0
    }
    devices {
      speaker {
        type AUDIO_DEVICE_OUT_SPEAKER
        gains {
          gain_1 {
            mode AUDIO_GAIN_MODE_JOINT
            min_value_mB -8400
            max_value_mB 4000
            default_value_mB 0
            step_value_mB 100
          }
        }
      }
    }
    //对输出设备的描述
    outputs {
      //这里指的是IOProfile
      primary {
        sampling_rates 48000
        channel_masks AUDIO_CHANNEL_OUT_STEREO
        formats AUDIO_FORMAT_PCM_16_BIT
        devices speaker
        flags AUDIO_OUTPUT_FLAG_PRIMARY
      }
    }
    inputs {
      primary {
        sampling_rates 8000|16000
        channel_masks AUDIO_CHANNEL_IN_MONO
        formats AUDIO_FORMAT_PCM_16_BIT
        devices AUDIO_DEVICE_IN_BUILTIN_MIC
      }
    }
  }
  r_submix {
    global_configuration {
      attached_input_devices AUDIO_DEVICE_IN_REMOTE_SUBMIX
      audio_hal_version 2.0
    }
    outputs {
      submix {
        sampling_rates 48000
        channel_masks AUDIO_CHANNEL_OUT_STEREO
        formats AUDIO_FORMAT_PCM_16_BIT
        devices AUDIO_DEVICE_OUT_REMOTE_SUBMIX
      }
    }
    inputs {
      submix {
        sampling_rates 48000
        channel_masks AUDIO_CHANNEL_IN_STEREO
        formats AUDIO_FORMAT_PCM_16_BIT
        devices AUDIO_DEVICE_IN_REMOTE_SUBMIX
      }
    }
  }
}
```

上面也对文档中的内容加了一些注释，他们每个HwModule会生成对应的硬件抽象库，例如primary，就会在设备的system/lib/hw/目录下生成 audio.primary.default.so文件，以供Native层程序调用。对这个文件的解析我们就不看了，我们看看defaultAudioPolicyConfig()方法做的事情。

```c++
void AudioPolicyManager::defaultAudioPolicyConfig(void)
{
    sp<HwModule> module;
    sp<IOProfile> profile;
    sp<DeviceDescriptor> defaultInputDevice =
                    new DeviceDescriptor(AUDIO_DEVICE_IN_BUILTIN_MIC);
    mAvailableOutputDevices.add(mDefaultOutputDevice);
    mAvailableInputDevices.add(defaultInputDevice);

    module = new HwModule("primary");

    profile = new IOProfile(String8("primary"), AUDIO_PORT_ROLE_SOURCE);
    profile->attach(module);
    profile->mSamplingRates.add(44100);
    profile->mFormats.add(AUDIO_FORMAT_PCM_16_BIT);
    profile->mChannelMasks.add(AUDIO_CHANNEL_OUT_STEREO);
    profile->mSupportedDevices.add(mDefaultOutputDevice);
    profile->mFlags = AUDIO_OUTPUT_FLAG_PRIMARY;
    module->mOutputProfiles.add(profile);

    profile = new IOProfile(String8("primary"), AUDIO_PORT_ROLE_SINK);
    profile->attach(module);
    profile->mSamplingRates.add(8000);
    profile->mFormats.add(AUDIO_FORMAT_PCM_16_BIT);
    profile->mChannelMasks.add(AUDIO_CHANNEL_IN_MONO);
    profile->mSupportedDevices.add(defaultInputDevice);
    module->mInputProfiles.add(profile);

    mHwModules.add(module);
}
```

这里会将conf文件中的每个硬件接口封装成 HwModule和 IOProfile 对象，至于这两个对象所代表的内容也在配置文件中描述了。我们继续看看系统如何加载HAL层的动态so库：

```c++
/* implementation of the client interface from the policy manager */
//AudioPolicyClientImpl.cpp文件
audio_module_handle_t AudioPolicyService::AudioPolicyClient::loadHwModule(const char *name)
{
    sp<IAudioFlinger> af = AudioSystem::get_audio_flinger();
    if (af == 0) {
        ALOGW("%s: could not get AudioFlinger", __func__);
        return 0;
    }
    //调用了 AudioFlinger 的 loadHwModule()方法
    return af->loadHwModule(name);
}
```

结果又回到了AudioFlinger中去处理：

```c++
audio_module_handle_t AudioFlinger::loadHwModule(const char *name)
{
    if (name == NULL) {
        return 0;
    }
    if (!settingsAllowed()) {
        return 0;
    }
    Mutex::Autolock _l(mLock);
    return loadHwModule_l(name);
}

// loadHwModule_l() must be called with AudioFlinger::mLock held
audio_module_handle_t AudioFlinger::loadHwModule_l(const char *name)
{
    //如果已经加载过了就不加载了...
    for (size_t i = 0; i < mAudioHwDevs.size(); i++) {
        if (strncmp(mAudioHwDevs.valueAt(i)->moduleName(), name, strlen(name)) == 0) {
            ALOGW("loadHwModule() module %s already loaded", name);
            return mAudioHwDevs.keyAt(i);
        }
    }
   
    //最后生成的对象是这个
    audio_hw_device_t *dev;

    //加载动态库
    int rc = load_audio_interface(name, &dev);
    if (rc) {
        ALOGI("loadHwModule() error %d loading module %s ", rc, name);
        return 0;
    }

    //.... 省略很多代码...
    
    audio_module_handle_t handle = nextUniqueId();
    mAudioHwDevs.add(handle, new AudioHwDevice(handle, name, dev, flags));

    ALOGI("loadHwModule() Loaded %s audio interface from %s (%s) handle %d",
          name, dev->common.module->name, dev->common.module->id, handle);

    return handle;

}

static int load_audio_interface(const char *if_name, audio_hw_device_t **dev)
{
    const hw_module_t *mod;
    int rc;

    //找到HAL层的Module
    rc = hw_get_module_by_class(AUDIO_HARDWARE_MODULE_ID, if_name, &mod);
    ALOGE_IF(rc, "%s couldn't load audio hw module %s.%s (%s)", __func__,
                 AUDIO_HARDWARE_MODULE_ID, if_name, strerror(-rc));
    if (rc) {
        goto out;
    }
    //打开设备
    rc = audio_hw_device_open(mod, dev);
    ALOGE_IF(rc, "%s couldn't open audio hw device in %s.%s (%s)", __func__,
                 AUDIO_HARDWARE_MODULE_ID, if_name, strerror(-rc));
    if (rc) {
        goto out;
    }
    if ((*dev)->common.version < AUDIO_DEVICE_API_VERSION_MIN) {
        ALOGE("%s wrong audio hw device version %04x", __func__, (*dev)->common.version);
        rc = BAD_VALUE;
        goto out;
    }
    return 0;

out:
    *dev = NULL;
    return rc;
}
```

load_audio_interface()方法中更多的是HAL层的接口函数，这里我们就不继续往下跟了，我们只需要知道它动态加载了so库，然后打开设备，返回到我们Native层的就是audio_hw_device_t 结构对象，它每个函数都与HAL层函数库的每个函数一一对应。我们还是来看看这个对象吧，因为它非常的重要。这个结构体的定义路径位于：

***./hardware/libhardware/include/hardware/audio.h***

```c
typedef struct audio_hw_device audio_hw_device_t;

struct audio_hw_device {
    /**
     * Common methods of the audio device.  This *must* be the first member of audio_hw_device
     * as users of this structure will cast a hw_device_t to audio_hw_device pointer in contexts
     * where it's known the hw_device_t references an audio_hw_device.
     */
    struct hw_device_t common;

    /**
     * used by audio flinger to enumerate what devices are supported by
     * each audio_hw_device implementation.
     *
     * Return value is a bitmask of 1 or more values of audio_devices_t
     *
     * NOTE: audio HAL implementations starting with
     * AUDIO_DEVICE_API_VERSION_2_0 do not implement this function.
     * All supported devices should be listed in audio_policy.conf
     * file and the audio policy manager must choose the appropriate
     * audio module based on information in this file.
     */
    uint32_t (*get_supported_devices)(const struct audio_hw_device *dev);

    /**
     * check to see if the audio hardware interface has been initialized.
     * returns 0 on success, -ENODEV on failure.
     */
    int (*init_check)(const struct audio_hw_device *dev);

    /** set the audio volume of a voice call. Range is between 0.0 and 1.0 */
    int (*set_voice_volume)(struct audio_hw_device *dev, float volume);

    /**
     * set the audio volume for all audio activities other than voice call.
     * Range between 0.0 and 1.0. If any value other than 0 is returned,
     * the software mixer will emulate this capability.
     */
    int (*set_master_volume)(struct audio_hw_device *dev, float volume);

    /**
     * Get the current master volume value for the HAL, if the HAL supports
     * master volume control.  AudioFlinger will query this value from the
     * primary audio HAL when the service starts and use the value for setting
     * the initial master volume across all HALs.  HALs which do not support
     * this method may leave it set to NULL.
     */
    int (*get_master_volume)(struct audio_hw_device *dev, float *volume);

    /**
     * set_mode is called when the audio mode changes. AUDIO_MODE_NORMAL mode
     * is for standard audio playback, AUDIO_MODE_RINGTONE when a ringtone is
     * playing, and AUDIO_MODE_IN_CALL when a call is in progress.
     */
    int (*set_mode)(struct audio_hw_device *dev, audio_mode_t mode);

    /* mic mute */
    int (*set_mic_mute)(struct audio_hw_device *dev, bool state);
    int (*get_mic_mute)(const struct audio_hw_device *dev, bool *state);

    /* set/get global audio parameters */
    int (*set_parameters)(struct audio_hw_device *dev, const char *kv_pairs);

    /*
     * Returns a pointer to a heap allocated string. The caller is responsible
     * for freeing the memory for it using free().
     */
    char * (*get_parameters)(const struct audio_hw_device *dev,
                             const char *keys);

    /* Returns audio input buffer size according to parameters passed or
     * 0 if one of the parameters is not supported.
     * See also get_buffer_size which is for a particular stream.
     */
    size_t (*get_input_buffer_size)(const struct audio_hw_device *dev,
                                    const struct audio_config *config);

    /** This method creates and opens the audio hardware output stream.
     * The "address" parameter qualifies the "devices" audio device type if needed.
     * The format format depends on the device type:
     * - Bluetooth devices use the MAC address of the device in the form "00:11:22:AA:BB:CC"
     * - USB devices use the ALSA card and device numbers in the form  "card=X;device=Y"
     * - Other devices may use a number or any other string.
     */

    int (*open_output_stream)(struct audio_hw_device *dev,
                              audio_io_handle_t handle,
                              audio_devices_t devices,
                              audio_output_flags_t flags,
                              struct audio_config *config,
                              struct audio_stream_out **stream_out,
                              const char *address);

    void (*close_output_stream)(struct audio_hw_device *dev,
                                struct audio_stream_out* stream_out);

    /** This method creates and opens the audio hardware input stream */
    int (*open_input_stream)(struct audio_hw_device *dev,
                             audio_io_handle_t handle,
                             audio_devices_t devices,
                             struct audio_config *config,
                             struct audio_stream_in **stream_in,
                             audio_input_flags_t flags,
                             const char *address,
                             audio_source_t source);

    void (*close_input_stream)(struct audio_hw_device *dev,
                               struct audio_stream_in *stream_in);

    /** This method dumps the state of the audio hardware */
    int (*dump)(const struct audio_hw_device *dev, int fd);

    /**
     * set the audio mute status for all audio activities.  If any value other
     * than 0 is returned, the software mixer will emulate this capability.
     */
    int (*set_master_mute)(struct audio_hw_device *dev, bool mute);

    /**
     * Get the current master mute status for the HAL, if the HAL supports
     * master mute control.  AudioFlinger will query this value from the primary
     * audio HAL when the service starts and use the value for setting the
     * initial master mute across all HALs.  HALs which do not support this
     * method may leave it set to NULL.
     */
    int (*get_master_mute)(struct audio_hw_device *dev, bool *mute);

    /**
     * Routing control
     */

    /* Creates an audio patch between several source and sink ports.
     * The handle is allocated by the HAL and should be unique for this
     * audio HAL module. */
    int (*create_audio_patch)(struct audio_hw_device *dev,
                               unsigned int num_sources,
                               const struct audio_port_config *sources,
                               unsigned int num_sinks,
                               const struct audio_port_config *sinks,
                               audio_patch_handle_t *handle);

    /* Release an audio patch */
    int (*release_audio_patch)(struct audio_hw_device *dev,
                               audio_patch_handle_t handle);

    /* Fills the list of supported attributes for a given audio port.
     * As input, "port" contains the information (type, role, address etc...)
     * needed by the HAL to identify the port.
     * As output, "port" contains possible attributes (sampling rates, formats,
     * channel masks, gain controllers...) for this port.
     */
    int (*get_audio_port)(struct audio_hw_device *dev,
                          struct audio_port *port);

    /* Set audio port configuration */
    int (*set_audio_port_config)(struct audio_hw_device *dev,
                         const struct audio_port_config *config);

};
typedef struct audio_hw_device audio_hw_device_t;
```

可以看到Google对它的注释还是蛮详细的，我们就不过多的描述了，这样做的好处是可以提高代码的拓展性，例如需要增加个HAL层接口，我们只需要在这个结构体中添加，Android层上层代码不需要做任何的修改。我们再继续分析openOutput()这个函数，还是回到AudioPolicyClientImpl这个类中来。

```c++
status_t AudioPolicyService::AudioPolicyClient::openOutput(audio_module_handle_t module,
                                                           audio_io_handle_t *output,
                                                           audio_config_t *config,
                                                           audio_devices_t *devices,
                                                           const String8& address,
                                                           uint32_t *latencyMs,
                                                           audio_output_flags_t flags)
{
    sp<IAudioFlinger> af = AudioSystem::get_audio_flinger();
    if (af == 0) {
        ALOGW("%s: could not get AudioFlinger", __func__);
        return PERMISSION_DENIED;
    }
    return af->openOutput(module, output, config, devices, address, latencyMs, flags);
}
```

非常有意思的是，AudioPolicyClientImpl似乎不想处理任何逻辑，很多时候只是做一个中转的处理，这里又把活丢给了AudioFlinger这个类。

```c++
status_t AudioFlinger::openOutput(audio_module_handle_t module,
                                  audio_io_handle_t *output,
                                  audio_config_t *config,
                                  audio_devices_t *devices,
                                  const String8& address,
                                  uint32_t *latencyMs,
                                  audio_output_flags_t flags)
{
    ALOGI("openOutput(), module %d Device %x, SamplingRate %d, Format %#08x, Channels %x, flags %x",
              module,
              (devices != NULL) ? *devices : 0,
              config->sample_rate,
              config->format,
              config->channel_mask,
              flags);

    if (*devices == AUDIO_DEVICE_NONE) {
        return BAD_VALUE;
    }

    Mutex::Autolock _l(mLock);

    sp<PlaybackThread> thread = openOutput_l(module, output, config, *devices, address, flags);
        //......
        return NO_ERROR;
    }

    return NO_INIT;
}

sp<AudioFlinger::PlaybackThread> AudioFlinger::openOutput_l(audio_module_handle_t module,
                                                            audio_io_handle_t *output,
                                                            audio_config_t *config,
                                                            audio_devices_t devices,
                                                            const String8& address,
                                                            audio_output_flags_t flags)
{
    //1. 找到合适的设备，AudioHwDevice在上文也说明了是对 audio_hw_device_t对象的封装
    //主要根据module的 handle_t 去获取 AudioHwDevice
    AudioHwDevice *outHwDev = findSuitableHwDev_l(module, devices);
    if (outHwDev == NULL) {
        return 0;
    }

    //获取 audio_hw_device_t 的指针
    audio_hw_device_t *hwDevHal = outHwDev->hwDevice();
    if (*output == AUDIO_IO_HANDLE_NONE) {
        *output = nextUniqueId();
    }

    mHardwareStatus = AUDIO_HW_OUTPUT_OPEN;

    // FOR TESTING ONLY:
    // This if statement allows overriding the audio policy settings
    // and forcing a specific format or channel mask to the HAL/Sink device for testing.
    if (!(flags & (AUDIO_OUTPUT_FLAG_COMPRESS_OFFLOAD | AUDIO_OUTPUT_FLAG_DIRECT))) {
        // Check only for Normal Mixing mode
        if (kEnableExtendedPrecision) {
            // Specify format (uncomment one below to choose)
            //config->format = AUDIO_FORMAT_PCM_FLOAT;
            //config->format = AUDIO_FORMAT_PCM_24_BIT_PACKED;
            //config->format = AUDIO_FORMAT_PCM_32_BIT;
            //config->format = AUDIO_FORMAT_PCM_8_24_BIT;
            // ALOGV("openOutput_l() upgrading format to %#08x", config->format);
        }
        if (kEnableExtendedChannels) {
            // Specify channel mask (uncomment one below to choose)
            //config->channel_mask = audio_channel_out_mask_from_count(4);  // for USB 4ch
            //config->channel_mask = audio_channel_mask_from_representation_and_bits(
            //        AUDIO_CHANNEL_REPRESENTATION_INDEX, (1 << 4) - 1);  // another 4ch example
        }
    }

    //获取输出流，往这个流中写数据，就会到Hal层，Hal就会往声卡里写入pcm流
    AudioStreamOut *outputStream = NULL;
    status_t status = outHwDev->openOutputStream(
            &outputStream,
            *output,
            devices,
            flags,
            config,
            address.string());

    mHardwareStatus = AUDIO_HW_IDLE;

    //根据flag的不同创建不同的输出线程，原来输出线程的创建是在这里
    //PlaybackThread是所有线程的基类
    if (status == NO_ERROR) {

        PlaybackThread *thread;
        if (flags & AUDIO_OUTPUT_FLAG_COMPRESS_OFFLOAD) {
            thread = new OffloadThread(this, outputStream, *output, devices, mSystemReady);
            ALOGV("openOutput_l() created offload output: ID %d thread %p", *output, thread);
        } else if ((flags & AUDIO_OUTPUT_FLAG_DIRECT)
                || !isValidPcmSinkFormat(config->format)
                || !isValidPcmSinkChannelMask(config->channel_mask)) {
            //直接输出线程
            thread = new DirectOutputThread(this, outputStream, *output, devices, mSystemReady);
            ALOGV("openOutput_l() created direct output: ID %d thread %p", *output, thread);
        } else {
            //混音线程
            thread = new MixerThread(this, outputStream, *output, devices, mSystemReady);
            ALOGV("openOutput_l() created mixer output: ID %d thread %p", *output, thread);
        }
        //将线程添加到mPlaybackThreads中去
        mPlaybackThreads.add(*output, thread);
        return thread;
    }

    return 0;
}
```

终于分析完这个方法，代码量实在太大了，我们只是看了其中最重要的代码片段，来总结下，压压惊，梳理下流程，首先，AudioPolicyService会加载音频设备的配置文件，audio_policy.conf，然后将数据加载到内存中去，生成了HwModule和IOProfile对象，可以把他们看成对音频设备的一个描述，之后AudioPolicyService会去根据HwModule的模块名去加载不同的HAL层的动态库，返回给Native层的是audio_hw_device_t对象的指针，接着会去获取AudioStreamOut的指针对象，这个对象是将Client端写入共享内存的pcm流 buffer直接写入声卡里去，然后最重要的一步就是生成这个设备的输出线程，并将output作为索引。可能我的描述比较简单，但是音频模块的AudioPolicyService的内容远远不只这么简单。

也许，你已经被AudioFlinger和AudioPolicyService的初始化工作给弄昏了头脑，但是还没有结束，我们回过头来看看AudioPolicyService的初始化的第三步，创建了三个线程，都做了哪些事情，我们还没有分析。我们来回顾下代码。

```c++
        // start tone playback thread
        mTonePlaybackThread = new AudioCommandThread(String8("ApmTone"), this);
        // start audio commands thread
        mAudioCommandThread = new AudioCommandThread(String8("ApmAudio"), this);
        // start output activity command thread
        mOutputCommandThread = new AudioCommandThread(String8("ApmOutput"), this);
```

 从注释上来看，一个是用来播放铃声的，一个是用来执行有关音频的指令的，另外一条是用来执行播放活动的命令的，不太清楚，但是三条线程都是AudioCommandThread，最核心处理的逻辑都是放在threadLoop()里面的。我们先看看这三条线程到底处理写什么事件，我们就不去看它的实现了，我们先看看AudioPolicyService.h文件中对它的定义：

```c++
   // commands for tone AudioCommand
        enum {
            //播放铃声
            START_TONE,
            STOP_TONE,
            SET_VOLUME,//设置音量
            SET_PARAMETERS, //setParameters，HAL层会接收到
            SET_VOICE_VOLUME,
            STOP_OUTPUT,
            RELEASE_OUTPUT,
            CREATE_AUDIO_PATCH,
            RELEASE_AUDIO_PATCH,
            UPDATE_AUDIOPORT_LIST,
            UPDATE_AUDIOPATCH_LIST,
            SET_AUDIOPORT_CONFIG,
            DYN_POLICY_MIX_STATE_UPDATE
        };
```

这个是AudioCommandThread类中的一个枚举，这是所有的处理指令，当然我们也可以继续拓展，其实最后真正处理音频相关的全都在AudioFlinger这个类中，谁让它直接和HAL层接触呢。我们可以回想下，当时再创建AudioPolicyService的时候， 我们创建了AudioPolicyClient，然后又创建了AudioPolicyManager，不知道大家还是否有印象，我们花了大量的精力分析AudioPolicyManager，并且其中的内容我们只是带过而已，因为音频策略相关的内容完全可以单独用一篇文章来讲解，但是它还是无比的复杂，这里就不说了，我们只是带过了AudioPolicyClient，只知道它是一个提供给客户端的接口，现在看看AudioCommandThread，再看看AudioPolicyClient就知道了，当用户端调用AudioPolicyClient这个接口，都是AudioCommandThread再处理，例如startTone，setParameters。我们就拿setParameters来分析吧：

```c++
//当客户端调用setParameters
void AudioPolicyService::setParameters(audio_io_handle_t ioHandle,
                                       const char *keyValuePairs,
                                       int delayMs)
{
    mAudioCommandThread->parametersCommand(ioHandle, keyValuePairs,
                                           delayMs);
}

status_t AudioPolicyService::AudioCommandThread::parametersCommand(
                            audio_io_handle_t ioHandle,
                            const char *keyValuePairs,
                            int delayMs)
{
    sp<AudioCommand> command = new AudioCommand();
    command->mCommand = SET_PARAMETERS;
    sp<ParametersData> data = new ParametersData();
    data->mIO = ioHandle;
    data->mKeyValuePairs = String8(keyValuePairs);
    command->mParam = data;
    command->mWaitStatus = true;
    ALOGV("AudioCommandThread() adding set parameter string %s, io %d ,delay %d",
            keyValuePairs, ioHandle, delayMs);
    return sendCommand(command, delayMs);
}

status_t AudioPolicyService::AudioCommandThread::sendCommand(sp<AudioCommand>& command, int delayMs)
{
    //有命令来了，通知线程干活!
    {
        Mutex::Autolock _l(mLock);
        insertCommand_l(command, delayMs);
        mWaitWorkCV.signal();
    }
    Mutex::Autolock _l(command->mLock);
    while (command->mWaitStatus) {
        nsecs_t timeOutNs = kAudioCommandTimeoutNs + milliseconds(delayMs);
        if (command->mCond.waitRelative(command->mLock, timeOutNs) != NO_ERROR) {
            command->mStatus = TIMED_OUT;
            command->mWaitStatus = false;
        }
    }
    return command->mStatus;
}

bool AudioPolicyService::AudioCommandThread::threadLoop()
{
    nsecs_t waitTime = INT64_MAX;

    mLock.lock();
    while (!exitPending())
    {
        sp<AudioPolicyService> svc;
        while (!mAudioCommands.isEmpty() && !exitPending()) {
            nsecs_t curTime = systemTime();
            // commands are sorted by increasing time stamp: execute them from index 0 and up
            if (mAudioCommands[0]->mTime <= curTime) {
                sp<AudioCommand> command = mAudioCommands[0];
                mAudioCommands.removeAt(0);
                mLastCommand = command;

                switch (command->mCommand) {
              
                case SET_PARAMETERS: {
                    ParametersData *data = (ParametersData *)command->mParam.get();
                    ALOGV("AudioCommandThread() processing set parameters string %s, io %d",
                            data->mKeyValuePairs.string(), data->mIO);
                    command->mStatus = AudioSystem::setParameters(data->mIO, data->mKeyValuePairs);
                    }break;
                default:
                    ALOGW("AudioCommandThread() unknown command %d", command->mCommand);
                }
                {
                    Mutex::Autolock _l(command->mLock);
                    if (command->mWaitStatus) {
                        command->mWaitStatus = false;
                        command->mCond.signal();
                    }
                }
                waitTime = INT64_MAX;
                // release mLock before releasing strong reference on the service as
                // AudioPolicyService destructor calls AudioCommandThread::exit() which
                // acquires mLock.
                mLock.unlock();
                svc.clear();
                mLock.lock();
            } else {
                waitTime = mAudioCommands[0]->mTime - curTime;
                break;
            }
        }

        // release delayed commands wake lock if the queue is empty
        if (mAudioCommands.isEmpty()) {
            release_wake_lock(mName.string());
        }

        // At this stage we have either an empty command queue or the first command in the queue
        // has a finite delay. So unless we are exiting it is safe to wait.
        //如果没有命令要处理，就等...
        if (!exitPending()) {
            ALOGV("AudioCommandThread() going to sleep");
            mWaitWorkCV.waitRelative(mLock, waitTime);
        }
    }
    //线程 quit之后
    // release delayed commands wake lock before quitting
    if (!mAudioCommands.isEmpty()) {
        release_wake_lock(mName.string());
    }
    mLock.unlock();
    return false;
}
```

上面的代码比较简单，我觉得对各位看客来说，还是比较容易理解的。值得一提的是，最后任务交给AudioSystem来处理，其实经过几次函数调用，还是AudioFlinger去处理了，所以为什么我上面那么说。至此，AudioTrack，AudioFlinger，AudioPolicyService初始化所作的事情，已经全部分析完了。