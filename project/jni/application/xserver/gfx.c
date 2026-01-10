#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/sockios.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/vfs.h>
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_screenkeyboard.h>
#include <SDL/SDL_android.h>
#include <savepng.h>
#include <android/log.h>

#include "gfx.h"

static TTF_Font* sFont;

static int unpackFinished = 0;
enum { UPGRADE_WARNING_NONE, UPGRADE_WARNING_ASK, UPGRADE_WARNING_PROCEED, UPGRADE_WARNING_CANCEL };
// Mutex here would be nice, but I'm lazy and don't care
static int upgradeWarning = UPGRADE_WARNING_NONE;
static char unpackLog[4][256];
static int freeSpaceRequiredMb = 0;

static void renderString(const char *c, int x, int y);
static void renderStringColor(const char *c, int x, int y, int r, int g, int b, SDL_Surface * surf);
static void renderStringScaled(const char *c, int size, int x, int y, int r, int g, int b, SDL_Surface * surf);

static void * unpackFilesThread(void * unused);
static void showErrorMessage(const char *msg);

static int unpackFiles(const char *archive, const char *script, const char *deleteOldDataMarkerFile, const char *pathsToDelete)
{
	int unpackProgressMb;
	int unpackProgressMbTotal = 1;
	char fname[PATH_MAX*3];
	char fname2[PATH_MAX*2];
	char buf[1024 * 4];
	struct stat st;
	const char *tarExtractCommand = "tar xz -C";
	char tarxz[PATH_MAX];

	if( stat( archive, &st ) == 0 )
	{
		unpackProgressMbTotal = st.st_size / 1024 / 1024;
		if( unpackProgressMbTotal <= 0 )
			unpackProgressMbTotal = 1;
		__android_log_print(ANDROID_LOG_INFO, "XSDL", "Unpacking data: total size %d Mb", unpackProgressMbTotal);
	}
	else
	{
		if (strstr(archive, ".tar.gz") == archive + strlen(archive) - strlen(".tar.gz"))
		{
			strcpy(tarxz, archive);
			strstr(tarxz, ".tar.gz")[5] = 'x'; // .tar.gz -> .tar.xz
			if( stat( tarxz, &st ) == 0 )
			{
				__android_log_print(ANDROID_LOG_INFO, "XSDL", "Found .tar.xz archive: %s", tarxz);
				archive = tarxz;
				tarExtractCommand = "tar xJ -C";
				unpackProgressMbTotal = st.st_size / 1024 / 1024;
				if( unpackProgressMbTotal <= 0 )
					unpackProgressMbTotal = 1;
				__android_log_print(ANDROID_LOG_INFO, "XSDL", "Unpacking data: total size %d Mb", unpackProgressMbTotal);
			}
			else
				return 1;
		}
	}

	unpackProgressMb = 0;

	strcpy( fname2, getenv("SECURE_STORAGE_DIR") );
	strcat( fname2, "/" );
	strcat( fname2, deleteOldDataMarkerFile );

	if( strlen(deleteOldDataMarkerFile) > 0 && stat( fname2, &st ) == 0 )
	{
		__android_log_print(ANDROID_LOG_INFO, "XSDL", "Upgrade detected, showing warning dialog");
		//upgradeWarning = UPGRADE_WARNING_PROCEED;
		upgradeWarning = UPGRADE_WARNING_CANCEL;
		/*
		upgradeWarning = UPGRADE_WARNING_ASK;
		while( upgradeWarning == UPGRADE_WARNING_ASK )
			SDL_Delay(200);
		*/
		if( upgradeWarning == UPGRADE_WARNING_CANCEL )
			return 1;
		__android_log_print(ANDROID_LOG_INFO, "XSDL", "Deleting old installation...");
		sprintf(unpackLog[0], "Deleting old installation...");

		strcpy( fname, getenv("SECURE_STORAGE_DIR") );
		strcat( fname, "/busybox" );
		strcat( fname, " sh -c 'cd " );
		strcat( fname, getenv("SECURE_STORAGE_DIR") );
		strcat( fname, " ; rm -rf " );
		strcat( fname, pathsToDelete );
		strcat( fname, "'" );

		__android_log_print(ANDROID_LOG_INFO, "XSDL", "%s", fname);

		system( fname );
	}

	for (;;)
	{
		struct statfs freeSpace;
		memset(&freeSpace, 0, sizeof(freeSpace));
		if( statfs(getenv("SECURE_STORAGE_DIR"), &freeSpace) == 0 )
		{
			if( (uint64_t)freeSpace.f_bsize * (uint64_t)freeSpace.f_bavail < (uint64_t)freeSpaceRequiredMb * 1024 * 1024 )
			{
				sprintf(unpackLog[0], "Error: not enough free space on internal storage");
				sprintf(unpackLog[1], "Available %llu Mb, required %d Mb", (long long)freeSpace.f_bsize * freeSpace.f_bavail / 1024 / 1024, freeSpaceRequiredMb);
				sprintf(unpackLog[2], "Uninstall large apps to free more space on internal storage");
				sleep(1);
				continue;
			}
		}
		sprintf(unpackLog[1], " ");
		sprintf(unpackLog[2], " ");
		break;
	}

	sprintf(unpackLog[0], "Unpacking data: %s", archive);
	__android_log_print(ANDROID_LOG_INFO, "XSDL", "Unpacking data: %s", archive);

	strcpy( fname, getenv("SECURE_STORAGE_DIR") );
	strcat( fname, "/busybox" );
	strcat( fname, " " );
	strcat( fname, tarExtractCommand );
	strcat( fname, " " );
	strcat( fname, getenv("SECURE_STORAGE_DIR") );
	FILE * fo = popen(fname, "w");
	FILE * ff = fopen(archive, "rb");
	if( !ff || !fo )
	{
		__android_log_print(ANDROID_LOG_INFO, "XSDL", "Error extracting data");
		sprintf(unpackLog[0], "Error extracting data");
		return 0;
	}
	__android_log_print(ANDROID_LOG_INFO, "XSDL", "POPEN OK");

	int unpackProgressKb = 0;
	for(;;)
	{
		//__android_log_print(ANDROID_LOG_INFO, "XSDL", "FREAD %d", unpackProgressKb);
		int cnt = fread( buf, 1, sizeof(buf), ff );
		//__android_log_print(ANDROID_LOG_INFO, "XSDL", "FREAD %d READ %d", unpackProgressKb, cnt);
		if( cnt < 0 )
		{
			__android_log_print(ANDROID_LOG_INFO, "XSDL", "Error extracting data");
			sprintf(unpackLog[0], "Error extracting data");
			return 1;
		}
		fwrite( buf, 1, cnt, fo );
		if( cnt < sizeof(buf) )
			break;
		unpackProgressKb += 4;
		if( unpackProgressKb >= 1024 )
		{
			unpackProgressKb = 0;
			unpackProgressMb++;
			sprintf(unpackLog[0], "Unpacking data: %d/%d Mb, %d%%", unpackProgressMb, unpackProgressMbTotal, unpackProgressMb * 100 / (unpackProgressMbTotal > 0 ? unpackProgressMbTotal : 1));
		}
	}
	__android_log_print(ANDROID_LOG_INFO, "XSDL", "FREAD %d Mb DONE", unpackProgressMb);

	fclose(ff);
	if( pclose(fo) != 0 ) // Returns error on Android 2.3 emulator!
	{
		__android_log_print(ANDROID_LOG_INFO, "XSDL", "Error extracting data - pclose() returned error, ignoring that error");
		//return 0;
	}

	__android_log_print(ANDROID_LOG_INFO, "XSDL", "Extracting data finished");

	remove(archive);

	if( strlen(script) == 0 )
	{
		__android_log_print(ANDROID_LOG_INFO, "XSDL", "No postinstall script");
		return 1;
	}

	sprintf(unpackLog[0], "Running postinstall script: %s", script);

	strcpy( fname, getenv("SECURE_STORAGE_DIR") );
	strcat( fname, "/" );
	strcat( fname, script );
	if( stat( fname, &st ) != 0 )
	{
		__android_log_print(ANDROID_LOG_INFO, "XSDL", "Cannot find postinstall script");
		return 1;
	}

	__android_log_print(ANDROID_LOG_INFO, "XSDL", "Running postinstall scipt: %s", script);

	fo = popen(fname, "r");
	if( !fo )
	{
		__android_log_print(ANDROID_LOG_INFO, "XSDL", "ERROR: Cannot launch postinstall script");
		return 0;
	}
	for(;;)
	{
		if( !fgets(buf, sizeof(buf), fo) )
			break;
		if( strchr(buf, '\n') != NULL )
			strchr(buf, '\n')[0] = 0;
		__android_log_print(ANDROID_LOG_VERBOSE, "XSDL", "> %s", buf);
		strncpy(unpackLog[3], unpackLog[2], sizeof(unpackLog[0]) - 4);
		strncpy(unpackLog[2], unpackLog[1], sizeof(unpackLog[0]) - 4);
		strncpy(unpackLog[1], unpackLog[0], sizeof(unpackLog[0]) - 4);
		strncpy(unpackLog[0], buf, sizeof(unpackLog[0]) - 4);
	}

	__android_log_print(ANDROID_LOG_INFO, "XSDL", "Postinstall script exited with status %d", pclose(fo));
	sprintf(unpackLog[0], "Running postinstall script finished");

	return 1;
}

