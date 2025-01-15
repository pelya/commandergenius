/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2012 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
/*
This source code is distibuted under ZLIB license, however when compiling with SDL 1.2,
which is licensed under LGPL, the resulting library, and all it's source code,
falls under "stronger" LGPL terms, so is this file.
If you compile this code with SDL 1.3 or newer, or use in some other way, the license stays ZLIB.
*/

#include "SDL_config.h"
#include "SDL_version.h"

#include "SDL_rwops.h"
#include "SDL_timer.h"
#include "SDL_audio.h"
#include "../SDL_audiomem.h"
#include "../SDL_audio_c.h"
#include "../SDL_audiodev_c.h"
#include "SDL_androidaudio.h"
#include "SDL_mutex.h"
#include "SDL_thread.h"
#include <jni.h>
#include <android/log.h>
#include <string.h> // for memset()
#include <pthread.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define _THIS	SDL_AudioDevice *this

/* Audio driver functions */

static void ANDROIDAUD_WaitAudio(_THIS);
static void ANDROIDAUD_PlayAudio(_THIS);
static Uint8 *ANDROIDAUD_GetAudioBuf(_THIS);
static void ANDROIDAUD_CloseAudio(_THIS);
static void ANDROIDAUD_ThreadInit(_THIS);
static void ANDROIDAUD_ThreadDeinit(_THIS);

#if SDL_VERSION_ATLEAST(1,3,0)

static int ANDROIDAUD_OpenAudio(_THIS, const char *devname, int iscapture);

static void ANDROIDAUD_DeleteDevice()
{
}

static int ANDROIDAUD_CreateDevice(SDL_AudioDriverImpl * impl)
{

	/* Set the function pointers */
	impl->OpenDevice = ANDROIDAUD_OpenAudio;
	impl->WaitDevice = ANDROIDAUD_WaitAudio;
	impl->PlayDevice = ANDROIDAUD_PlayAudio;
	impl->GetDeviceBuf = ANDROIDAUD_GetAudioBuf;
	impl->CloseDevice = ANDROIDAUD_CloseAudio;
	impl->ThreadInit = ANDROIDAUD_ThreadInit;
	impl->WaitDone = ANDROIDAUD_ThreadDeinit;
	impl->Deinitialize = ANDROIDAUD_DeleteDevice;
	impl->OnlyHasDefaultOutputDevice = 1;

	return 1;
}

AudioBootStrap ANDROIDAUD_bootstrap = {
	"android", "SDL Android audio driver",
	ANDROIDAUD_CreateDevice, 0
};

#else

static int ANDROIDAUD_OpenAudio(_THIS, SDL_AudioSpec *spec);

static int ANDROIDAUD_Available(void)
{
	return(1);
}

static void ANDROIDAUD_DeleteDevice(SDL_AudioDevice *device)
{
	SDL_free(device);
}

static SDL_AudioDevice *ANDROIDAUD_CreateDevice(int devindex)
{
	SDL_AudioDevice *this;

	/* Initialize all variables that we clean on shutdown */
	this = (SDL_AudioDevice *)SDL_malloc(sizeof(SDL_AudioDevice));
	if ( this ) {
		SDL_memset(this, 0, (sizeof *this));
		this->hidden = NULL;
	} else {
		SDL_OutOfMemory();
		return(0);
	}

	/* Set the function pointers */
	this->OpenAudio = ANDROIDAUD_OpenAudio;
	this->WaitAudio = ANDROIDAUD_WaitAudio;
	this->PlayAudio = ANDROIDAUD_PlayAudio;
	this->GetAudioBuf = ANDROIDAUD_GetAudioBuf;
	this->CloseAudio = ANDROIDAUD_CloseAudio;
	this->ThreadInit = ANDROIDAUD_ThreadInit;
	this->WaitDone = ANDROIDAUD_ThreadDeinit;
	this->free = ANDROIDAUD_DeleteDevice;

	return this;
}

AudioBootStrap ANDROIDAUD_bootstrap = {
	"android", "SDL Android audio driver",
	ANDROIDAUD_Available, ANDROIDAUD_CreateDevice
};

#endif


