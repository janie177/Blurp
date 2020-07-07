#include "BlurpEngine.h"

#include <Windows.h>
#include "GL/glew.h"
#include <cstdio>
#include <iostream>

blurp::BlurpEngine::BlurpEngine()
{

}

void blurp::BlurpEngine::Run()
{
	//GLuint handle;

 //   const wchar_t CLASS_NAME[] = L"Sample Window Class";

 //   WNDCLASS wc = { };

 //   wc.lpfnWndProc = myWndProc;
 //   wc.hInstance = GetModuleHandle(NULL);
 //   wc.lpszClassName = CLASS_NAME;

 //   HWND hwnd = CreateWindowEx(
 //       0,                              // Optional window styles.
 //       CLASS_NAME,                     // Window class
 //       L"Learn to Program Windows",    // Window text
 //       WS_OVERLAPPEDWINDOW,            // Window style

 //       // Size and position
 //       CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

 //       NULL,       // Parent window    
 //       NULL,       // Menu
 //       GetModuleHandle(NULL),  // Instance handle
 //       NULL        // Additional application data
 //   );

 //   HDC ourWindowHandleToDeviceContext = GetDC(hwnd);

 //   PIXELFORMATDESCRIPTOR pfd =
 //   {
 //       sizeof(PIXELFORMATDESCRIPTOR),
 //       1,
 //       PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    //Flags
 //       PFD_TYPE_RGBA,        // The kind of framebuffer. RGBA or palette.
 //       32,                   // Colordepth of the framebuffer.
 //       0, 0, 0, 0, 0, 0,
 //       0,
 //       0,
 //       0,
 //       0, 0, 0, 0,
 //       24,                   // Number of bits for the depthbuffer
 //       8,                    // Number of bits for the stencilbuffer
 //       0,                    // Number of Aux buffers in the framebuffer.
 //       PFD_MAIN_PLANE,
 //       0,
 //       0, 0, 0
 //   };

 //   int  letWindowsChooseThisPixelFormat;
 //   letWindowsChooseThisPixelFormat = ChoosePixelFormat(ourWindowHandleToDeviceContext, &pfd);
 //   SetPixelFormat(ourWindowHandleToDeviceContext, letWindowsChooseThisPixelFormat, &pfd);

 //   HGLRC ourOpenGLRenderingContext = wglCreateContext(ourWindowHandleToDeviceContext);
 //   wglMakeCurrent(ourWindowHandleToDeviceContext, ourOpenGLRenderingContext);

 //   glewExperimental = GL_TRUE;
 //   if (glewInit() != GLEW_OK)
 //   {
 //       std::cout << "Failure!" << std::endl;
 //       getchar();
 //       return;
 //   }

	//glCreateBuffers(1, &handle);



	getchar();
}

LRESULT myWndProc(HWND, UINT, WPARAM, LPARAM)
{
    return 0;
}