static void * unpackFilesThread(void * unused)
{
	const char *unpack[][4] =
	{
		{ "data.tar.gz", "usr/bin/postinstall.sh", "usr/lib/xorg/protocol.txt", "img img-* postinstall.sh update*.sh" },
		{ "xfonts.tar.gz", "", "", "" },
		{ "update1.tar.gz", "update1.sh", "", "" },
		{ "update2.tar.gz", "update2.sh", "", "" },
		{ "update3.tar.gz", "update3.sh", "", "" },
		{ "update4.tar.gz", "update4.sh", "", "" },
		{ "update5.tar.gz", "update5.sh", "", "" },
		{ "update6.tar.gz", "update6.sh", "", "" },
		{ "update7.tar.gz", "update7.sh", "", "" },
		{ "update8.tar.gz", "update8.sh", "", "" },
		{ "update9.tar.gz", "update9.sh", "", "" },
		{NULL, NULL, NULL, NULL}
	};
	int i;

	for( i = 0; unpack[i][0] != NULL; i++ )
	{
		int status = unpackFiles(unpack[i][0], unpack[i][1], unpack[i][2], unpack[i][3]);
		if( status == 0 && i == 0 ) // Only the first archive is mandatory
		{
			unpackFinished = 1;
			return (void *)0;
		}
	}

	unpackFinished = 1;
	return (void *)1;
}

