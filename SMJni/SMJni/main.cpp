
#include "jni.h"
#include "base_log.h"
#include "yanghui_smoperator.h"

#define NELEM(x) ((int)(sizeof(x)/sizeof((x)[0])))

static JNINativeMethod gMethods[]  = {

	{ "write", "(I[BIIII)I", (jint*)Java_com_hikvision_yanghui11_frameworkstudy_SMOperator_write},
	{ "read", "(I[BIIII)I", (jint*)Java_com_hikvision_yanghui11_frameworkstudy_SMOperator_read},
	{ "mmap", "(III)I", (jint*)Java_com_hikvision_yanghui11_frameworkstudy_SMOperator_mmap},
	{ "close", "(I)V", (void*)Java_com_hikvision_yanghui11_frameworkstudy_SMOperator_close }
};

jint JNI_OnLoad(JavaVM* vm, void* reserved) {
	
	JNIEnv* env = NULL;
	if (vm->GetEnv((void**)&env, JNI_VERSION_1_4) != JNI_OK){
		LOGI("GET env fail ~");
	}

	jclass jclazz = env->FindClass("com/hikvision/yanghui11/frameworkstudy/SMOperator");
	if (env->RegisterNatives(jclazz, gMethods, NELEM(gMethods)) < 0){
		LOGI("register jni error !");
	}

	return JNI_VERSION_1_4;
}