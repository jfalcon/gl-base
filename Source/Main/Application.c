#include "Main\Application.h"    // standard application include
#include "Main\Render.h"         // main rendering routines
#include "Utility\General.h"     // general utility routines
#include "Primitives\Triforce.h" // zelda triforce primitive
#include <VersionHelpers.h>      // used to determine OS version

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////// MAIN APPLICATION ROUTINES //////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*/
/ / --------------------------------------------------------------------------------------------------------------------------------------------------
/ /
/ /  Author:        Jeremy Lee Falcon
/ /  Copyright:     ©2002-2014 Jeremy Falcon. All Rights Reserved.
/ /  Platforms:     Any Windows version supported by the VS2013 C runtime.
/ /  Program:       GLBASE.EXE (OpenGL Base Program)
/ /  Purpose:       A skeleton project and starting point for OpenGL applications.
/ /
/ /  Legal License Crap:
/ /       [Common Declarations]
/ /            This software is not subject to any export provision of the United States Department of Commerce,
/ /            and may be exported to any country or planet.
/ /
/ /       [License]
/ /            Permission is granted to anyone to use this software for any purpose on any computer system, and
/ /            to alter and redistribute it freely, subject to the following restrictions:
/ /
/ /            1. The author is not responsible for any consequences of resulting from the use of this software,
/ /               no matter how awful, even if they arise from flaws within it.
/ /
/ /            2. The origin of this software must not be misrepresented, either by explicit claim or by omission.
/ /               Since few users ever read sources, a simple thanks in the credits or documentation would be nice.
/ /
/ /            3. Altered versions must be plainly marked as such, and must not be misrepresented as being the
/ /               original software.
/ /
/ /            4. This notice may not be removed or altered.
/ /`
/ /  Legal Disclaimer Crap:
/ /     THIS CODE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT
/ /     NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
/ /
/ /  Acknowledgements:
/ /     The main application icon included with this distribution was graciously donated by iconshock.com,
/ /     for non-commercial use only, as part of their Sigma collection.
/ /
/ / --------------------------------------------------------------------------------------------------------------------------------------------------
/*/

// local variables
static bool _bGoFullscreen = false;
static unsigned int _nRenderThreadID = 0;
static HANDLE _hRenderThread = NULL;

// local prototypes
static LRESULT CALLBACK __wndProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static bool __goFullscreen        (HWND hWnd, unsigned int nWidth, unsigned int nHeight, BYTE nBits, BYTE nRefresh);
static void __goWindowed          (void);
static bool __procStartOptions    (PRENDERARGS pArgs, PRECT pWndRect, HANDLE *pMutex);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*/
/ / PARAMETERS:
/ /     hInstance = handle to the current instance of the application
/ /     hPrevInstance = handle to the previous instance of the application (always NULL)
/ /     lpCmdLine = command line string the application was launched with, excluding the program name
/ /     nCmdShow = flag to specify how the main window should be shown (we ignore this)
/ /
/ / PURPOSE:
/ /     This function initializes the application and spawns the main window process.
/*/

int WINAPI
WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    HWND        hWnd = NULL;        // handle to the main window
    HDC         hDC = NULL;         // handle to the window's device context
    RECT        rcWndPos = {0};     // position to use when creating the main app window
    HANDLE      hMutex = NULL;      // handle to the single instance mutex
    MSG         msg = {0};          // message structure for the queue
    RENDERARGS  args = {0};         // arguments to be passed to the render thread

    // process independent startup options, if returns false then exit the app
    if(__procStartOptions(&args, &rcWndPos, &hMutex))
    {
        WNDCLASS wc = {0};          // window's class structure
        DWORD dwWindowStyle = 0;    // style bits to use when creating the main app window
        HBRUSH hBrush = NULL;       // will contain the background color of the main window

        // set the background color to black (this may be changed to whatever is needed)
        hBrush = CreateSolidBrush(CONFIG_DEF_BACKGROUND);

        // we must specify the attributes for the window's class
        wc.style            = CS_HREDRAW|CS_VREDRAW|CS_OWNDC;   // style bits (CS_OWNDC very important for OGL)
        wc.lpfnWndProc      = (WNDPROC)__wndProc;               // window procedure to use
        wc.cbClsExtra       = 0;                                // no extra data
        wc.cbWndExtra       = 0;                                // no extra data
        wc.hInstance        = hInstance;                        // associated instance
        wc.hIcon            = NULL;                             // use default icon temporarily
        wc.hCursor          = LoadCursor(NULL, IDC_ARROW);      // use default cursor (Windows owns it, so don't unload)
        wc.hbrBackground    = hBrush;                           // background brush (let windows destroy it)
        wc.lpszClassName    = CLASS_NAME;                       // class name

        // this will create or not create a menu for the main window (depends on app settings)
        wc.lpszMenuName = (CONFIG_ALLOW_MENU) ? MAKEINTRESOURCE(IDR_MAINFRAME) : NULL;

        // now, we can register this class
        RegisterClass(&wc);

        // here we determine the correct style bits to use for the man window (depends on settings)
        if(_bGoFullscreen)
            dwWindowStyle = WS_CLIPSIBLINGS|WS_CLIPCHILDREN|WS_POPUP|WS_CLIPCHILDREN;
        else
        {
            dwWindowStyle = (CONFIG_ALLOW_RESIZE) ? WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN :
                WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX|WS_CLIPCHILDREN;
        }

        // use a custom window class (as in type) to create a main window with
        hWnd = CreateWindow(CLASS_NAME,                         // class to use
                            APP_NAME,                           // title for window
                            dwWindowStyle,                      // style bits
                            rcWndPos.left, rcWndPos.top,        // position (x, y)
                            rcWndPos.right, rcWndPos.bottom,    // width/height
                            NULL,                               // no parent
                            NULL,                               // no menu
                            hInstance,                          // associated instance
                            NULL);                              // no extra data

        if(hWnd == NULL)
        {
            // the window wasn't created, let the user know and leave
            ResourceMessage(NULL, IDS_ERR_CREATEWIN_MAIN, 0, MB_OK|MB_ICONERROR);
        }
        else
        {
            PIXELFORMATDESCRIPTOR pfd = {0};    // pixel descriptor structure
            int nFormat = 0;                    // index of the closest matching pixel format provided by the system
            HICON hIcon = NULL;                 // handle to the main window's large icon
            HICON hIconSmall = NULL;            // handle to the main window's small icon

            // if we are going fullscreen, then do so here
            if(_bGoFullscreen)
            {
                // attempt to enter fullscreen mode
                if(!__goFullscreen(hWnd, rcWndPos.right, rcWndPos.bottom, args.nBPP, args.nRefresh))
                {
                    ResourceMessage(NULL, IDS_ERR_DISPLAYMODE, 0, MB_OK|MB_ICONERROR);
                    return false;
                }
            }
            else
            {
                // center the window on the screen if windowed and no registry data
                // exists for the old position from the application being ran before
                if(!args.bZoomed && ((rcWndPos.left == 0) && (rcWndPos.top == 0)))
                    AlignWindow(hWnd, ALIGN_CENTER|ALIGN_MIDDLE, NULL);
            }

            // load the icons we wish to use for the main window
            hIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDR_MAINFRAME),
                IMAGE_ICON, GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);

            hIconSmall = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDR_MAINFRAME),
                IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);

            // use our custom icons for the window instead of the Windows'
            // default icons on the title bar and when using Alt+Tab key
            SendMessage(hWnd, WM_SETICON, (WPARAM)ICON_BIG, (LPARAM)hIcon);
            SendMessage(hWnd, WM_SETICON, (WPARAM)ICON_SMALL, (LPARAM)hIconSmall);

            /*/
            / / Now we must set-up the display settings and the render context
            / / to use with the GL. We then enter the traditional message loop
            / / for the window. Afterwards, we need to perform some clean-up.
            /*/

            // get the window's device context
            hDC = GetDC(hWnd);

            // set the pixel format for the DC, this tells Windows things like
            // the color depth and so on that we wish to use in the context
            pfd.nSize = sizeof(pfd);
            pfd.nVersion = 1;
            pfd.dwFlags = PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER;
            pfd.iPixelType = PFD_TYPE_RGBA;
            pfd.cColorBits = args.nBPP;         // BPP is ignored for windowed mode
            pfd.cDepthBits = 16;                // 16-bit z-buffer
            pfd.iLayerType = PFD_MAIN_PLANE;

            // if Windows can't handle what we asked for it will approximate the closest thing
            nFormat = ChoosePixelFormat(hDC, &pfd);

            // if we can't find a format to use, let the user know and split
            if(nFormat == 0)
            {
                ResourceMessage(hWnd, IDS_ERR_PIXFORMAT, 0, MB_OK|MB_ICONERROR);
            }
            else
            {
                // set the DC to the format we want
                SetPixelFormat(hDC, nFormat, &pfd);

                // set additional parameters to send the worker thread
                args.hWnd = hWnd;
                args.hDC = hDC;

                ///// THIS IS WHERE THE MAIN RENDER ROUTINE IS SET //////

                // set the main render delegate to be the triforce
                args.pRenderFrame = TriforcePrimitive;

                // initialize the rendering context in a separate thread (do not use CreateThread()
                // to avoid leaks caused by the CRT when trying to use standard CRT libs)
                _hRenderThread = (HANDLE)_beginthreadex(NULL, 0, RenderMain, &args, 0, &_nRenderThreadID);

                /*/
                / / WARNING: If you do not use a separate thread for rendering, it is imperative that
                / / PeekMessage() is used rather than GetMessage(). However, it is just the opposite if
                / / you do use a render thread as it will save a bit of processing on the main thread.
                /*/

                // start the endless main application loop we use to process Windows messages with
                while(GetMessage(&msg, NULL, 0, 0) != 0)
                {
                    // send the message off to the appropriate window procedure, as a security
                    // precaution only do so if it belongs to the the main window in question
                    if(msg.hwnd == hWnd)
                    {
                        TranslateMessage(&msg);
                        DispatchMessage(&msg);
                    }
                }

                // we're done, destroy the render thread
                if(_hRenderThread != NULL) CloseHandle(_hRenderThread);
            }

            // clean-up (windows specific items)
            ReleaseDC(hWnd, hDC);

            if(_bGoFullscreen) __goWindowed();
            if(hMutex != NULL) CloseHandle(hMutex);
            if(hIcon != NULL) DestroyIcon(hIcon);
            if(hIconSmall != NULL) DestroyIcon(hIconSmall);
        }

        // clean-up (windows specific items)
        if(hBrush != NULL) DeleteObject(hBrush);
    }

    // let's play nice and return any message sent by windows
    return (int)msg.wParam;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*/
/ / PARAMETERS:
/ /     hWnd = handle to the window the message belongs to
/ /     uMsg = message to process
/ /     wParam = word sized parameter who's value depends on the message
/ /     lParam = long sized parameter who's value depends on the message
/ /
/ / PURPOSE:
/ /     Window procedure to handle messages for the main window.
/*/

static LRESULT CALLBACK
__wndProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lReturn = false;

    switch(uMsg)
    {
        case UWM_SHOW:

            // the render thread will send this message when its inits are done
            // wParam will contain true if we are to maximize the window
            if((bool)wParam == true)
            {
                // if the user left the window maxed on close, then restore the state
                // note: must be done this way b/c when the window style is set to
                // overlapped window, Windows ignores the maxed style bit
                ShowWindow(hWnd, SW_MAXIMIZE);
            }
            else
                ShowWindow(hWnd, SW_SHOW);

            // give a slightly higher priority and set keyboard focus
            SetForegroundWindow(hWnd);
            SetFocus(hWnd);

            // let the caller know this message was processed
            lReturn = true;
            break;

        case WM_CLOSE:

            // before the main window is destroyed, save it's position (and width/height) if in windowed mode
            // WARNING: do not perform this operation if the main window is closed in a maxed or mined state
            if(!_bGoFullscreen)
            {
                if(!IsIconic(hWnd))
                {
                    RECT rcWndPos = {0}; // contains the position and size of the window

                    GetWindowRect(hWnd, &rcWndPos);

                    // save width/height data if allowed to resize
                    #if CONFIG_ALLOW_RESIZE
                    {
                        bool bZoomed = IsZoomed(hWnd);

                        // save position data (even if maximized as it will tell us which monitor to maximize on)
                        SetUserValue(NULL, _T("Left"), REG_DWORD, &rcWndPos.left, sizeof(LONG));
                        SetUserValue(NULL, _T("Top"), REG_DWORD, &rcWndPos.top, sizeof(LONG));

                        // only save size data if the app is not maximized
                        if(!bZoomed)
                        {
                            long lWidth = 0, lHeight = 0;

                            lWidth = rcWndPos.right - rcWndPos.left;
                            lHeight = rcWndPos.bottom - rcWndPos.top;

                            SetUserValue(NULL, _T("Width"), REG_DWORD, &lWidth, sizeof(lWidth));
                            SetUserValue(NULL, _T("Height"), REG_DWORD, &lHeight, sizeof(lHeight));
                        }

                        // if we are in windowed mode, save the maxed state to restore later
                        SetUserValue(NULL, _T("Zoom"), REG_DWORD, &bZoomed, sizeof(bZoomed));
                    }
                    #else
                    {
                        // save position data
                        SetUserValue(NULL, _T("Left"), REG_DWORD, &rcWndPos.left, sizeof(LONG));
                        SetUserValue(NULL, _T("Top"), REG_DWORD, &rcWndPos.top, sizeof(LONG));
                    }
                    #endif
                }
            }

            // we need to shutdown the render thread, so we signal it to stop
            // to play nice, let it finish before proceeding (up to 5 seconds)
            if((_hRenderThread) != NULL && (_nRenderThreadID > 0))
            {
                PostThreadMessage(_nRenderThreadID, UWM_STOP, 0, 0);
                WaitForSingleObject(_hRenderThread, 5000);
            }

            // destroy this window and thus the app as requested
            DestroyWindow(hWnd);
            break;

        case WM_DESTROY:

            // send WM_QUIT and close main running thread
            PostQuitMessage(true);
            break;

        case WM_ENTERMENULOOP:

            // NOTE: always leave this whether or not the app has a menu! (remember the system menu)
            // to suspend the render thread if the user is navigating menus (if not minimized)
            if(!IsIconic(hWnd)) PostThreadMessage(_nRenderThreadID, UWM_PAUSE, true, 0);
            break;

        case WM_EXITMENULOOP:

            // NOTE: always leave this whether or not the app has a menu! (remember the system menu)
            // to start the render thread back up if the user leaves the menu (if not minimized)
            if(!IsIconic(hWnd)) PostThreadMessage(_nRenderThreadID, UWM_PAUSE, false, 0);
            break;

        case WM_GETMINMAXINFO:

            // must specify a minimum for both aspects before we care about this message
            #if (CONFIG_ALLOW_RESIZE && (CONFIG_MIN_HEIGHT > 0) && (CONFIG_MIN_WIDTH > 0))
            {
                MINMAXINFO *pInfo = (MINMAXINFO *)lParam;

                // we use the message to enforce a minimum width and height of the window (if desired)
                pInfo->ptMinTrackSize.x = CONFIG_MIN_HEIGHT;
                pInfo->ptMinTrackSize.y = CONFIG_MIN_WIDTH;
            }
            #endif
            break;

        case WM_KEYDOWN:

            switch(wParam)
            {
                case VK_ESCAPE:

                    // if we are in fullscreen mode only
                    if(_bGoFullscreen)
                    {
                        HMENU hMenu = NULL;
                        bool bHidden = false;

                        // get the handle of the menu bar (if present)
                        hMenu = GetMenu(hWnd);

                        // to determine what gets done, we use a boolean
                        // stored in the window's user data buffer
                        bHidden = (bool)GetWindowLongPtr(hWnd, GWLP_USERDATA);

                        // the ESC key can do one of two things in fullscreen mode, if there
                        // is a menu, it will show and hide it otherwise it will exit the app
                        // if the menu is hidden then we do not process this (we toggle it)
                        if((hMenu == NULL) && !bHidden)
                        {
                            // no menu so close the application down
                            SendMessage(hWnd, WM_CLOSE, 0, 0);
                        }
                        else
                        {
                            if(bHidden)
                            {
                                // recreate the menu bar to toggle it on and show cursor
                                SetMenu(hWnd, LoadMenu(GetWindowInstance(hWnd), MAKEINTRESOURCE(IDR_MAINFRAME)));
                                ShowCursor(true);

                                // set the user data to false to indicate we toggled the menu
                                SetWindowLongPtr(hWnd, GWLP_USERDATA, false);
                            }
                            else
                            {
                                // we have a menu bar that's shown, so toggle it off and hide cursor
                                SetMenu(hWnd, NULL);
                                ShowCursor(false);

                                // set the user data to true to indicate we toggled the menu
                                SetWindowLongPtr(hWnd, GWLP_USERDATA, true);
                            }
                        }
                    }
                    break;
            }
            break;

        case WM_SIZE:

            switch(wParam)
            {
                case SIZE_MINIMIZED:

                    // play nice and don't hog the computer if the main window is minimized
                    #if CONFIG_PAUSE_MINIMIZED
                        PostThreadMessage(_nRenderThreadID, UWM_PAUSE, true, 0);
                    #endif
                    break;

                case SIZE_RESTORED:

                    // we need to resize the view port for OGL, but it must be done in the context of
                    // the render thread, so set the signal to be picked up by the render thread
                    PostThreadMessage(_nRenderThreadID, UWM_RESIZE, 0, 0);

                    // we're back in action, so let the threads continue
                    #if (CONFIG_PAUSE_MINIMIZED == TRUE)
                        PostThreadMessage(_nRenderThreadID, UWM_PAUSE, false, 0);
                    #endif
                    break;
            }
            break;

        default:
            lReturn = DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

    return lReturn;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*/
/ / PARAMETERS:
/ /     hWnd = handle to the associated window to work with
/ /     nWidth = width to set the display's resolution to
/ /     nHeight = height to set the display's resolution to
/ /     nBits = bits per pixel to use
/ /     nRefresh = vertical refresh rate to use
/ /
/ / PURPOSE:
/ /     Sets the display for the window to fullscreen mode.
/*/

static bool
__goFullscreen (HWND hWnd, unsigned int nWidth, unsigned int nHeight, BYTE nBits, BYTE nRefresh)
{
    DEVMODE dm = {0}; // device mode structure

    dm.dmSize       = sizeof(DEVMODE);
    dm.dmPelsWidth  = nWidth;                                   // screen width
    dm.dmPelsHeight = nHeight;                                  // screen height
    dm.dmBitsPerPel = nBits;                                    // display bits per pixel (BPP)
    dm.dmFields     = DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT; // flags

    // as a safety precaution, double check to make sure the refresh rate is in range
    if((nRefresh >= CONFIG_MIN_REFRESH) && (nRefresh <= CONFIG_MAX_REFRESH)) dm.dmDisplayFrequency = nRefresh;

    // attempt to move to fullscreen
    if(ChangeDisplaySettings(&dm, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
        return false;
    else
    {
        // hide the cursor (if in fullscreen with no menu)
        if(GetMenu(hWnd) == NULL) ShowCursor(false);
        return true;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*/
/ / PARAMETERS:
/ /     none
/ /
/ / PURPOSE:
/ /     Sets the display for the window to windowed mode.
/*/

static void
__goWindowed (void)
{
    ChangeDisplaySettings(NULL, 0); // switch back to the desktop
    ShowCursor(true);               // show the cursor again
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*/
/ / PARAMETERS:
/ /     pArgs = pointer to the argument structure that will eventually get sent to the render thread.
/ /     pWndRect = pointer to a rectangle to receive the coordinates for the main window to create with
/ /     pMutex = pointer to a mutex handle used to determine if this app will be a single instance or not
/ /
/ / PURPOSE:
/ /     Processes the application defined options and command line options. Returns
/ /     false if the app needs to be shutdown or true otherwise.
/*/

static bool
__procStartOptions (PRENDERARGS pArgs, PRECT pWndRect, HANDLE *pMutex)
{
    bool bReturn = true;

    // first and foremost, make sure the host os meets our requirements
    // and, let's hope and pray you don't support anything below XP
    if(!IsWindowsXPOrGreater())
    {
        ResourceMessage(NULL, IDS_ERR_WINVER, 0, MB_OK|MB_ICONERROR);
        bReturn = false;
    }
    else
    {
        if(CONFIG_SINGLE_INSTANCE && (pMutex != NULL))
        {
            *pMutex = CreateMutex(NULL, true, CLASS_NAME);

            // limit this app to a single instance only (make sure class name is unique)
            // NOTE: using a mutex is MUCH safer than using FindWindow() alone, if the
            // executable gets executed back-to-back - as in a script - mutexes still work
            if(GetLastError() == ERROR_ALREADY_EXISTS)
            {
                HWND hPrevious = NULL;

                // set focus to the other main window (if possible, may not be created yet) then exit
                // if the class name is unique we don't worry about searching against the title
                if((hPrevious = FindWindow(CLASS_NAME, NULL)) != NULL)
                    ShowWindow(hPrevious, SW_SHOWNORMAL);

                bReturn = false;
            }
        }

        // don't bother processing the rest if the app is just going to be shutdown
        if(bReturn && (pArgs != NULL) && (pWndRect != NULL))
        {
            DWORD dwTemp = 0; // used to pull DWORD values from the registry

            /*/
            / / Here, we need to determine if this app allows fullscreen mode. If so, do we default to it
            / / or not? If not, then process nothing and stay windowed. If it is allowed, then we need to
            / / specify the fullscreen command argument and set it's value to yes or no (default yes).
            /*/
            #if CONFIG_ALLOW_FULLSCREEN
            {
                TCHAR szBuff[MAX_LOADSTRING] = {0};

                // read the command line arguments to see what we should do about fullscreen
                if(GetCmdLineValue(_T("fullscreen"), szBuff, STRING_SIZE(szBuff)))
                {
                    // any value other than "no" (including no value) is considered a yes
                    if(!STRING_MATCH(szBuff, _T("no"))) _bGoFullscreen = true;
                }
                else
                {
                    // command line argument not found, but we need to check if we default to fullscreen
                    #if CONFIG_DEF_FULLSCREEN
                        _bGoFullscreen = true;
                    #endif
                }

                // set the parameter that will eventually be sent to the render thread
                pArgs->bFullscreen = _bGoFullscreen;
            }
            #endif

            // get the bits per pixel data (if any) from the registry, can only be 8, 16, 24, or 32
            dwTemp = 0;
            if(!GetUserValue(NULL, _T("BPP"), REG_DWORD, &dwTemp, sizeof(dwTemp)))
                pArgs->nBPP = CONFIG_DEF_BPP;
            else
            {
                // to be valid, it must be a multiple of 8 in the range of 8-32, otherwise use the default
                if((dwTemp < 8 || dwTemp > 32) || ((dwTemp % 8) != 0)) dwTemp = CONFIG_DEF_BPP;

                // finally set the BPP value
                pArgs->nBPP = (BYTE)dwTemp;
            }

            // get the vertical refresh rate data (if any) from the registry, must be between min and max
            dwTemp = 0;
            if(!GetUserValue(NULL, _T("Refresh"), REG_DWORD, &dwTemp, sizeof(dwTemp)))
                pArgs->nRefresh = CONFIG_MIN_REFRESH;
            else
            {
                // to be valid, it must be between the min and max, otherwise use the default
                if(dwTemp > CONFIG_MAX_REFRESH)
                    pArgs->nRefresh = CONFIG_MAX_REFRESH;

                else if(dwTemp < CONFIG_MIN_REFRESH)
                    pArgs->nRefresh = CONFIG_MIN_REFRESH;

                else
                    pArgs->nRefresh = (BYTE)dwTemp;
            }

            /*/
            / / Here, we need to determine if we need to set the vertical on or off. It's important to
            / / note, on nVidia cards, this can be overridden so the app is not able to adjust this.
            / / If it is not allowed, the whatever setting/default set by the video card will take effect.
            /*/
            #if CONFIG_ALLOW_VSYNC
            {
                // assume vsync is indeterminate if no setting is found
                dwTemp = 0;
                if(!GetUserValue(NULL, _T("VSync"), REG_DWORD, &dwTemp, sizeof(dwTemp)))

                    // if no option is set, then default to indeterminate
                    pArgs->bVSync = maybe;
                else
                    // ensure we are working with proper boolean data
                    pArgs->bVSync = (dwTemp == 0) ? no : yes;
            }
            #else
                // not allowed, so default to leaving vsync alone
                pArgs->bVSync = maybe;
            #endif

            /*/
            / / At this point we need to see if the registry contains data regarding the main window position.
            / / Of course, we only care about this if not in fullscreen mode, but check now to reduce flicker.
            /*/

            // independently test left/top (if not in registry then we center the window later on)
            // if we are in fullscreen mode, always leave these set to zero
            if(!_bGoFullscreen)
            {
                GetUserValue(NULL, _T("Left"), REG_DWORD, &pWndRect->left, sizeof(LONG));
                GetUserValue(NULL, _T("Top"), REG_DWORD, &pWndRect->top, sizeof(LONG));
            }

            // just in case the data was corrupted, perform a bit of checking (no negative values are allowed)
            pWndRect->left = (unsigned long)pWndRect->left;
            pWndRect->top = (unsigned long)pWndRect->top;

            /*/
            / / WARNING: if we do not allow the user to resize then never use the values from the registry as this
            / / will create an exploit, so in this case we always use the defaults and allow the programmer to adjust
            / / as needed, also if the app will only run in fullscreen mode it should always turn CONFIG_ALLOW_RESIZE off
            /*/
            #if CONFIG_ALLOW_RESIZE
            {
                // if this data doesn't exist or is bogus, we just revert back to the default width/height
                GetUserValue(NULL, _T("Width"), REG_DWORD, &pWndRect->right, sizeof(LONG));
                GetUserValue(NULL, _T("Height"), REG_DWORD, &pWndRect->bottom, sizeof(LONG));

                pWndRect->right = (pWndRect->right <= 0) ? CONFIG_DEF_WIDTH : pWndRect->right;
                pWndRect->bottom = (pWndRect->bottom <= 0) ? CONFIG_DEF_HEIGHT : pWndRect->bottom;

                if(!_bGoFullscreen)
                {
                    // if we are in windowed mode, get the maximized state so we can restore it if needed
                    dwTemp = 0;
                    if(!GetUserValue(NULL, _T("Zoom"), REG_DWORD, &dwTemp, sizeof(dwTemp)))
                        pArgs->bZoomed = false;
                    else
                        // ensure we are working with proper boolean data
                        pArgs->bZoomed = (dwTemp == 0) ? false : true;
                }
            }
            #else
                pWndRect->right = CONFIG_DEF_WIDTH;
                pWndRect->bottom = CONFIG_DEF_HEIGHT;
            #endif
        }
    }

    return bReturn;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////