static void symlinkUsrBin(void)
{
	char libname[PATH_MAX];
	char targetname[PATH_MAX];
	char libpath[PATH_MAX];
	char targetpath[PATH_MAX];
	char mappingpath[PATH_MAX];
	FILE *mapping;

	sprintf( mappingpath, "%s/bin-map-%s.txt", getenv("DATADIR"), XSDL_ARCH );
	__android_log_print(ANDROID_LOG_INFO, "XSDL", "Opening %s", mappingpath);
	mapping = fopen(mappingpath, "rb");
	if (!mapping)
	{
		__android_log_print(ANDROID_LOG_INFO, "XSDL", "Opening %s failed", mappingpath);
		return;
	}

	sprintf( targetpath, "%s/usr", getenv("APPDIR") );
	mkdir(targetpath, 0700);

	sprintf( targetpath, "%s/usr/bin", getenv("APPDIR") );
	mkdir(targetpath, 0700);

	while (fgets(libname, sizeof(libname), mapping) &&
			fgets(targetname, sizeof(targetname), mapping))
	{
		if (strchr(libname, '\n'))
			strchr(libname, '\n')[0] = 0;
		if (strchr(targetname, '\n'))
			strchr(targetname, '\n')[0] = 0;
		sprintf( libpath, "%s/%s", getenv("LIBDIR"), libname );
		sprintf( targetpath, "%s/usr/bin/%s", getenv("APPDIR"), targetname );
		__android_log_print(ANDROID_LOG_INFO, "XSDL", "ln -s %s %s", libpath, targetpath);
		remove(targetpath);
		symlink(libpath, targetpath);
	}
	fclose(mapping);

	sprintf(targetpath, "%s/busybox", getenv("APPDIR"));
	remove(targetpath);
	symlink("usr/bin/busybox", targetpath);
	__android_log_print(ANDROID_LOG_INFO, "XSDL", "ln -s usr/bin/busybox %s", targetpath);

	// Reinstalling the app with the same version number will change LIBDIR, so update symlinks on each start
	//remove(mappingpath);
}

void XSDL_unpackFiles(int _freeSpaceRequiredMb)
{
	symlinkUsrBin();

	pthread_t thread_id;
	void * status;
	memset(unpackLog, 0, sizeof(unpackLog));
	freeSpaceRequiredMb = _freeSpaceRequiredMb;
	pthread_create(&thread_id, NULL, &unpackFilesThread, NULL);
	int progress = 0;
	enum {PROGRESS_WHEEL_NUM = 14};
	const char *progressWheel[PROGRESS_WHEEL_NUM] =
	{
		"[<=>=======]",
		"[=<=>======]",
		"[==<=>=====]",
		"[===<=>====]",
		"[====<=>===]",
		"[=====<=>==]",
		"[======<=>=]",
		"[=======<=>]",
		"[======<=>=]",
		"[=====<=>==]",
		"[====<=>===]",
		"[===<=>====]",
		"[==<=>=====]",
		"[=<=>======]",
	};
	/*
	const char *progressWheel[PROGRESS_WHEEL_NUM] =
	{
		"°¤ø,¸˳¸,ø¤°´˚´°¤ø,¸˳¸,ø¤°´˚´°¤ø,¸˳¸,ø¤°´˚´°¤ø,¸˳¸,ø¤°´˚´°¤ø,¸˳¸,ø¤°´˚´",
		"´°¤ø,¸˳¸,ø¤°´˚´°¤ø,¸˳¸,ø¤°´˚´°¤ø,¸˳¸,ø¤°´˚´°¤ø,¸˳¸,ø¤°´˚´°¤ø,¸˳¸,ø¤°´˚",
		"˚´°¤ø,¸˳¸,ø¤°´˚´°¤ø,¸˳¸,ø¤°´˚´°¤ø,¸˳¸,ø¤°´˚´°¤ø,¸˳¸,ø¤°´˚´°¤ø,¸˳¸,ø¤°´",
		"´˚´°¤ø,¸˳¸,ø¤°´˚´°¤ø,¸˳¸,ø¤°´˚´°¤ø,¸˳¸,ø¤°´˚´°¤ø,¸˳¸,ø¤°´˚´°¤ø,¸˳¸,ø¤°",
		"°´˚´°¤ø,¸˳¸,ø¤°´˚´°¤ø,¸˳¸,ø¤°´˚´°¤ø,¸˳¸,ø¤°´˚´°¤ø,¸˳¸,ø¤°´˚´°¤ø,¸˳¸,ø¤",
		"¤°´˚´°¤ø,¸˳¸,ø¤°´˚´°¤ø,¸˳¸,ø¤°´˚´°¤ø,¸˳¸,ø¤°´˚´°¤ø,¸˳¸,ø¤°´˚´°¤ø,¸˳¸,ø",
		"ø¤°´˚´°¤ø,¸˳¸,ø¤°´˚´°¤ø,¸˳¸,ø¤°´˚´°¤ø,¸˳¸,ø¤°´˚´°¤ø,¸˳¸,ø¤°´˚´°¤ø,¸˳¸,",
		",ø¤°´˚´°¤ø,¸˳¸,ø¤°´˚´°¤ø,¸˳¸,ø¤°´˚´°¤ø,¸˳¸,ø¤°´˚´°¤ø,¸˳¸,ø¤°´˚´°¤ø,¸˳¸",
		"¸,ø¤°´˚´°¤ø,¸˳¸,ø¤°´˚´°¤ø,¸˳¸,ø¤°´˚´°¤ø,¸˳¸,ø¤°´˚´°¤ø,¸˳¸,ø¤°´˚´°¤ø,¸˳",
		"˳¸,ø¤°´˚´°¤ø,¸˳¸,ø¤°´˚´°¤ø,¸˳¸,ø¤°´˚´°¤ø,¸˳¸,ø¤°´˚´°¤ø,¸˳¸,ø¤°´˚´°¤ø,¸",
		"¸˳¸,ø¤°´˚´°¤ø,¸˳¸,ø¤°´˚´°¤ø,¸˳¸,ø¤°´˚´°¤ø,¸˳¸,ø¤°´˚´°¤ø,¸˳¸,ø¤°´˚´°¤ø,",
		",¸˳¸,ø¤°´˚´°¤ø,¸˳¸,ø¤°´˚´°¤ø,¸˳¸,ø¤°´˚´°¤ø,¸˳¸,ø¤°´˚´°¤ø,¸˳¸,ø¤°´˚´°¤ø",
		"ø,¸˳¸,ø¤°´˚´°¤ø,¸˳¸,ø¤°´˚´°¤ø,¸˳¸,ø¤°´˚´°¤ø,¸˳¸,ø¤°´˚´°¤ø,¸˳¸,ø¤°´˚´°¤",
		"¤ø,¸˳¸,ø¤°´˚´°¤ø,¸˳¸,ø¤°´˚´°¤ø,¸˳¸,ø¤°´˚´°¤ø,¸˳¸,ø¤°´˚´°¤ø,¸˳¸,ø¤°´˚´°",
	};
	*/
	SDL_Joystick * j0 = SDL_JoystickOpen(0);
	while (!unpackFinished)
	{
		SDL_Delay(300);
		SDL_FillRect(SDL_GetVideoSurface(), NULL, 0);
		renderString(unpackLog[0], VID_X/2, VID_Y*2/8);
		renderString(unpackLog[1], VID_X/2, VID_Y*3/8);
		renderString(unpackLog[2], VID_X/2, VID_Y*4/8);
		renderString(unpackLog[3], VID_X/2, VID_Y*5/8);
		progress++;
		renderString(progressWheel[progress % PROGRESS_WHEEL_NUM], VID_X/2, VID_Y*6/8);
		renderString("Unpacking data ...", VID_X/2, VID_Y*7/8);
		SDL_Flip(SDL_GetVideoSurface());
		int x = 0, y = 0;
		while( upgradeWarning == UPGRADE_WARNING_ASK )
		{
			SDL_Delay(100);
			SDL_FillRect(SDL_GetVideoSurface(), NULL, 0);
			char s[PATH_MAX];
			sprintf(s, "New update available for %s", getenv("ANDROID_APP_NAME"));
			renderString(s, VID_X/2, VID_Y*2/8);
			sprintf(s, "Please move all your %s files to SD card", getenv("ANDROID_APP_NAME"));
			renderString(s, VID_X/2, VID_Y*3/8);
			renderString("or they will be deleted during upgrade", VID_X/2, VID_Y*4/8);

			renderString("――――――――――――――――――――",  VID_X/4, VID_Y*11/16);
			renderString("|                   |", VID_X/4, VID_Y*45/64);
			renderString("|                   |", VID_X/4, VID_Y*48/64);
			renderString("|                   |", VID_X/4, VID_Y*101/128);
			renderString("――――――――――――――――――――",  VID_X/4, VID_Y*13/16);
			renderString("Install now",           VID_X/4, VID_Y*6/8);

			renderString("――――――――――――――――――――",  VID_X*3/4, VID_Y*11/16);
			renderString("|                   |", VID_X*3/4, VID_Y*45/64);
			renderString("|                   |", VID_X*3/4, VID_Y*48/64);
			renderString("|                   |", VID_X*3/4, VID_Y*101/128);
			renderString("――――――――――――――――――――",  VID_X*3/4, VID_Y*13/16);
			renderString("Install later",         VID_X*3/4, VID_Y*6/8);
			SDL_Event event;
			while (SDL_PollEvent(&event))
			{
				switch (event.type)
				{
					case SDL_KEYDOWN:
						if (event.key.keysym.sym == SDLK_HELP)
							upgradeWarning = UPGRADE_WARNING_CANCEL;
					break;
					case SDL_MOUSEBUTTONUP:
						if( y > VID_Y*5/8 && y < VID_Y*7/8 )
							upgradeWarning = (x > VID_X/2) ? UPGRADE_WARNING_CANCEL : UPGRADE_WARNING_PROCEED;
					break;
					case SDL_JOYBALLMOTION:
						x = event.jball.xrel;
						y = event.jball.yrel;
					break;
				}
			}
			SDL_Flip(SDL_GetVideoSurface());
		}
	}

	pthread_join(thread_id, &status);
	if( status == 0 )
	{
		showErrorMessage("Cannot unpack data files, please reinstall the app");
		exit(1);
	}
	SDL_JoystickClose(j0);
}

