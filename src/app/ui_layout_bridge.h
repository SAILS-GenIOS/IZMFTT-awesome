#pragma once
#include <functional>

// Tiny global bridge (same spirit as touch_mode.h) so PLUGINS can read and
// switch the interface layout without a PluginContext round-trip. Application
// binds the two callbacks at startup; until then the getter reports Classic
// and the setter is a no-op. Index values mirror UiLayout in io/Settings.h:
// 0 = Classic, 1 = Modern, 2 = im-touch.
//
// First consumer: the Getting Started tour's layout picker, which switches
// the layout LIVE behind its modal as the user taps each option (the whole
// app re-renders in the candidate layout — a real preview, not a screenshot).
namespace materializr {

int  currentUiLayoutIndex();
void requestUiLayout(int index);   // switches immediately and persists

void bindUiLayoutBridge(std::function<int()> get,
                        std::function<void(int)> set);

// Desktop UI scale preference (Linux HiDPI): 1.0 = Normal, 2.0 = High DPI.
// Exposed so the first-run picker can set it too (in case Settings is unreadably
// tiny). Persists; takes effect on the next launch (fonts bake at startup).
float currentUiScalePref();
void  requestUiScalePref(float scale);

void  bindUiScaleBridge(std::function<float()> get,
                        std::function<void(float)> set);

} // namespace materializr
