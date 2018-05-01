#include "Main\Application.h"   // standard application include
#include "Main\Render.h"        // include for this file
#include "Utility\Graphical.h"  // graphical utility routines

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////// MAIN OPENGL RENDERING ROUTINES ///////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// local state variables, used to signal the worker thread (terminate, resize, etc.), if another
// thread wishes to set the states these track then they must do so via the message queue
static bool _bStopRenderThread = false;
static bool _bResizeFrame = true; // must default to true to set initial sizes
static bool _bPaused = false;

// local function prototypes
static void   __initRender    (const PRENDERARGS pArgList);
static void   __onResizeFrame (HWND hWnd, unsigned int nWidth, unsigned int nHeight);
static void   __threadProc    (UINT uMsg, WPARAM wParam, LPARAM lParam);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*/
/ / PARAMETERS:
/ /     pArgList->bVSync;          // flag to indicate if we enable or disable or leave alone vsync
/ /     pArgList->hWnd;            // handle to the calling window
/ /     pArgList->hDC;             // handle to the device context of the client area of the calling window
/ /     pArgList->nBPP;            // bits per pixel the application is trying to use (ignored if windowed)
/ /     pArgList->nRefresh;        // vertical refresh rate of the display in hertz (ignored if windowed)
/ /     pArgList->bFullscreen;     // flag to indicate to the thread if we are in fullscreen mode
/ /     pArgList->bZoomed;         // flag to indicate to the thread if we are to maximize the main window
/ /     pArgList->pRenderFrame;    // delegate function to be called when a frame needs to be renderred
/ /
/ / PURPOSE:
/ /        Wraps the process of calling initialization routines and
/ /        contains the main loop for the rendering functions.
/ /
/ / NOTE:
/ /        The function must be declaed as __stdcall. Also, this function is in a seperate worker thread!
/*/

unsigned int __stdcall
RenderMain (const PRENDERARGS pArgList)
{
    #ifdef _DEBUG
        DWORD dwFPSCurrent = 0, dwFPSLast = 0;  // used to help calc the fps
        unsigned short nFPS = 0;                // current framerate/FPS for the main loop
    #endif

    static double dLastTime = 0, dCurTime = 0;  // used to calc CPU cycles during a render
    static double dElapsed = 0;                 // used to calc CPU cycles during a render

    HGLRC hRC     = NULL;                       // handle to the GLs render context
    RECT rcClient = {0};                        // coordinates of the area safe to draw on
    MSG msg       = {0};                        // message struct for the queue

    // create and activate (in OGL) the render context
    hRC = wglCreateContext(pArgList->hDC);
    if(wglMakeCurrent(pArgList->hDC, hRC))
    {
        #ifdef _DEBUG
            ENTER_GL
        #endif

        // take care of initialization routines specific to OGL
        __initRender(pArgList);

        #ifdef _DEBUG
            // test to see if we got an initialization error, if so then stop the render thread
            _bStopRenderThread = LEAVE_GL(_T("__initRender()"))
        #endif

        // if no previous error exists, let the main thread know it's ok to display the main window
        if(!_bStopRenderThread) _bStopRenderThread = (bool)!SendMessage(pArgList->hWnd, UWM_SHOW, pArgList->bZoomed, 0);
    }

    // note: this is the main render loop used for OpenGL, it's an endless loop
    // with low level access to the video hardware, use this power wisely
    while(!_bStopRenderThread)
    {
        // NOTE: it is imparative that PeekMessage() is used rather than GetMessage()
        // listen to the queue in case this thread receives a message
        if(PeekMessage(&msg, NULL, UWM_PAUSE, UWM_STOP, PM_REMOVE)) __threadProc(msg.message, msg.wParam, msg.lParam);

        // do not waste processing time if the window is minimized
        // note: even in fullscreen a window can end up minimized
        if(!IsIconic(pArgList->hWnd))
        {
            // if we need to resize the window then do so, but only once per request
            // this will be called on startup
            if(_bResizeFrame)
            {
                // set-up the perspective screen to be the size of the client area
                // to avoid clipping, use the client area size, not the window size
                GetClientRect(pArgList->hWnd, &rcClient);

                // call the resize handler and set flag that it's been processed
                __onResizeFrame(pArgList->hWnd, rcClient.right, rcClient.bottom);
                _bResizeFrame = false;
            }

            // only perform this action if the rendering isn't paused
            if(!_bPaused)
            {
                #ifdef _DEBUG
                    // use a low resolution for the FPS count
                    dwFPSCurrent = GetTickCount();

                    ENTER_GL
                #endif

                // call the main drawing delegate, if it's to be seen, this routine must show it
                // we use time-based rendering, so the time argument should be used as a factor
                dCurTime = GetCPUTicks();
                dElapsed = dCurTime - dLastTime;
                pArgList->pRenderFrame(dElapsed, rcClient.right, rcClient.bottom);
                dLastTime = dCurTime;

                #ifdef _DEBUG
                    // if we had OGL errors during the render, let's find out about them
                    _bPaused = LEAVE_GL(_T("RenderDelegate()"))
                #endif

                // swap the buffers (double buffering)
                SwapBuffers(pArgList->hDC);

                #ifdef _DEBUG

                    if(!pArgList->bFullscreen)
                    {
                        // display FPS on the titlebar of the main window if not in fullscreen mode
                        if(nFPS == 0)
                        {
                            dwFPSLast = dwFPSCurrent;
                            nFPS = 1;
                        }
                        else
                        {
                            // check for overflow (as a precaution)
                            if(nFPS < 0xFFFF) nFPS++;

                            // reset the FPS counter if we reach a new second
                            if((dwFPSCurrent - dwFPSLast) >= 1000)
                            {
                                TCHAR szBuff[MAX_LOADSTRING] = {0};
                                TCHAR szVersion[MAX_LOADSTRING] = {0};
                                size_t nConverted = 0;

                                // to save performance, only update the window title once a second
                                // also, place the OGL version information in the titlebar
                                mbstowcs_s(&nConverted, szVersion, MAX_LOADSTRING, (const char *)glGetString(GL_VERSION), MAX_LOADSTRING);
                                wsprintf(szBuff, _T("OpenGL %s - %hu FPS"), szVersion, nFPS);
                                SetWindowText(pArgList->hWnd, szBuff);

                                nFPS = 0;
                            }
                        }
                    }
                #endif
            }
            else
            {
                // in order to keep the timed-based counter current, call this if we're paused
                dLastTime = GetCPUTicks();
            }
        }
    }

    // clean-up (OGL and wiggle specific items)
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hRC);

    // kill this thread and its resources (CRT allocates them)
    _endthreadex(0);
    return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*/