void XSDL_showConfigMenu(int * resolutionW, int * displayW, int * resolutionH, int * displayH, int * builtinKeyboard, int * ctrlAltShiftKeys, char * portStr, int * pulseAudio)
{
	int x = 0, y = 0, i, ii;
	SDL_Event event;
	int res = -1, dpi = -1;
	int customX = 1000, customY = 1000;
	int port = atoi(portStr + 1);
	enum { MODE_CUSTOM = 11 };
	char native[32] = "0x0", native56[32], native46[32], native36[32], native26[32];
	char custom[32] = "1000x1000";
	int vertical = SDL_ListModes(NULL, 0)[0]->w < SDL_ListModes(NULL, 0)[0]->h;
	char cfgpath[PATH_MAX];
	FILE * cfgfile;
	int okay = 0;

	if( vertical )
	{
		x = *resolutionW;
		*resolutionW = *resolutionH;
		*resolutionH = x;
		x = *displayW;
		*displayW = *displayH;
		*displayH = x;
	}

	const char * resStr[] =
	{
		native, native56, native46, native36,
		native26, "1280x1024", "1280x960", "1280x720",
		"1024x768", "800x600", "800x480", custom
	};
	const int resVal[][2] = {
		{*resolutionW, *resolutionH},
		{(*resolutionW * 5 / 6) & ~0x3, (*resolutionH * 5 / 6) & ~0x3},
		{(*resolutionW * 4 / 6) & ~0x3, (*resolutionH * 4 / 6) & ~0x3},
		{(*resolutionW * 3 / 6) & ~0x3, (*resolutionH * 3 / 6) & ~0x3},
		{(*resolutionW * 2 / 6) & ~0x3, (*resolutionH * 2 / 6) & ~0x3},
		{1280,1024}, {1280,960}, {1280,720},
		{1024,768}, {800,600}, {800,480}, {customX,customY}
	};

	const char * fontsStr[] = {
		"X2.5", "X2", "X1.7", "X1.5",
		"X1.3", "X1", "X0.9", "X0.8",
		"X0.7", "X0.6", "X0.5", "X0.4",
		"X0.3", "X0.2", "X0.15", "X0.1"
	};
	const float fontsVal[] = {
		2.5f, 2.0f, 1.7f, 1.5f,
		1.3f, 1.0f, 0.9f, 0.8f,
		0.7f, 0.6f, 0.5f, 0.4f,
		0.3f, 0.2f, 0.15f, 0.1f
	};

	sprintf(native, "%dx%d", resVal[0][0], resVal[0][1]);
	sprintf(native56, "%dx%d", resVal[1][0], resVal[1][1]);
	sprintf(native46, "%dx%d", resVal[2][0], resVal[2][1]);
	sprintf(native36, "%dx%d", resVal[3][0], resVal[3][1]);
	sprintf(native26, "%dx%d", resVal[4][0], resVal[4][1]);

	int savedRes = 0;
	int savedDpi = 8;

	sprintf(cfgpath, "%s/.xsdl.cfg", getenv("SECURE_STORAGE_DIR"));
	cfgfile = fopen(cfgpath, "r");
	if( cfgfile )
	{
		fscanf(cfgfile, "%d %d %d %d %d %d %d %d", &savedRes, &savedDpi, &customX, &customY, builtinKeyboard, ctrlAltShiftKeys, &port, pulseAudio);
		fclose(cfgfile);
		if (strcmp(portStr, ":0") != 0)
		{
			port = atoi(portStr + 1);
		}
	}
	sprintf(custom, "%dx%d", customX, customY);
	sprintf(portStr, ":%d", port);

	int counter = 3000, config = 0;
	Uint32 curtime = SDL_GetTicks();
	while ( counter > 0 && !config )
	{
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
				case SDL_KEYDOWN:
					if (event.key.keysym.sym == SDLK_HELP)
						counter = 0;
				break;
				case SDL_MOUSEBUTTONUP:
					config = 1;
				break;
			}
		}
		SDL_FillRect(SDL_GetVideoSurface(), NULL, 0);
		y = VID_Y/4;
		renderString("Tap the screen to change", vertical ? VID_Y / 2 : VID_X/2, y);
		y += 30;
		renderString("display resolution and font scale (DPI)", vertical ? VID_Y / 2 : VID_X/2, y);
		char buf[100];
		y += 30;
		sprintf(buf, "Resolution: %s", resStr[savedRes]);
		renderString(buf, vertical ? VID_Y / 2 : VID_X/2, y);
		y += 30;
		sprintf(buf, "Font scale: %s", fontsStr[savedDpi]);
		renderString(buf, vertical ? VID_Y / 2 : VID_X/2, y);
		y += 30;
		sprintf(buf, "Keyboard: %s", *builtinKeyboard == 0 ? "System" : *builtinKeyboard == 1 ? "Builtin QWERTY" : "System + Builtin");
		renderString(buf, vertical ? VID_Y / 2 : VID_X/2, y);
		y += 30;
		sprintf(buf, "Display number: %d", port);
		renderString(buf, vertical ? VID_Y / 2 : VID_X/2, y);
		y += 30;
		sprintf(buf, "Ctrl/Alt/Shift overlay: %s", *ctrlAltShiftKeys == 0 ? "No" : *ctrlAltShiftKeys == 1 ? "Yes, left side" : "Yes, right side");
		renderString(buf, vertical ? VID_Y / 2 : VID_X/2, y);
		y += 40;
		sprintf(buf, "Starting in %d seconds", counter / 1000 + 1);
		renderString(buf, vertical ? VID_Y / 2 : VID_X/2, y);
		SDL_Delay(50);
		SDL_Flip(SDL_GetVideoSurface());
		counter -= SDL_GetTicks() - curtime;
		curtime = SDL_GetTicks();
	}

	if( !config )
	{
		res = savedRes;
		dpi = savedDpi;
	}

	SDL_Joystick * j0 = SDL_JoystickOpen(0);
	int mouse = 0;

	x = 0;
	y = 0;
	while ( res < 0 )
	{
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
				case SDL_KEYDOWN:
					if (event.key.keysym.sym == SDLK_HELP)
						return;
				break;
				case SDL_MOUSEBUTTONUP:
				{
					if( vertical )
					{
						int z = x;
						x = y;
						y = z;
					}
					i = (y / (VID_Y/3));
					ii = (x / (VID_X/4));
					res = i * 4 + ii;
					__android_log_print(ANDROID_LOG_INFO, "XSDL", "Screen coords %d %d res %d\n", x, y, res);
				}
				break;
				case SDL_MOUSEMOTION:
					if (mouse > 2)
					{
						x = event.motion.x;
						y = event.motion.y;
					}
					else
					{
						mouse++;
					}
				break;
				case SDL_JOYBALLMOTION:
					mouse = 0;
					x = event.jball.xrel;
					y = event.jball.yrel;
				break;
			}
		}

		//__android_log_print(ANDROID_LOG_INFO, "XSDL", "Screen coords %d %d\n", x, y, res);
		SDL_FillRect(SDL_GetVideoSurface(), NULL, 0);
		renderString("Select display resolution", vertical ? VID_Y / 2 : VID_X/2, VID_Y/3);
		for(i = 0; i < 3; i++)
		for(ii = 0; ii < 4; ii++)
		{
			if( vertical )
				renderString(resStr[i*4+ii], VID_Y/6 + (i*VID_Y/3), VID_X/8 + (ii*VID_X/4));
			else
				renderString(resStr[i*4+ii], VID_X/8 + (ii*VID_X/4), VID_Y/6 + (i*VID_Y/3));
			if( i == 0 && ii == 0 && !vertical )
				renderString("native", VID_X/8, VID_Y/6 - VID_Y/12);
			if( i == 2 && ii == 3 && !vertical )
				renderString("custom", VID_X/8 + (ii*VID_X/4), VID_Y/6 - VID_Y/12 + (i*VID_Y/3));
		}

		renderString("∆", x, y);
		SDL_Delay(50);
		SDL_Flip(SDL_GetVideoSurface());
		if (res == MODE_CUSTOM)
		{
			__android_log_print(ANDROID_LOG_INFO, "XSDL", "Selected custom display resolution");
			SDL_ANDROID_ToggleScreenKeyboardWithoutTextInput();
			customX = 0;
			customY = 0;
			custom[0] = 0;
			while (customX == 0 || customY == 0)
			{
				while (SDL_PollEvent(&event))
				{
					if (event.type == SDL_KEYDOWN)
					{
						switch (event.key.keysym.sym)
						{
							case SDLK_HELP:
								return;
							case SDLK_RETURN:
								if (customX == 0)
									customX = atoi(custom);
								else
									customY = atoi(custom);
								custom[0] = 0;
								break;
							case SDLK_BACKSPACE:
								if (strlen(custom) > 0)
									custom[strlen(custom) - 1] = 0;
								break;
							case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
								custom[strlen(custom) + 1] = 0;
								custom[strlen(custom)] = event.key.keysym.sym;
								break;
							default:
								break;
						}
					}
				}
				SDL_FillRect(SDL_GetVideoSurface(), NULL, 0);
				if (customX == 0)
					renderString("Enter width:", VID_X/8, VID_Y/6);
				else
					renderString("Enter height:", VID_X/8, VID_Y/6);
				renderString("Press Enter when done", VID_X*3/4, VID_Y/6);
				renderString(custom, VID_X/8 + VID_X/4, VID_Y/6);
				SDL_Delay(50);
				SDL_Flip(SDL_GetVideoSurface());
			}
			__android_log_print(ANDROID_LOG_INFO, "XSDL", "Selected custom display resolution: %s = %d %d", custom, customX, customY);
		}
	}
	*resolutionW = resVal[res][0];
	*resolutionH = resVal[res][1];
	if (res == MODE_CUSTOM)
	{
		*resolutionW = customX;
		*resolutionH = customY;
	}

	x = 0;
	y = 0;
	while ( dpi < 0 )
	{
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
				case SDL_KEYDOWN:
					if (event.key.keysym.sym == SDLK_HELP)
						return;
				break;
				case SDL_MOUSEBUTTONUP:
				{
					if( vertical )
					{
						int z = x;
						x = y;
						y = z;
					}
					i = (y / (VID_Y/4));
					ii = (x / (VID_X/4));
					dpi = i * 4 + ii;
					__android_log_print(ANDROID_LOG_INFO, "XSDL", "Screen coords %d %d dpi %d\n", x, y, dpi);
				}
				break;
				case SDL_MOUSEMOTION:
					if (mouse > 2)
					{
						x = event.motion.x;
						y = event.motion.y;
					}
					else
					{
						mouse++;
					}
				break;
				case SDL_JOYBALLMOTION:
					mouse = 0;
					x = event.jball.xrel;
					y = event.jball.yrel;
				break;
			}
		}
		SDL_FillRect(SDL_GetVideoSurface(), NULL, 0);
		renderString("Select font scale (DPI)", vertical ? VID_Y / 2 : VID_X/2, VID_Y/2);
		for(i = 0; i < 4; i++)
		for(ii = 0; ii < 4; ii++)
		{
			int scale = (float)(*displayH) * (float)(*resolutionH) / 3500.0f * fontsVal[i*4+ii];
			if( vertical )
				renderStringScaled(fontsStr[i*4+ii], scale < VID_Y/12 ? scale : VID_Y/12, VID_Y/8 + (i*VID_Y/4), VID_X/8 + (ii*VID_X/4), 255, 255, 255, SDL_GetVideoSurface());
			else
				renderStringScaled(fontsStr[i*4+ii], scale, VID_X/8 + (ii*VID_X/4), VID_Y/8 + (i*VID_Y/4), 255, 255, 255, SDL_GetVideoSurface());
		}

		renderString("∆", x, y);
		SDL_Delay(50);
		SDL_Flip(SDL_GetVideoSurface());
	}
	*displayW = *displayW / fontsVal[dpi];
	*displayH = *displayH / fontsVal[dpi];

	x = 0;
	y = 0;
	okay = !config;
	while ( !okay )
	{
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
				case SDL_KEYDOWN:
					if (event.key.keysym.sym == SDLK_HELP)
						return;
				break;
				case SDL_MOUSEBUTTONUP:
				{
					if( vertical )
					{
						int z = x;
						x = y;
						y = z;
					}
					if( y > 0 && y < VID_Y * 1.5f / 6 )
					{
						*builtinKeyboard = (*builtinKeyboard + 1) % 3;
					}
					if( y > VID_Y * 1.5f / 6 &&  y < VID_Y * 2.5f / 6 )
					{
						*ctrlAltShiftKeys = *ctrlAltShiftKeys + 1;
						if (*ctrlAltShiftKeys > 2)
						{
							*ctrlAltShiftKeys = 0;
						}
					}
					if( y > VID_Y * 2.5f / 6 &&  y < VID_Y * 3.5f / 6 )
					{
						port ++;
						port %= 4;
					}
					if( y > VID_Y * 3.5f / 6 &&  y < VID_Y * 4.5f / 6 )
					{
						*pulseAudio = !(*pulseAudio);
					}
					if( y > VID_Y * 4.5 / 6 &&  y < VID_Y * 6 / 6 )
					{
						okay = 1;
					}
					__android_log_print(ANDROID_LOG_INFO, "XSDL", "Screen coords %d %d\n", x, y);
					x = 0;
					y = 0;
				}
				break;
				case SDL_MOUSEMOTION:
					if (mouse > 2)
					{
						x = event.motion.x;
						y = event.motion.y;
					}
					else
					{
						mouse++;
					}
				break;
				case SDL_JOYBALLMOTION:
					mouse = 0;
					x = event.jball.xrel;
					y = event.jball.yrel;
				break;
			}
		}
		SDL_FillRect(SDL_GetVideoSurface(), NULL, 0);

		char buf[100];

		sprintf(buf, "Keyboard: %s", *builtinKeyboard == 0 ? "System" : *builtinKeyboard == 1 ? "Builtin QWERTY" : "System + Builtin");
		renderString(buf, VID_X/2, VID_Y * 1 / 6);

		sprintf(buf, "Ctrl/Alt/Shift overlay: %s", *ctrlAltShiftKeys == 0 ? "No" : *ctrlAltShiftKeys == 1 ? "Yes, left side" : "Yes, right side");
		renderString(buf, VID_X/2, VID_Y * 2 / 6);

		sprintf(buf, "Display number: %d", port);
		renderString(buf, VID_X/2, VID_Y * 3 / 6);

		sprintf(buf, "PulseAudio: %s", *pulseAudio ? "Yes" : "No");
		renderString(buf, VID_X/2, VID_Y * 4 / 6);

		sprintf(buf, "Okay");
		renderString(buf, VID_X/2, VID_Y * 5 / 6);

		renderString("∆", x, y);
		SDL_Delay(50);
		SDL_Flip(SDL_GetVideoSurface());
	}

	SDL_JoystickClose(j0);

	if( config )
	{
		cfgfile = fopen(cfgpath, "w");
		if( cfgfile )
		{
			fprintf(cfgfile, "%d %d %d %d %d %d %d %d\n", res, dpi, customX, customY, *builtinKeyboard, *ctrlAltShiftKeys, port, *pulseAudio);
			fclose(cfgfile);
		}
	}
	sprintf(portStr, ":%d", port);
}

