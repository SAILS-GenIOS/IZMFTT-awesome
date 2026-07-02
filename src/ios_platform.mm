// iOS runtime setup — the counterpart of android_platform.cpp, but much
// smaller: the .app bundle is a real directory, so nothing needs extracting.
// We chdir into the bundle (cwd-relative "assets/fonts/<name>" resolves as on
// Android after its extraction step) and point the OCCT CSF_* env vars straight
// at the bundled resource tree.
#include "ios_platform.h"

#if defined(MZ_IOS)

#import <Foundation/Foundation.h>

#include <SDL.h>

#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <string>
#include <system_error>
#include <unistd.h>

namespace materializr {
namespace {

std::atomic<bool> g_inBackground{false};

// SDL delivers SDL_APP_WILLENTERBACKGROUND / DIDENTERFOREGROUND through event
// watches *during* the UIKit callback — they cannot be polled later, because
// the process may be suspended before the queue is drained. The watch just
// flips the flag Application::run() gates rendering on.
int lifecycleWatch(void*, SDL_Event* e) {
    if (e->type == SDL_APP_WILLENTERBACKGROUND) {
        g_inBackground = true;
    } else if (e->type == SDL_APP_DIDENTERFOREGROUND) {
        g_inBackground = false;
    }
    return 1;
}

} // namespace

bool iosInBackground() { return g_inBackground.load(std::memory_order_relaxed); }

void iosInitRuntime() {
    @autoreleasepool {
        // (1) cwd -> bundle root, so resolveBundledFont()'s cwd-relative
        //     "assets/fonts/<name>" candidate resolves (fonts are staged into
        //     the bundle at assets/fonts/ by ios/CMakeLists.txt).
        NSString* res = [NSBundle mainBundle].resourcePath;
        if (!res || chdir(res.UTF8String) != 0)
            std::fprintf(stderr, "ios: chdir to bundle resourcePath failed\n");

        // (2) Settings: SettingsIO::defaultPath() uses $HOME/.config/materializr.
        //     $HOME already points at the sandbox container on iOS; just make
        //     sure the directory exists (container root is writable, but be
        //     loud if that ever changes — settings silently not persisting is
        //     a miserable bug to chase).
        const char* home = std::getenv("HOME");
        if (home) {
            std::error_code ec;
            std::filesystem::create_directories(
                std::filesystem::path(home) / ".config" / "materializr", ec);
            if (ec)
                std::fprintf(stderr, "ios: could not create ~/.config/materializr: %s\n",
                             ec.message().c_str());
        }

        // (3) OpenCASCADE resources: bundled as a plain directory tree —
        //     point every CSF_* var straight at it, no extraction step.
        const std::string resRoot = std::string(res ? res.UTF8String : ".") + "/occt-resources";
        auto setres = [&](const char* var, const std::string& sub) {
            setenv(var, (resRoot + "/" + sub).c_str(), 1);
        };
        setres("CSF_StandardDefaults",     "StdResource");
        setres("CSF_StandardLiteDefaults", "StdResource");
        setres("CSF_XCAFDefaults",         "StdResource");
        setres("CSF_PluginDefaults",       "StdResource");
        setres("CSF_TObjMessage",          "TObj");
        setres("CSF_XmlOcafResource",      "XmlOcafResource");
        setres("CSF_XSMessage",            "XSMessage");
        setres("CSF_SHMessage",            "SHMessage");
        setres("CSF_XSTEPDefaults",        "XSTEPResource");
        setres("CSF_STEPDefaults",         "XSTEPResource");
        setres("CSF_IGESDefaults",         "XSTEPResource");
        setres("CSF_MIGRATION_TYPES",      "StdResource/MigrationSheet.txt");

        // (4) Background-lifecycle watch (see header). The events subsystem
        //     must exist before a watch can be added; Window's full SDL_Init
        //     comes later and is a no-op for already-initialized subsystems.
        SDL_InitSubSystem(SDL_INIT_EVENTS);
        SDL_AddEventWatch(lifecycleWatch, nullptr);

        // Log what actually resolved — the first thing to check if fonts or
        // STEP import misbehave (see ios/README.md "First-build checklist").
        std::error_code ec;
        std::fprintf(stderr,
                     "Materializr iOS runtime initialized (bundle=%s, "
                     "assets/fonts %s, occt-resources %s)\n",
                     res ? res.UTF8String : "?",
                     std::filesystem::exists("assets/fonts", ec) ? "found" : "MISSING",
                     std::filesystem::exists(resRoot, ec) ? "found" : "MISSING");
    }
}

} // namespace materializr

#endif // MZ_IOS