/ / PARAMETERS:
/ /     pArgList = pointer to argument structure that the render thread received
/ /
/ / PURPOSE:
/ /     This routine is meant to setup OGL specifics before the rendering loop starts.
/ /     This should only be called once, prior to the main render loop.
/ /
/ / NOTES:
/ /        This must be called in the context of the render thread.
/*/

static void
__initRender (const PRENDERARGS pArgList)
{
    if(pArgList != NULL)
    {
        GLfloat LightPos[] = {0.0f, 0.0f, 0.0f, 1.0f};

        // do we modify vsync?
        if(pArgList->bVSync == yes)     SetVerticalSync(true);
        else if(pArgList->bVSync == no) SetVerticalSync(false);

        glShadeModel(GL_SMOOTH);                    // enable smooth shading
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);       // black background/clear color

        // set-up the depth buffer
        glClearDepth(1.0f);
        glEnable(GL_DEPTH_TEST);
        glDepthMask(true);
        glDepthFunc(GL_LEQUAL);

        // set up one-byte alignment for pixel storage (saves memory)
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        // use backface culling (this is 2D, so we'll never see the back faces anyway)
        glFrontFace(GL_CW);
        glCullFace(GL_BACK);

        // perspective calculations
        glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

        // use lighting
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
        glLightfv(GL_LIGHT0, GL_POSITION, LightPos);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

/*/
/ / void
/ /        OnResizeFrame (HWND hWnd, unsigned int nWidth, unsigned int nHeight)
/ /
/ /        hWnd = handle to the window who's client area is attach to the rc
/ /        nWidth = width to resize to (passed to save calcs)
/ /        nHeight = height to resize to (passed to save calcs)
/ /
/ / PURPOSE:
/ /        This routine sets up the perspective and viewport used by OGL.
/ /        It should be called as needed when its parent window is sized.
/ /
/ / NOTES:
/ /        This must be called in the context of the render thread.
/*/

static void
__onResizeFrame (HWND hWnd, unsigned int nWidth, unsigned int nHeight)
{
    // prevent division by zero
    if(nHeight <= 0) nHeight = 1;

    // (re)size the viewport to consume the entire client area
    glViewport(0, 0, nWidth, nHeight);

    // reset the projection matrix
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // (re)calculate the aspect ratio of the viewport (0,0 is bottom left)
    //glOrtho(0.0f, nWidth, 0.0f, nHeight, 0.0f, 1.0f);
    gluPerspective(45.0f, (float)nWidth / (float)nHeight, 1.0f, 100.0f);

    // lastly, reset the modelview matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*/
/ / void
/ /        ThreadProc (UINT uMsg, WPARAM wParam, LPARAM lParam)
/ /
/ /        uMsg = message to process
/ /        wParam = word sized param who's value depends on the message
/ /        lParam = long sized param who's value depends on the message
/ /
/ / PURPOSE:
/ /        Procedure to handle messages for this thread.
/*/

static void
__threadProc (UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
        case UWM_PAUSE:

            // notification that we should either pause or resume rendering
            _bPaused = (bool)wParam;
            break;

        case UWM_RESIZE:

            // notification that viewport(s) need(s) to be resized
            _bResizeFrame = true;
            break;

        case UWM_STOP:

            // notification that we should stop the render thread from executing
            _bStopRenderThread = true;
            break;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////