void XSDL_generateBackground(const char * port, int showHelp, int resolutionW, int resolutionH)
{
	int sd, addr, ifc_num, i;
	struct ifconf ifc;
	struct ifreq ifr[20];
	SDL_Surface * surf;
	int y = resolutionH * 30 / VID_Y;
	char msg[128];
	char clipboard[8192] = "";

	if (resolutionH > resolutionW)
		resolutionH = resolutionW;

	if( !showHelp )
	{
		surf = SDL_CreateRGBSurface(SDL_SWSURFACE, 16, 16, 24, 0x0000ff, 0x00ff00, 0xff0000, 0);
		SDL_FillRect(surf, NULL, 0x00002f);
		SDL_SavePNG(surf, "background.png");
		SDL_FreeSurface(surf);
		return;
	}

	surf = SDL_CreateRGBSurface(SDL_SWSURFACE, resolutionW, resolutionH, 24, 0x0000ff, 0x00ff00, 0xff0000, 0);
	SDL_FillRect(surf, NULL, 0x7f0000);

	renderStringScaled("To show keyboard, tap Back < or swipe from the screen edge", 12 * resolutionH / VID_Y, resolutionW/2, y, 255, 255, 255, surf);
	y += resolutionH * 30 / VID_Y;

	renderStringScaled("Launch these commands on your Linux PC:", 12 * resolutionH / VID_Y, resolutionW/2, y, 255, 255, 255, surf);
	y += resolutionH * 30 / VID_Y;

	sd = socket(PF_INET, SOCK_DGRAM, 0);
	if (sd > 0)
	{
		ifc.ifc_len = sizeof(ifr);
		ifc.ifc_ifcu.ifcu_buf = (caddr_t)ifr;

		if (ioctl(sd, SIOCGIFCONF, &ifc) == 0)
		{
			ifc_num = ifc.ifc_len / sizeof(struct ifreq);
			__android_log_print(ANDROID_LOG_INFO, "XSDL", "%d network interfaces found", ifc_num);

			for (i = 0; i < ifc_num; ++i)
			{
				int addr = 0;
				char saddr[32];
				if (ifr[i].ifr_addr.sa_family != AF_INET)
					continue;

				if (ioctl(sd, SIOCGIFADDR, &ifr[i]) == 0)
					addr = ((struct sockaddr_in *)(&ifr[i].ifr_addr))->sin_addr.s_addr;
				if (addr == 0)
					continue;
				sprintf (saddr, "%d.%d.%d.%d", (addr & 0xFF), (addr >> 8 & 0xFF), (addr >> 16 & 0xFF), (addr >> 24 & 0xFF));
				__android_log_print(ANDROID_LOG_INFO, "XSDL", "interface: %s address: %s\n", ifr[i].ifr_name, saddr);
				if (strcmp(saddr, "127.0.0.1") == 0)
					continue;
				sprintf (msg, "export DISPLAY=%s%s", saddr, port);
				renderStringScaled(msg, 12 * resolutionH / VID_Y, resolutionW/2, y, 255, 255, 255, surf);
				strcat(clipboard, msg); strcat(clipboard, "\n");
				y += resolutionH * 15 / VID_Y;
				sprintf (msg, "export PULSE_SERVER=tcp:%s:4713", saddr);
				renderStringScaled(msg, 12 * resolutionH / VID_Y, resolutionW/2, y, 255, 255, 255, surf);
				strcat(clipboard, msg); strcat(clipboard, "\n");
				y += resolutionH * 15 / VID_Y;
				sprintf (msg, "xfwm4 & firefox");
				renderStringScaled(msg, 12 * resolutionH / VID_Y, resolutionW/2, y, 255, 255, 255, surf);
				strcat(clipboard, msg); strcat(clipboard, "\n");
				y += resolutionH * 20 / VID_Y;
			}
		}

		close(sd);
	}

	SDL_SetClipboardText(clipboard);

	y += resolutionH * 10 / VID_Y;
	sprintf (msg, "To tunnel X over SSH, forward port %d", atoi(port+1) + 6000);
	renderStringScaled(msg, 12 * resolutionH / VID_Y, resolutionW/2, y, 255, 255, 255, surf);
	y += resolutionH * 15 / VID_Y;
	sprintf (msg, "in your SSH client");
	renderStringScaled(msg, 12 * resolutionH / VID_Y, resolutionW/2, y, 255, 255, 255, surf);

	y += resolutionH * 20 / VID_Y;
	sprintf (msg, "If you run Linux in chroot on this device, run:");
	renderStringScaled(msg, 12 * resolutionH / VID_Y, resolutionW/2, y, 255, 255, 255, surf);
	y += resolutionH * 15 / VID_Y;
	sprintf (msg, "export DISPLAY=:0 PULSE_SERVER=tcp:127.0.0.1:4713");
	renderStringScaled(msg, 12 * resolutionH / VID_Y, resolutionW/2, y, 255, 255, 255, surf);

	SDL_SavePNG(surf, "background.png");
	SDL_FreeSurface(surf);
}

