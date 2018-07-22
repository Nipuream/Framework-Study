//
// Created by yanghui11 on 2018/6/29.
//

#include "yanghui_smoperator.h"
#include "base_log.h"
#include <cutils/ashmem.h>
#include <unistd.h>
#include <sys/mman.h>
#include <JNIHelp.h>
#include <errno.h>

static bool unpinned = false;


JNIEXPORT jint JNICALL Java_com_hikvision_yanghui11_frameworkstudy_SMOperator_write
(JNIEnv *env, jclass cls, jint fd, jbyteArray buf, jint srcOffset, jint destOffset, jint count, jint address) {

	LOGI("enter sm write ~");

	if (unpinned && ashmem_pin_region(fd, 0, 0) == ASHMEM_NOT_PURGED) {
		ashmem_unpin_region(fd, 0, 0);
		jniThrowException(env, "java/io/IOException", "ashmem region was purged");
		return -1;
	}

	env->GetByteArrayRegion(buf, srcOffset, count, (jbyte *)address + destOffset);

	if (unpinned){
		ashmem_unpin_region(fd, 0, 0);
	}

	return count;
}


JNIEXPORT jint JNICALL Java_com_hikvision_yanghui11_frameworkstudy_SMOperator_read
(JNIEnv *env, jclass cls, jint fd, jbyteArray buf, jint srcOffset, jint destOffset, jint count, jint address) {

	LOGI("enter sm read ~");

	if (unpinned && ashmem_pin_region(fd, 0, 0) == ASHMEM_WAS_PURGED) {
		ashmem_unpin_region(fd, 0, 0);
		jniThrowException(env, "java/io/IOException", "ashmem region was purged");
		return -1;
	}

	env->SetByteArrayRegion(buf, destOffset, count, (const jbyte*)address + srcOffset);

	if (unpinned){
		ashmem_unpin_region(fd, 0, 0);
	}

	return count;
}


JNIEXPORT jint JNICALL Java_com_hikvision_yanghui11_frameworkstudy_SMOperator_mmap
(JNIEnv  *env, jclass cls, jint fd, jint length, jint mode) {

	LOGI("enter mmap function ~");
	void* result = mmap(0, length, mode, MAP_SHARED, fd, 0);
	LOGI("mmap result : %d", result);
	LOGI("mmap faild : %s",strerror(errno));
	if (result == MAP_FAILED)
		 jniThrowException(env, "java/io/IOException", "mmap failed");
	
	return (jint)result;
}

JNIEXPORT void JNICALL Java_com_hikvision_yanghui11_frameworkstudy_SMOperator_close
(JNIEnv *env, jclass cls, jint fd) {

	LOGI("enter close fd ~");
	if (fd >= 0){
		close(fd);
	}

}





