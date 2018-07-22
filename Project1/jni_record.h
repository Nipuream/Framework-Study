#ifndef _JNI_RECORD_H_
#define _JNI_RECORD_H_


/*

   JNI学习记录

    1. 当GC获取调试器暂停了线程工作，Android将会暂停下一个JNI调用，而不是native代码
	2. native 线程执行JNI调用，需AttachCurrentThread 上虚拟机，结束调用再执行DetachCurrentThread
	3. 长时间保持引用的存活，必须使用NewGlobalRef方法来接受一个临时引用参数，然后在返回一个全局引用
	    eg:
		            jclass localClass = env->FindClass("MyClass");
					jclass globalClass = reinterpret_cast<jclass>(env->NewGlobalRef(localClass));
		注意：对同一个对象不同的全局引用值可能不是同一个，必须使用 IsSameObjet方法来判断；
		               jfieldID和jmethodID是不透明的类型，而不是对象引用，所以不能用NewGlobalRef来调用。
					   某些方法 GetStringUTFChars 和 GetByteArrayElements返回的原始数据指针也不是对象。

	4. 如果创建了很多临时引用，最好使用DeleteLocalRef来释放，而不是让JNI来做。
	5. 如果想对数组写入或读出，一般调用方法：
	     
		       jbyte* data = env->GetByteArrayElements(array, NULL);
			   if(data != NULL){
			       memcpy(buffer, data, len);
				   env->ReleaseByteArrayElements(array, data, JNI_ABORT);
			   }

			   另外一个更为简单的调用：
			    env->GetByteArrayRegion(array, 0, len, buffer);
				<减小开支，不需要对原始数据进行额外的拷贝，减少开发者的风险>

				同理，GetStringRegion 或者 GetStringUTFRegion 从String 中拷贝字符

		6. 在原生代码中怎样分享数据？
		     如果有个很大的原始数据，例如图片或者音频数据
			 1. 可以在java将数据存在byte[]中，java中有很快的访问速度，JNI通过GetByteArrayElements 或 GetPrimitiveArrayCritical
			      将获得实际数据的地址的指针。
			 2. 保存在byte缓存中，可以用java.nio.ByteBuffer.allocateDirect来创建，或者用JNI的NewDirectByteBuffer方法，这个缓存
			      并不是在java堆上分配空间，可以原生代码可以直接通到GetDirectBufferAddress获得地址

*/


#endif