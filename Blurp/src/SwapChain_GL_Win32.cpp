#include "opengl/SwapChain_GL_Win32.h"
#include <GL/glew.h>
#include <BlurpEngine.h>
#include <iostream>


#include "opengl/RenderTarget_GL.h"
#include <windows.h>
#include "Window.h"
#include "Window_Win32.h"

namespace blurp
{
    void SwapChain_GL_Win32::Resize(const glm::vec2& a_Dimensions, bool a_FullScreen)
    {
        std::cout << "SwapChain Resized" << std::endl;
        //OpenGL does this for me so I can be lazy.
    }

    std::uint16_t SwapChain_GL_Win32::GetNumBuffers()
    {
        //OpenGL only really does double buffering so yeah.
        return 2;
    }

    std::shared_ptr<RenderTarget> SwapChain_GL_Win32::GetRenderTarget()
    {
        return m_RenderTarget;
    }

    void SwapChain_GL_Win32::Present()
    {
        glBindBuffer(GL_FRAMEBUFFER, reinterpret_cast<RenderTarget_GL*>(m_RenderTarget.get())->GetFrameBufferID());

        static int framecount = 0;
        ++framecount;
        static float red = 0.5f;
        static bool reverse = false;
        red = red + (reverse ?  0.001f : -0.001f);
        if(red > 1.0)
        {
            reverse = !reverse;
            red = 1.0;
        }
        if(red < 0.0)
        {
            reverse = !reverse;
            red = 0.0;
        }

        glClearColor(red, 0.7f, 0.9f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glFlush();
        wglSwapLayerBuffers(m_Hdc, WGL_SWAP_MAIN_PLANE);

        if(framecount % 100 == 0)
        {
            std::cout << "Frames: " << framecount << std::endl;
        }
    }

    bool SwapChain_GL_Win32::OnLoad(BlurpEngine& a_BlurpEngine)
    {
        //Retrieve a pointer to the window.
        const auto window = a_BlurpEngine.GetWindow().get();

        if(window->GetWindowType() != WindowType::WINDOW_WIN32)
        {
            throw std::exception("SwapChain_GL_Win32 can only be constructed for a Win32 window with OpenGL!");
            return false;
        }

        auto win32Window = reinterpret_cast<Window_Win32*>(window);
        const auto hwnd = win32Window->GetHwnd();

        //TODO currently bitdepth for color and depthstencil are always static. Make this dynamic.
        PIXELFORMATDESCRIPTOR pfd =
        {
            sizeof(PIXELFORMATDESCRIPTOR),
            1,
            PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    //Flags
            PFD_TYPE_RGBA,        // The kind of framebuffer. RGBA or palette.
            32,                   // Colordepth of the framebuffer.
            0, 0, 0, 0, 0, 0,
            0,
            0,
            0,
            0, 0, 0, 0,
            24,                   // Number of bits for the depthbuffer
            8,                    // Number of bits for the stencilbuffer
            0,                    // Number of Aux buffers in the framebuffer.
            PFD_MAIN_PLANE,
            0,
            0, 0, 0
        };

        m_Hdc = GetDC(hwnd);

        int  chosenFormat;
        chosenFormat = ChoosePixelFormat(m_Hdc, &pfd);
        SetPixelFormat(m_Hdc, chosenFormat, &pfd);

        m_GlContext = wglCreateContext(m_Hdc);

        wglMakeCurrent(m_Hdc, m_GlContext);

        std::cout << "OpenGL version: " << (char*)glGetString(GL_VERSION) << std::endl;

        //Create the default OpenGL rendertarget which is really just a dummy.
        //This doesn't need loading because it is a dummy.
        m_RenderTarget = std::make_shared<RenderTarget_GL>(m_Settings.renderTargetSettings, true);

        //Disable vsync if specified
        {
            typedef BOOL(APIENTRY* PFNWGLSWAPINTERVALPROC)(int);
            PFNWGLSWAPINTERVALPROC wglSwapIntervalEXT = 0;

            const char* extensions = (char*)glGetString(GL_EXTENSIONS);

            wglSwapIntervalEXT = (PFNWGLSWAPINTERVALPROC)wglGetProcAddress("wglSwapIntervalEXT");

            if (wglSwapIntervalEXT)
            {
                int vSync = m_Settings.vsync ? 1 : 0;
                wglSwapIntervalEXT(m_Settings.vsync);
            }
        }
    
        return true;
    }

    bool SwapChain_GL_Win32::OnDestroy(BlurpEngine& a_BlurpEngine)
    {
        wglDeleteContext(m_GlContext);
        return true;
    }
}
