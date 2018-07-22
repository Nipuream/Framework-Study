#ifndef _JNI_RECORD_H_
#define _JNI_RECORD_H_


/*

   JNIѧϰ��¼

    1. ��GC��ȡ��������ͣ���̹߳�����Android������ͣ��һ��JNI���ã�������native����
	2. native �߳�ִ��JNI���ã���AttachCurrentThread �������������������ִ��DetachCurrentThread
	3. ��ʱ�䱣�����õĴ�����ʹ��NewGlobalRef����������һ����ʱ���ò�����Ȼ���ڷ���һ��ȫ������
	    eg:
		            jclass localClass = env->FindClass("MyClass");
					jclass globalClass = reinterpret_cast<jclass>(env->NewGlobalRef(localClass));
		ע�⣺��ͬһ������ͬ��ȫ������ֵ���ܲ���ͬһ��������ʹ�� IsSameObjet�������жϣ�
		               jfieldID��jmethodID�ǲ�͸�������ͣ������Ƕ������ã����Բ�����NewGlobalRef�����á�
					   ĳЩ���� GetStringUTFChars �� GetByteArrayElements���ص�ԭʼ����ָ��Ҳ���Ƕ���

	4. ��������˺ܶ���ʱ���ã����ʹ��DeleteLocalRef���ͷţ���������JNI������
	5. ����������д��������һ����÷�����
	     
		       jbyte* data = env->GetByteArrayElements(array, NULL);
			   if(data != NULL){
			       memcpy(buffer, data, len);
				   env->ReleaseByteArrayElements(array, data, JNI_ABORT);
			   }

			   ����һ����Ϊ�򵥵ĵ��ã�
			    env->GetByteArrayRegion(array, 0, len, buffer);
				<��С��֧������Ҫ��ԭʼ���ݽ��ж���Ŀ��������ٿ����ߵķ���>

				ͬ��GetStringRegion ���� GetStringUTFRegion ��String �п����ַ�

		6. ��ԭ�������������������ݣ�
		     ����и��ܴ��ԭʼ���ݣ�����ͼƬ������Ƶ����
			 1. ������java�����ݴ���byte[]�У�java���кܿ�ķ����ٶȣ�JNIͨ��GetByteArrayElements �� GetPrimitiveArrayCritical
			      �����ʵ�����ݵĵ�ַ��ָ�롣
			 2. ������byte�����У�������java.nio.ByteBuffer.allocateDirect��������������JNI��NewDirectByteBuffer�������������
			      ��������java���Ϸ���ռ䣬����ԭ���������ֱ��ͨ��GetDirectBufferAddress��õ�ַ

*/


#endif