void XSDL_showServerLaunchErrorMessage()
{
	showErrorMessage(	"Error: X server failed to launch.\n\n"
						"Try to use different display number,\n"
						"reboot your device, or reinstall the app.");
}

void showErrorMessage(const char *msg)
{
	SDL_Event event;
	const char * s;
	int y = VID_Y/3;
	SDL_FillRect(SDL_GetVideoSurface(), NULL, 0);
	for( s = msg; s && s[0]; s = strchr(s, '\n'), s += (s ? 1 : 0), y += 30 )
	{
		const char * s1 = strchr(s, '\n');
		int len = s1 ? s1 - s : strlen(s);
		char buf[512];
		strncpy(buf, s, len);
		buf[len] = 0;
		if( len > 0 )
			renderString(buf, VID_X/2, y);
	}
	SDL_Flip(SDL_GetVideoSurface());
	while (1)
	{
		while (SDL_WaitEvent(&event))
		{
			switch (event.type)
			{
				case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_HELP)
					return;
				break;
			}
		}
	}
}

void XSDL_initSDL()
{
	char fontpath[PATH_MAX];
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);

	__android_log_print(ANDROID_LOG_INFO, "XSDL", "Current video mode: %d %d", SDL_ListModes(NULL, 0)[0]->w, SDL_ListModes(NULL, 0)[0]->h);

	if( SDL_ListModes(NULL, 0)[0]->w > SDL_ListModes(NULL, 0)[0]->h )
		SDL_SetVideoMode(VID_X, VID_Y, 0, SDL_SWSURFACE);
	else
		SDL_SetVideoMode(VID_Y, VID_X, 0, SDL_SWSURFACE);
	TTF_Init();
	strcpy( fontpath, getenv("UNSECURE_STORAGE_DIR") );
	strcat( fontpath, "/DroidSansMono.ttf" );
	sFont = TTF_OpenFont(fontpath, 14);
	if (!sFont)
	{
		__android_log_print(ANDROID_LOG_INFO, "XSDL", "Error: cannot open font file, please reinstall the app");
		exit(1);
	}
}

