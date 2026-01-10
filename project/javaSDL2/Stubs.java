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
import android.os.Bundle;

// Stubs for compatibility with SDL 1.2 code

class SettingsMenu {
	public static void showConfig(final MainActivity p, final boolean firstStart) {
	}
}

class RestartMainActivity extends Activity {
	// For compatibility with SDL 1.2 code
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
	}

	public static final String ACTIVITY_AUTODETECT_SCREEN_ORIENTATION = "libsdl.org.ACTIVITY_AUTODETECT_SCREEN_ORIENTATION";
	public static final String SDL_RESTART_PARAMS = "SDL_RESTART_PARAMS";
}

class DemoGLSurfaceView {
	static void SetupTouchscreenKeyboardGraphics(Activity p) {
	}
}
