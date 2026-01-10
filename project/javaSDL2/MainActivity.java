/*
Simple DirectMedia Layer
Java source code (C) 2009-2014 Sergii Pylypenko

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

package net.sourceforge.clonekeenplus;

import android.app.Activity;
import android.app.UiModeManager;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.res.Configuration;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.util.Log;
import android.widget.FrameLayout;
import java.util.ArrayList;
import java.util.Arrays;

public class MainActivity extends org.libsdl.app.SDLActivity {
	public static MainActivity instance = null;
	public String ObbMountPath = null; // Deprecated, always empty
	public String assetPackPath = null; // Not saved to the config file
	public boolean readExternalStoragePermissionDialogAnswered = false; // Deprecated, always false
	public boolean nativeThreadResumeCaptured = false;
	public DataDownloader dataDownloader = null;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		this.instance = this;
		super.onCreate(savedInstanceState);

		Globals.DataDir = this.getFilesDir().getAbsolutePath();
		Settings.settingsLoaded = Settings.LoadConfig(this); // Load Globals.DataDir from SDL 1.2 installation

		if (Settings.settingsLoaded) {
			Log.i("SDL", "libSDL: Settings.ProcessConfig(): loaded settings successfully");
			Log.i("SDL", "libSDL: old app version " + Settings.settingsAppVersion + ", new app version " + this.getApplicationVersion());
			if (Settings.settingsAppVersion != this.getApplicationVersion()) {
				Settings.DeleteFilesOnUpgrade(this);
				if (Globals.ResetSdlConfigForThisVersion) {
					Log.i("SDL", "libSDL: old app version " + Settings.settingsAppVersion + ", new app version " + this.getApplicationVersion() + " and we need to clean up config file");
					// Delete settings file, and restart the application
					Settings.DeleteSdlConfigOnUpgradeAndRestart(this);
				}
				Settings.Save(this);
			}
		}

		try
		{
			if( android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.LOLLIPOP )
			{
				ApplicationInfo info = this.getPackageManager().getApplicationInfo(this.getPackageName(), 0);
				if( info.splitSourceDirs != null )
				{
					for( String apk: info.splitSourceDirs )
					{
						Log.i("SDL", "Package apk: " + apk);
						if( apk.endsWith("assetpack.apk") )
						{
							this.assetPackPath = apk;
							Log.i("SDL", "Found asset pack: " + this.assetPackPath);
						}
					}
				}
			}
		}
		catch( Exception eee )
		{
			Log.i("SDL", "Asset pack exception: " + eee);
		}

		Settings.setEnvVars(this);
		Log.v("SDL", "chdir() to: " + Globals.DataDir);
		Settings.nativeChdir(Globals.DataDir);
		Log.i("SDL", "Starting data download");
		this.dataDownloader = new DataDownloader(this, null);
	}

	public void downloadFinishedInitSDL() {
		if (!this.dataDownloader.DownloadComplete)
		{
			Log.i("SDL", "Data download failed!");
			AlertDialog.Builder dlgAlert  = new AlertDialog.Builder(this);
			dlgAlert.setMessage(this.getResources().getString(R.string.error_dl_from, String.join(" ", Globals.DataDownloadUrl)));
			dlgAlert.setTitle("SDL Error");
			dlgAlert.setPositiveButton("Exit",
				new DialogInterface.OnClickListener() {
					@Override
					public void onClick(DialogInterface dialog,int id) {
						System.exit(0);
					}
				});
			dlgAlert.setCancelable(false);
			dlgAlert.create().show();
			return;
		}
		Log.i("SDL", "Data download finished, starting native thread");
		this.dataDownloader = null;
		if (this.nativeThreadResumeCaptured)
			this.resumeNativeThread();
		else
			this.pauseNativeThread();
	}

	@Override
	protected void pauseNativeThread() {
		Log.i("SDL", "Intercepted pauseNativeThread() from MainActivity");
		this.nativeThreadResumeCaptured = false;
		if (this.dataDownloader == null) {
			super.pauseNativeThread();
		}
	}

	@Override
	protected void resumeNativeThread() {
		Log.i("SDL", "Intercepted resumeNativeThread() from MainActivity");
		this.nativeThreadResumeCaptured = true;
		if (this.dataDownloader == null) {
			super.resumeNativeThread();
		}
	}

	@Override
	protected String[] getLibraries() {
		ArrayList<String> ret = new ArrayList<String>();
		for (String l: Globals.AppLibraries) {
			ret.add(GetMappedLibraryName(l));
		}
		for (String l: Globals.AppMainLibraries) {
			ret.add(GetMappedLibraryName(l));
		}
		Log.i("SDL", "Loading native libraries: " + String.join(" ", ret));
		return ret.toArray(new String[0]);
	}

	@Override
	protected String[] getArguments() {
		// Strip the first argument, it will get passed as commandline parameter by SDL2
		ArrayList<String> ret = new ArrayList<String>(Arrays.asList(Globals.CommandLine.split(" ")));
		if (ret.size() >= 1) {
			ret.remove(0);
		}
		Log.i("SDL", "Commandline arguments: '" + String.join("', '", ret) + "'");
		return ret.toArray(new String[0]);
	}

	private static String GetMappedLibraryName(final String s) {
		for (int i = 0; i < Globals.LibraryNamesMap.length; i++) {
			if (Globals.LibraryNamesMap[i][0].equals(s))
				return Globals.LibraryNamesMap[i][1];
		}
		return s;
	}

	public int getApplicationVersion() {
		try {
			PackageInfo packageInfo = getPackageManager().getPackageInfo(getPackageName(), 0);
			return packageInfo.versionCode;
		} catch (PackageManager.NameNotFoundException e) {
			Log.i("SDL", "libSDL: Cannot get the version of our own package: " + e);
		}
		return 0;
	}

	public boolean isRunningOnOUYA() {
		try {
			PackageInfo packageInfo = getPackageManager().getPackageInfo("tv.ouya", 0);
			return true;
		} catch (PackageManager.NameNotFoundException e) {
		}
		UiModeManager uiModeManager = (UiModeManager) getSystemService(UI_MODE_SERVICE);
		return (uiModeManager.getCurrentModeType() == Configuration.UI_MODE_TYPE_TELEVISION) || Globals.OuyaEmulation;
	}

	public static void setScreenOrientation() {
		// For compatibility with SDL 1.2 code
	}

	public static void setUpStatusLabel() {
		// For compatibility with SDL 1.2 code
	}

	public static FrameLayout getVideoLayout() {
		// For compatibility with SDL 1.2 code
		return null;
	}

}