void XSDL_deinitSDL()
{
	TTF_CloseFont(sFont);
	sFont = NULL;
	TTF_Quit();
	// Do NOT call SDL_Quit(), it crashes!
}

void renderStringColor(const char *c, int x, int y, int r, int g, int b, SDL_Surface * surf)
{
	if (!c || !c[0])
		return;
	SDL_Color fColor = {r, g, b};
	SDL_Rect fontRect = {0, 0, 0, 0};
	SDL_Surface* fontSurface = TTF_RenderUTF8_Solid(sFont, c, fColor);
	fontRect.w = fontSurface->w;
	fontRect.h = fontSurface->h;
	fontRect.x = x - fontRect.w / 2;
	fontRect.y = y - fontRect.h / 2;
	SDL_BlitSurface(fontSurface, NULL, surf, &fontRect);
	SDL_FreeSurface(fontSurface);
}

void renderString(const char *c, int x, int y)
{
	renderStringColor(c, x, y, 255, 255, 255, SDL_GetVideoSurface());
}

void renderStringScaled(const char *c, int size, int x, int y, int r, int g, int b, SDL_Surface * surf)
{
	char fontpath[PATH_MAX];
	if (!c || !c[0])
		return;
	SDL_Color fColor = {r, g, b};
	SDL_Rect fontRect = {0, 0, 0, 0};
	strcpy( fontpath, getenv("UNSECURE_STORAGE_DIR") );
	strcat( fontpath, "/DroidSansMono.ttf" );
	TTF_Font* font = TTF_OpenFont(fontpath, size);
	SDL_Surface* fontSurface = TTF_RenderUTF8_Solid(font, c, fColor);
	TTF_CloseFont(font);
	fontRect.w = fontSurface->w;
	fontRect.h = fontSurface->h;
	fontRect.x = x - fontRect.w / 2;
	fontRect.y = y - fontRect.h / 2;
	SDL_BlitSurface(fontSurface, NULL, surf, &fontRect);
	SDL_FreeSurface(fontSurface);
}
