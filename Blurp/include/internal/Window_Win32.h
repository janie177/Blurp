#pragma once
#include "Window.h"
#include <windows.h>

namespace blurp
{
    /*
     * Windows 32 window encapsulating a hwnd.
     */
    class Window_Win32 : public Window
    {
    public:
        Window_Win32(const WindowSettings& a_Settings);

        HWND GetHwnd();

    protected:
        void Load() override;

        void Destroy() override;

        static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    public:
        void Resize(glm::vec2 a_Dimensions) override;
        void SetFullScreen(bool a_FullScreen) override;
        bool IsFullScreen() override;
        void SetNativeMousePosition(const glm::vec2& a_MousePosition) override;
        glm::vec2 GetNativeMousePosition() const override;
        InputData PollInput() override;
        bool IsClosed() const override;
        void Close() override;

    private:
        RECT GetClientCoordinates() const;
        RECT GetWindowCoordinates() const;
        void CenterCursor(RECT clientRect);
        void OnMoveResize();
        void ObtainFocus();

    private:
        //Event queue
        InputQueue m_InputQueue;

        //Screen dimensions
        glm::vec2 m_Dimensions;

        //Old window rect for switching back from fullscreen.
        RECT m_WindowedRect;

        //Mouse position
        glm::vec2 m_MousePos;

        //True when full screen is enabled.
        bool m_FullScreen;

        //True if the window has closed
        bool m_Closed;

        //Window handle
        HWND m_Hwnd;

        //Flag settings
        bool m_ShowCursor;
        bool m_CaptureCursor;
        bool m_HasFocus;

        //Cursor restore options.
        HCURSOR m_OldCursor;
        bool m_CursorHidden;

        static LONG m_WindowStyle;
    };
}
