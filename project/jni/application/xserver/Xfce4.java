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

package x.org.server;

import android.app.Activity;
import android.app.Service;
import android.content.Context;
import android.os.Bundle;
import android.os.IBinder;
import android.view.MotionEvent;
import android.view.KeyEvent;
import android.view.Window;
import android.view.WindowManager;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;
import android.widget.EditText;
import android.text.Editable;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.FrameLayout;
import android.graphics.drawable.Drawable;
import android.graphics.Color;
import android.content.res.Configuration;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Intent;
import android.view.View.OnKeyListener;
import android.view.MenuItem;
import android.view.Menu;
import android.view.Gravity;
import android.text.method.TextKeyListener;
import java.util.LinkedList;
import java.io.SequenceInputStream;
import java.io.BufferedInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.FileOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.util.zip.*;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;
import java.util.Set;
import android.text.SpannedString;
import java.io.BufferedReader;
import java.io.BufferedInputStream;
import java.io.InputStreamReader;
import android.view.inputmethod.InputMethodManager;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.os.Handler;
import android.os.Message;
import android.os.SystemClock;
import java.util.concurrent.Semaphore;
import android.content.pm.ActivityInfo;
import android.view.Display;
import android.util.DisplayMetrics;
import android.text.InputType;
import android.util.Log;
import android.view.Surface;
import android.app.ProgressDialog;
import android.app.KeyguardManager;
import android.view.ViewTreeObserver;
import android.graphics.Rect;
import android.net.Uri;
import android.content.ComponentName;
import android.widget.Toast;
import android.content.pm.PackageManager;
import android.os.Build;
import android.content.Intent;
import android.content.pm.PackageInfo;
import androidx.activity.ComponentActivity;
import androidx.activity.result.ActivityResultLauncher;
import androidx.activity.result.contract.ActivityResultContracts;


public class Xfce4 extends ComponentActivity
{
	/** Termux package name */
	public static final String TERMUX_PACKAGE_NAME = "com.termux"; // Default: "com.termux"

	/** Android OS permission declared by Termux app in AndroidManifest.xml which can be requested by
	  * 3rd party apps to run various commands in Termux app context */
	public static final String PERMISSION_RUN_COMMAND = TERMUX_PACKAGE_NAME + ".permission.RUN_COMMAND"; // Default: "com.termux.permission.RUN_COMMAND"

	/** Termux app internal private app data directory path */
	public static final String TERMUX_INTERNAL_PRIVATE_APP_DATA_DIR_PATH = "/data/data/" + TERMUX_PACKAGE_NAME; // Default: "/data/data/com.termux"

	/** Termux app Files directory path */
	public static final String TERMUX_FILES_DIR_PATH = TERMUX_INTERNAL_PRIVATE_APP_DATA_DIR_PATH + "/files"; // Default: "/data/data/com.termux/files"

	/** Termux app run command service name. */
	public static final String RUN_COMMAND_SERVICE_NAME = TERMUX_PACKAGE_NAME + ".app.RunCommandService"; // Termux app service to receive commands from 3rd party apps "com.termux.app.RunCommandService"

	/** Intent action to execute command with RUN_COMMAND_SERVICE */
	public static final String ACTION_RUN_COMMAND = TERMUX_PACKAGE_NAME + ".RUN_COMMAND"; // Default: "com.termux.RUN_COMMAND"

	/** Intent {@code String} extra for absolute path of command for the RUN_COMMAND_SERVICE.ACTION_RUN_COMMAND intent */
	public static final String EXTRA_COMMAND_PATH = TERMUX_PACKAGE_NAME + ".RUN_COMMAND_PATH"; // Default: "com.termux.RUN_COMMAND_PATH"
	/** Intent {@code String[]} extra for arguments to the executable of the command for the RUN_COMMAND_SERVICE.ACTION_RUN_COMMAND intent */
	public static final String EXTRA_ARGUMENTS = TERMUX_PACKAGE_NAME + ".RUN_COMMAND_ARGUMENTS"; // Default: "com.termux.RUN_COMMAND_ARGUMENTS"
	/** Intent {@code String} extra for stdin of the command for the RUN_COMMAND_SERVICE.ACTION_RUN_COMMAND intent */
	public static final String EXTRA_STDIN = TERMUX_PACKAGE_NAME + ".RUN_COMMAND_STDIN"; // Default: "com.termux.RUN_COMMAND_STDIN"
	/** Intent {@code String} extra for current working directory of command for the RUN_COMMAND_SERVICE.ACTION_RUN_COMMAND intent */
	public static final String EXTRA_WORKDIR = TERMUX_PACKAGE_NAME + ".RUN_COMMAND_WORKDIR"; // Default: "com.termux.RUN_COMMAND_WORKDIR"
	/** Intent {@code String} extra for label of the command for the RUN_COMMAND_SERVICE.ACTION_RUN_COMMAND intent */
	public static final String EXTRA_COMMAND_LABEL = TERMUX_PACKAGE_NAME + ".RUN_COMMAND_COMMAND_LABEL"; // Default: "com.termux.RUN_COMMAND_COMMAND_LABEL"

