#if !defined (RENDER_H_E20F17B9_0045_4F33_81F7_6BF65092DE9A_)
#define RENDER_H_E20F17B9_0045_4F33_81F7_6BF65092DE9A_

#pragma once  // in case the compiler supports it

// delegate function to be called when a frame needs to be rendered
// it will be called in the context of the RC that requires it
typedef void (*RenderDelegate) (const double dElapsed, const unsigned int nWidth, const unsigned int nHeight);

// needed to pass multiple arguments when creating a worker thread
typedef struct
{
    tribool bVSync;                 // flag to indicate if we enable or disable or leave alone vsync
    HWND    hWnd;                   // handle to the calling window
    HDC     hDC;                    // handle to the device context of the client area of the calling window
    BYTE    nBPP;                   // bits per pixel the application is trying to use (ignored if windowed)
    BYTE    nRefresh;               // vertical refresh rate of the display in hertz (ignored if windowed)
    bool    bFullscreen;            // flag to indicate to the thread if we are in fullscreen mode
    bool    bZoomed;                // flag to indicate to the thread if we are to maximize the main window

}  RENDERARGS, *PRENDERARGS;

// function prototypes
unsigned int __stdcall RenderMain (const PRENDERARGS pArgList);

// user defined window messages the render thread uses to communicate
#define UWM_PAUSE   (WM_APP + 1)
#define UWM_RESIZE  (WM_APP + 2)
#define UWM_SHOW    (WM_APP + 3)
#define UWM_STOP    (WM_APP + 4)

#endif  // RENDER_H