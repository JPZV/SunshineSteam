#pragma once

#include <functional>

#ifdef _WIN32
using CloseCallback = std::function<void()>;
int StartDummyWindow(CloseCallback _onClose);
bool WindowLoop();
#endif