	@Override
	protected void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);

		runTermuxCommand();
	}

	public void runTermuxCommand()
	{
		Log.i("SDL", "Check if Termux is installed");
		try {
			this.getPackageManager().getPackageInfo("com.termux", 0);
		} catch (PackageManager.NameNotFoundException e) {
			Log.i("SDL", "Termux is not installed, show toast");
			Toast.makeText(this, "Please install Termux from F-Droid", Toast.LENGTH_LONG).show();
			return;
		}

		Log.i("SDL", "Check Termux command permission");

		int permissionCheck = this.checkSelfPermission(PERMISSION_RUN_COMMAND);
		if (permissionCheck != PackageManager.PERMISSION_GRANTED)
		{
			Log.i("SDL", "Request Termux command permission");
			Toast.makeText(this, "Please grant permission to run commands in Termux", Toast.LENGTH_SHORT).show();
			//this.requestPermissions(new String[]{PERMISSION_RUN_COMMAND}, 0);
			final ActivityResultLauncher<String> requestPermissionLauncher =
				this.registerForActivityResult(new ActivityResultContracts.RequestPermission(), isGranted ->
				{
					if (isGranted)
					{
						runTermuxCommand();
					}
					else
					{
						Toast.makeText(this, "Termux permission denied", Toast.LENGTH_SHORT).show();
					}
				});
			requestPermissionLauncher.launch(PERMISSION_RUN_COMMAND);
			return;
		}

		Log.i("SDL", "Run XFCE4 from XSDL");

		Intent main = new Intent(this, MainActivity.class);
		main.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TOP);
		main.putExtra(RestartMainActivity.SDL_RESTART_PARAMS, "xfce4");
		startActivity(main);

		new Thread(new Runnable()
		{
			public void run()
			{
				Log.i("SDL", "Waiting for DISPLAY env vars to be set");
				while( System.getenv("DISPLAY") == null )
				{
					try {
						Thread.sleep(300);
					} catch (InterruptedException e) {}
				}
				Log.i("SDL", "DISPLAY env var set");

				Log.i("SDL", "Launching Termux with XFCE4");
				try {
					Thread.sleep(500);
				} catch (InterruptedException e) {}

				Intent termux = new Intent();
				termux.setClassName(TERMUX_PACKAGE_NAME, RUN_COMMAND_SERVICE_NAME);
				termux.setAction(ACTION_RUN_COMMAND);
				termux.putExtra(EXTRA_COMMAND_PATH, TERMUX_FILES_DIR_PATH + "/usr/bin/bash");
				termux.putExtra(EXTRA_ARGUMENTS, new String[]
					{
						"-c",
						"export DISPLAY=:0 ; " +
						"xfce4-session --help || { " +
						"apt update ; " +
						"apt install -y x11-repo ; " +
						"apt update ; " +
						"apt install -y xfce4 ; " +
						" } ; " +
						"dbus-run-session xfce4-session"
					});
				termux.putExtra(EXTRA_WORKDIR, TERMUX_FILES_DIR_PATH + "/home");
				termux.putExtra(EXTRA_COMMAND_LABEL, "Xfce4");

				startService(termux);

				Log.i("SDL", "Switching back to XSDL window");
				try {
					Thread.sleep(2000);
				} catch (InterruptedException e) {}

				Intent mainAgain = new Intent(Xfce4.this, MainActivity.class);
				mainAgain.setFlags(Intent.FLAG_ACTIVITY_REORDER_TO_FRONT);
				startActivity(mainAgain);
			}
		}).start();
	}
}
