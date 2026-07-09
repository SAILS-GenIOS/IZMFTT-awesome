#include "ui_layout_bridge.h"

namespace materializr {

namespace {
std::function<int()>     g_get;
std::function<void(int)> g_set;
std::function<float()>      g_getScale;
std::function<void(float)>  g_setScale;
} // namespace

int currentUiLayoutIndex() { return g_get ? g_get() : 0; }

void requestUiLayout(int index) {
    if (g_set && index >= 0 && index <= 2) g_set(index);
}

void bindUiLayoutBridge(std::function<int()> get, std::function<void(int)> set) {
    g_get = std::move(get);
    g_set = std::move(set);
}

float currentUiScalePref() { return g_getScale ? g_getScale() : 1.0f; }

void requestUiScalePref(float scale) {
    if (g_setScale && scale > 0.0f) g_setScale(scale);
}

void bindUiScaleBridge(std::function<float()> get, std::function<void(float)> set) {
    g_getScale = std::move(get);
    g_setScale = std::move(set);
}

} // namespace materializr
