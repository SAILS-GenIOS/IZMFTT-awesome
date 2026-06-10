package com.materializr.app;

import android.os.Build;
import android.view.View;
import org.libsdl.app.SDLActivity;

// Materializr's Android entry activity. SDLActivity does the heavy lifting:
// it creates the GL surface, loads the native libraries below, and calls
// SDL_main (defined in android_main.cpp) on its own thread.
public class MaterializrActivity extends SDLActivity {

    // Go edge-to-edge / immersive so the system nav + status bars don't overlay
    // (and clip) the viewport and status bar. Sticky immersive re-hides them
    // after the user swipes them in.
    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);
        if (hasFocus) {
            getWindow().getDecorView().setSystemUiVisibility(
                View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_FULLSCREEN
                | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY);
        }
    }

    @Override
    protected String[] getLibraries() {
        // libmain.so links the OCCT toolkits via DT_NEEDED, so the dynamic
        // loader pulls them (and libc++_shared) from the APK automatically.
        return new String[] {
            "SDL2",
            "main"
        };
    }
}
