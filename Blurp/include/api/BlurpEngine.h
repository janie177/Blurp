#pragma once
#include <windows.h>

namespace blurp
{
    class BlurpEngine
    {
    public:
        BlurpEngine();

        void Run();
    };
}

LRESULT CALLBACK myWndProc(HWND, UINT, WPARAM, LPARAM);