static unsigned char * audioBuffer = NULL;
static size_t audioBufferSize = 0;
static Uint32 audioLastTick = 0;
static unsigned char * shadowAppBuffer = NULL;
static Uint32 shadowAppBufferPos = 0;
static Uint32 shadowAppBufferSize = 0;

// Extremely wicked JNI environment to call Java functions from C code
static jbyteArray audioBufferJNI = NULL;
static JavaVM *jniVM = NULL;
static jobject JavaAudioThread = NULL;
static jmethodID JavaInitAudio = NULL;
static jmethodID JavaDeinitAudio = NULL;
static jmethodID JavaPauseAudioPlayback = NULL;
static jmethodID JavaResumeAudioPlayback = NULL;

// Audio recording

static SDL_AudioSpec recording;
static jbyteArray recordingBufferJNI = NULL;
static size_t recordingBufferSize = 0;


static Uint8 *ANDROIDAUD_GetAudioBuf(_THIS)
{
#ifdef SDL_AUDIO_APP_IGNORES_RETURNED_BUFFER_SIZE
	return(shadowAppBuffer);
#else
	return(audioBuffer);
#endif
}


#if SDL_VERSION_ATLEAST(1,3,0)
static int ANDROIDAUD_OpenAudio (_THIS, const char *devname, int iscapture)
{
	SDL_AudioSpec *audioFormat = &this->spec;

#else
static int ANDROIDAUD_OpenAudio (_THIS, SDL_AudioSpec *spec)
{
	SDL_AudioSpec *audioFormat = spec;
#endif

	int bytesPerSample;
	JNIEnv * jniEnv = NULL;

	this->hidden = NULL;

	if( ! (audioFormat->format == AUDIO_U8 || audioFormat->format == AUDIO_S16) )
	{
		__android_log_print(ANDROID_LOG_ERROR, "libSDL", "Application requested unsupported audio format - only S8 and S16 are supported");
		return (-1); // TODO: enable format conversion? Don't know how to do that in SDL
	}

	bytesPerSample = (audioFormat->format & 0xFF) / 8;
	audioFormat->format = ( bytesPerSample == 2 ) ? AUDIO_S16 : AUDIO_U8;

	__android_log_print(ANDROID_LOG_INFO, "libSDL", "ANDROIDAUD_OpenAudio(): app requested audio bytespersample %d freq %d channels %d samples %d", bytesPerSample, audioFormat->freq, (int)audioFormat->channels, (int)audioFormat->samples);

	if(audioFormat->samples <= 0)
		audioFormat->samples = 16; // Some sane value
	if( audioFormat->samples > 32768 ) // Why anyone need so huge audio buffer?
	{
		audioFormat->samples = 32768;
		__android_log_print(ANDROID_LOG_INFO, "libSDL", "ANDROIDAUD_OpenAudio(): limiting samples size to %d", (int)audioFormat->samples);
	}
	
	SDL_CalculateAudioSpec(audioFormat);
	
	(*jniVM)->AttachCurrentThread(jniVM, &jniEnv, NULL);

	if( !jniEnv )
	{
		__android_log_print(ANDROID_LOG_ERROR, "libSDL", "ANDROIDAUD_OpenAudio: Java VM AttachCurrentThread() failed");
		return (-1);
	}

	__android_log_print(ANDROID_LOG_INFO, "libSDL", "ANDROIDAUD_OpenAudio(): Requesting audio: freq %d channels %d format %s bufsize %d", audioFormat->freq, audioFormat->channels, bytesPerSample == 2 ? "S16" : "S8", audioFormat->size);
	// The returned audioBufferSize may be huge, up to 100 Kb for 44100 because user may have selected large audio buffer to get rid of choppy sound
	audioBufferSize = (*jniEnv)->CallIntMethod( jniEnv, JavaAudioThread, JavaInitAudio, 
					(jint)audioFormat->freq, (jint)audioFormat->channels, 
					(jint)(( bytesPerSample == 2 ) ? 1 : 0), (jint)(audioFormat->size) );

	if( audioBufferSize == 0 )
	{
		__android_log_print(ANDROID_LOG_INFO, "libSDL", "ANDROIDAUD_OpenAudio(): failed to get audio buffer from JNI");
		ANDROIDAUD_CloseAudio(this);
		return(-1);
	}

	/* We cannot call DetachCurrentThread() from main thread or we'll crash */
	/* (*jniVM)->DetachCurrentThread(jniVM); */

#ifdef SDL_AUDIO_APP_IGNORES_RETURNED_BUFFER_SIZE
	shadowAppBufferSize = audioFormat->size;
	shadowAppBuffer = malloc(shadowAppBufferSize);
	shadowAppBufferPos = 0;
	if( shadowAppBufferSize > audioBufferSize )
		__android_log_print(ANDROID_LOG_FATAL, "libSDL", "ANDROIDAUD_OpenAudio(): Java returned audio buffer smaller than app requested, SDL will crash!");
#else
	audioFormat->samples = audioBufferSize / bytesPerSample / audioFormat->channels;
	audioFormat->size = audioBufferSize;
#endif

	SDL_CalculateAudioSpec(audioFormat);
	__android_log_print(ANDROID_LOG_INFO, "libSDL", "ANDROIDAUD_OpenAudio(): app opened audio bytespersample %d freq %d channels %d bufsize %d, SDL returns bufsize %d", bytesPerSample, audioFormat->freq, (int)audioFormat->channels, (int)audioBufferSize, audioFormat->size);

#if SDL_VERSION_ATLEAST(1,3,0)
	return(1);
#else
	return(0);
#endif
}

static void ANDROIDAUD_CloseAudio(_THIS)
{
	//__android_log_print(ANDROID_LOG_INFO, "libSDL", "ANDROIDAUD_CloseAudio()");
	JNIEnv * jniEnv = NULL;
	(*jniVM)->AttachCurrentThread(jniVM, &jniEnv, NULL);

	(*jniEnv)->DeleteGlobalRef(jniEnv, audioBufferJNI);
	audioBufferJNI = NULL;
	audioBuffer = NULL;
	audioBufferSize = 0;
#ifdef SDL_AUDIO_APP_IGNORES_RETURNED_BUFFER_SIZE
	free(shadowAppBuffer);
	shadowAppBuffer = NULL;
#endif
	(*jniEnv)->CallIntMethod( jniEnv, JavaAudioThread, JavaDeinitAudio );

	/* We cannot call DetachCurrentThread() from main thread or we'll crash */
	/* (*jniVM)->DetachCurrentThread(jniVM); */
	
}

/* This function waits until it is possible to write a full sound buffer */
static void ANDROIDAUD_WaitAudio(_THIS)
{
	/* We will block in PlayAudio(), do nothing here */
#ifdef SDL_AUDIO_PREVENT_CHOPPING_WITH_DELAY
	//ZX:
	if (audioLastTick == 0) {
		audioLastTick = SDL_GetTicks();
	} else {
		unsigned int audioNewTick = SDL_GetTicks();
		unsigned int delay_in_ms = (this->spec.samples*1000)/this->spec.freq;
		int deltaTick = audioNewTick - audioLastTick;
		if (delay_in_ms > deltaTick) {
			SDL_Delay( delay_in_ms - deltaTick );
		}
		audioLastTick = audioNewTick;
	}
#endif
}

static JNIEnv * jniEnvPlaying = NULL;
static jmethodID JavaFillBuffer = NULL;

static void ANDROIDAUD_ThreadInit(_THIS)
{
	jclass JavaAudioThreadClass = NULL;
	jmethodID JavaInitThread = NULL;
	jmethodID JavaGetBuffer = NULL;
	jboolean isCopy = JNI_TRUE;

	(*jniVM)->AttachCurrentThread(jniVM, &jniEnvPlaying, NULL);

	JavaAudioThreadClass = (*jniEnvPlaying)->GetObjectClass(jniEnvPlaying, JavaAudioThread);
	JavaFillBuffer = (*jniEnvPlaying)->GetMethodID(jniEnvPlaying, JavaAudioThreadClass, "fillBuffer", "()I");

	/* HACK: raise our own thread priority to max to get rid of "W/AudioFlinger: write blocked for 54 msecs" errors */
	JavaInitThread = (*jniEnvPlaying)->GetMethodID(jniEnvPlaying, JavaAudioThreadClass, "initAudioThread", "()I");
	(*jniEnvPlaying)->CallIntMethod( jniEnvPlaying, JavaAudioThread, JavaInitThread );

	JavaGetBuffer = (*jniEnvPlaying)->GetMethodID(jniEnvPlaying, JavaAudioThreadClass, "getBuffer", "()[B");
	audioBufferJNI = (*jniEnvPlaying)->CallObjectMethod( jniEnvPlaying, JavaAudioThread, JavaGetBuffer );
	audioBufferJNI = (*jniEnvPlaying)->NewGlobalRef(jniEnvPlaying, audioBufferJNI);
	//audioBuffer = (unsigned char *) (*jniEnvPlaying)->GetByteArrayElements(jniEnvPlaying, audioBufferJNI, &isCopy);
	audioBuffer = (unsigned char *) (*jniEnvPlaying)->GetPrimitiveArrayCritical(jniEnvPlaying, audioBufferJNI, &isCopy);
	if( !audioBuffer )
	{
		__android_log_print(ANDROID_LOG_ERROR, "libSDL", "ANDROIDAUD_ThreadInit() JNI::GetByteArrayElements() failed! we will crash now");
		return;
	}
	if( isCopy == JNI_TRUE )
		__android_log_print(ANDROID_LOG_ERROR, "libSDL", "ANDROIDAUD_ThreadInit(): JNI returns a copy of byte array - no audio will be played");

	//__android_log_print(ANDROID_LOG_INFO, "libSDL", "ANDROIDAUD_ThreadInit()");
	SDL_memset(audioBuffer, this->spec.silence, this->spec.size);
};

static void ANDROIDAUD_ThreadDeinit(_THIS)
{
	//(*jniEnvPlaying)->ReleaseByteArrayElements(jniEnvPlaying, audioBufferJNI, (jbyte *)audioBuffer, 0);
	(*jniEnvPlaying)->ReleasePrimitiveArrayCritical(jniEnvPlaying, audioBufferJNI, (jbyte *)audioBuffer, 0);
	// (*jniEnvPlaying)->DeleteGlobalRef(jniEnvPlaying, audioBufferJNI); // Application crashes here for some unknown reason
	(*jniVM)->DetachCurrentThread(jniVM);
};

static void ANDROIDAUD_SendAudioToJava(void)
{
	//__android_log_print(ANDROID_LOG_INFO, "libSDL", "ANDROIDAUD_PlayAudio()");
	//jboolean isCopy = JNI_TRUE;

	//(*jniEnvPlaying)->ReleaseByteArrayElements(jniEnvPlaying, audioBufferJNI, (jbyte *)audioBuffer, 0);
	(*jniEnvPlaying)->ReleasePrimitiveArrayCritical(jniEnvPlaying, audioBufferJNI, (jbyte *)audioBuffer, 0);
	audioBuffer = NULL;

	(*jniEnvPlaying)->CallIntMethod( jniEnvPlaying, JavaAudioThread, JavaFillBuffer );

	//audioBuffer = (unsigned char *) (*jniEnvPlaying)->GetByteArrayElements(jniEnvPlaying, audioBufferJNI, NULL);
	audioBuffer = (unsigned char *) (*jniEnvPlaying)->GetPrimitiveArrayCritical(jniEnvPlaying, audioBufferJNI, NULL);
	if( !audioBuffer )
		__android_log_print(ANDROID_LOG_ERROR, "libSDL", "ANDROIDAUD_PlayAudio() JNI::GetByteArrayElements() failed! we will crash now");

	//if( isCopy == JNI_TRUE )
	//	__android_log_print(ANDROID_LOG_INFO, "libSDL", "ANDROIDAUD_PlayAudio() JNI returns a copy of byte array - that's slow");
}

#ifdef SDL_AUDIO_APP_IGNORES_RETURNED_BUFFER_SIZE
static void ANDROIDAUD_PlayAudio(_THIS)
{
	int audioCopiedSize = MIN(shadowAppBufferSize, audioBufferSize - shadowAppBufferPos);
	memcpy(audioBuffer + shadowAppBufferPos, shadowAppBuffer, audioCopiedSize);
	shadowAppBufferPos += audioCopiedSize;
	if( shadowAppBufferPos >= audioBufferSize )
	{
		ANDROIDAUD_SendAudioToJava();
		int audioCopiedRemaining = shadowAppBufferSize - audioCopiedSize;
		memcpy(audioBuffer, shadowAppBuffer + audioCopiedSize, audioCopiedRemaining);
		shadowAppBufferPos = audioCopiedRemaining;
	}
}
#else
static void ANDROIDAUD_PlayAudio(_THIS)
{
	ANDROIDAUD_SendAudioToJava();
}
#endif


int SDL_ANDROID_PauseAudioPlayback(void)
{
	JNIEnv * jniEnv = NULL;
	(*jniVM)->AttachCurrentThread(jniVM, &jniEnv, NULL);
	return (*jniEnv)->CallIntMethod( jniEnv, JavaAudioThread, JavaPauseAudioPlayback );
};
int SDL_ANDROID_ResumeAudioPlayback(void)
{
	JNIEnv * jniEnv = NULL;
	(*jniVM)->AttachCurrentThread(jniVM, &jniEnv, NULL);
	return (*jniEnv)->CallIntMethod( jniEnv, JavaAudioThread, JavaResumeAudioPlayback );
};


#ifndef SDL_JAVA_PACKAGE_PATH
#error You have to define SDL_JAVA_PACKAGE_PATH to your package path with dots replaced with underscores, for example "com_example_SanAngeles"
#endif
#define JAVA_EXPORT_NAME2(name,package) Java_##package##_##name
#define JAVA_EXPORT_NAME1(name,package) JAVA_EXPORT_NAME2(name,package)
#define JAVA_EXPORT_NAME(name) JAVA_EXPORT_NAME1(name,SDL_JAVA_PACKAGE_PATH)

JNIEXPORT jint JNICALL JAVA_EXPORT_NAME(AudioThread_nativeAudioInitJavaCallbacks) (JNIEnv * jniEnv, jobject thiz)
{
	jclass JavaAudioThreadClass = NULL;
	JavaAudioThread = (*jniEnv)->NewGlobalRef(jniEnv, thiz);
	JavaAudioThreadClass = (*jniEnv)->GetObjectClass(jniEnv, JavaAudioThread);
	JavaInitAudio = (*jniEnv)->GetMethodID(jniEnv, JavaAudioThreadClass, "initAudio", "(IIII)I");
	JavaDeinitAudio = (*jniEnv)->GetMethodID(jniEnv, JavaAudioThreadClass, "deinitAudio", "()I");
	JavaPauseAudioPlayback = (*jniEnv)->GetMethodID(jniEnv, JavaAudioThreadClass, "pauseAudioPlayback", "()I");
	JavaResumeAudioPlayback = (*jniEnv)->GetMethodID(jniEnv, JavaAudioThreadClass, "resumeAudioPlayback", "()I");
	return 0;
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved)
{
	jniVM = vm;
	return JNI_VERSION_1_6;
};

JNIEXPORT void JNICALL JNI_OnUnload(JavaVM *vm, void *reserved)
{
	/* TODO: free JavaAudioThread */
	jniVM = vm;
};

// ----- Audio recording -----

JNIEXPORT void JNICALL JAVA_EXPORT_NAME(AudioThread_nativeAudioRecordCallback) (JNIEnv * jniEnv, jobject thiz)
{
	if( !recordingBufferJNI || !recordingBufferSize )
	{
		__android_log_print(ANDROID_LOG_ERROR, "libSDL", "AudioThread_nativeAudioRecordCallbacks(): error: recording buffer is NULL");
		return;
	}

	//Uint8 *buffer = (Uint8 *) (*jniEnv)->GetByteArrayElements(jniEnv, recordingBufferJNI, NULL);
	Uint8 *buffer = (Uint8 *) (*jniEnv)->GetPrimitiveArrayCritical(jniEnv, recordingBufferJNI, NULL);
	if( !buffer )
	{
		__android_log_print(ANDROID_LOG_ERROR, "libSDL", "AudioThread_nativeAudioRecordCallbacks(): error: JNI::GetByteArrayElements() failed!");
		return;
	}

	//__android_log_print(ANDROID_LOG_INFO, "libSDL", "AudioThread_nativeAudioRecordCallbacks(): got buffer %p len %d", buffer, recordingBufferSize);

	recording.callback(recording.userdata, buffer, recordingBufferSize);

	//(*jniEnv)->ReleaseByteArrayElements(jniEnv, recordingBufferJNI, (jbyte *)buffer, 0);
	(*jniEnv)->ReleasePrimitiveArrayCritical(jniEnv, recordingBufferJNI, (jbyte *)buffer, 0);
}

extern DECLSPEC int SDLCALL SDL_ANDROID_OpenAudioRecording(SDL_AudioSpec *spec)
{
	JNIEnv * jniEnv = NULL;
	jclass JavaAudioThreadClass = NULL;
	jmethodID JavaStartRecording = NULL;

	recording = *spec;

	if( ! (recording.format == AUDIO_U8 || recording.format == AUDIO_S16) )
	{
		__android_log_print(ANDROID_LOG_ERROR, "libSDL", "SDL_ANDROID_OpenAudioRecording(): Application requested unsupported audio format - only S8 and S16 are supported");
		return 0;
	}

	if( ! recording.callback )
	{
		__android_log_print(ANDROID_LOG_ERROR, "libSDL", "SDL_ANDROID_OpenAudioRecording(): Application did not provide callback");
		return 0;
	}

	//__android_log_print(ANDROID_LOG_INFO, "libSDL", "SDL_ANDROID_OpenAudioRecording(): VM %p JavaAudioThread %p JavaStartRecording %p", jniVM, JavaAudioThread, JavaStartRecording);

	(*jniVM)->AttachCurrentThread( jniVM, &jniEnv, NULL );
	JavaAudioThreadClass = (*jniEnv)->GetObjectClass( jniEnv, JavaAudioThread );
	JavaStartRecording = (*jniEnv)->GetMethodID( jniEnv, JavaAudioThreadClass, "startRecording", "(IIII)[B" );

	recordingBufferJNI = (*jniEnv)->CallObjectMethod( jniEnv, JavaAudioThread, JavaStartRecording,
														(jint)recording.freq, (jint)recording.channels,
														(jint)(recording.format == AUDIO_S16 ? 1 : 0), (jint)recording.size );
	if( !recordingBufferJNI )
	{
		__android_log_print(ANDROID_LOG_ERROR, "libSDL", "SDL_ANDROID_OpenAudioRecording(): Java did not return audio buffer");
		return 0;
	}
	recordingBufferJNI = (*jniEnv)->NewGlobalRef( jniEnv, recordingBufferJNI );
	recordingBufferSize = (*jniEnv)->GetArrayLength( jniEnv, recordingBufferJNI );
	//__android_log_print(ANDROID_LOG_INFO, "libSDL", "SDL_ANDROID_OpenAudioRecording(): JNI buffer %p len %d", recordingBufferJNI, recordingBufferSize);
	return 1;
}

extern DECLSPEC void SDLCALL SDL_ANDROID_CloseAudioRecording(void)
{
	JNIEnv * jniEnv = NULL;
	jclass JavaAudioThreadClass = NULL;
	jmethodID JavaStopRecording = NULL;

	(*jniVM)->AttachCurrentThread( jniVM, &jniEnv, NULL );
	JavaAudioThreadClass = (*jniEnv)->GetObjectClass( jniEnv, JavaAudioThread );
	JavaStopRecording = (*jniEnv)->GetMethodID( jniEnv, JavaAudioThreadClass, "stopRecording", "()V" );

	(*jniEnv)->CallVoidMethod( jniEnv, JavaAudioThread, JavaStopRecording );
	if( recordingBufferJNI )
		(*jniEnv)->DeleteGlobalRef( jniEnv, recordingBufferJNI );
	recordingBufferJNI = NULL;
	recordingBufferSize = 0;
}

extern DECLSPEC JavaVM* SDL_ANDROID_JavaVM()
{
	return jniVM;
}
