
#define LOG_TAG "Codec_jni"
#include <utils/Log.h>


#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include "jni.h"
#include "mydsp.h"

jint dsp_Init(JNIEnv *env, jobject thiz)
{  jint status = 0;
	status = init_dsp();
	return status;
}

jint ld_dspcfg(JNIEnv *env, jobject thiz)
{
	 jint status = 0;
	 status = ld_cfg();
	 return status;
}

static JNINativeMethod myMethods[] ={
	{"Dsp_Init", "()I", (void *)dsp_Init},
	{"Dsp_Ld_Cfg", "()I", (void *)ld_dspcfg},	
};


jint JNI_OnLoad(JavaVM * vm,void * reserved)
{

	JNIEnv *env = NULL;

	jint ret ;

	ret = vm->GetEnv((void * * )&env,  JNI_VERSION_1_4);
	if(ret != 0)
	{
		//LOGE("vm->GetEnv error\n");
		return -1;
	}

	jclass myclz = env->FindClass("Codecdsp/CODEC_API");
	if(myclz == NULL)
	{
		
		//LOGE("env->FindClass error\n");
		return -1;
	}
	
	ret = env->RegisterNatives(myclz, myMethods, sizeof(myMethods)/sizeof(myMethods[0]));
	if(ret < 0)
	{
		//LOGE("env->RegisterNatives error\n");
		return -1;
	}

	return JNI_VERSION_1_4;

}



