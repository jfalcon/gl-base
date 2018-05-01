#if !defined (APPLICATION_H_CA52C692_38BA_4E31_9A2C_89B6B36FE073_)
#define APPLICATION_H_CA52C692_38BA_4E31_9A2C_89B6B36FE073_

#pragma once                    // in case the compiler supports it
#pragma warning (disable:4100)  // unreferenced formal parameter

// standard definitions
#define STRICT                                                  // enable strict type-checking of Windows handles
#define WIN32_LEAN_AND_MEAN                                     // allow the exclusion of uncommon features
#define WINVER                                          0x0501  // allow the use of Windows XP specific features
#define _WIN32_WINNT                                    0x0501  // allow the use of Windows XP specific features
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES         1       // use the new secure functions in the CRT
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES_COUNT   1       // use the new secure functions in the CRT

// standard includes
#include <windows.h>            // fundamental Windows header file
#include <windowsx.h>           // useful Windows programming extensions
#include <tchar.h>              // generic text character mapping
#include <string.h>             // includes string manipulation routines
#include <stdlib.h>             // includes standard libraries
#include <stdio.h>              // includes standard input/output routines
#include <process.h>            // threading routines for the CRT
#include <gl\gl.h>              // the OpenGL Library
#include <gl\glu.h>             // the OpenGL Utility Library

// global includes
#include "Resource\Resource.h"  // resource numeric identifiers

#ifndef __cplusplus
    // ensure we have a boolean type in C, we do this because we want "true"
    // "false" to an actual type instead of a define in <stdbool.h>
    typedef enum {false=FALSE, true=TRUE} bool;
#endif

// borrow an idea from C++ boost to have a triple state boolean
typedef enum {no=FALSE, yes=TRUE, maybe=-1} tribool;

// application constants
#define APP_NAME        _T("OpenGL Base")                                    // name of the application
#define CLASS_NAME      _T("GL_BASE_9826C328_598D_4C2E_85D4_0FF8E0310366")   // unique class name of the main window
#define COMPANY_NAME    _T("Wizkit")                                         // company responsible for this software
#define MAX_LOADSTRING  256                                                  // max buffer size for simple string data

// application configurations and defaults
#define CONFIG_ALLOW_FULLSCREEN    TRUE          // should the app allow fullscreen mode
#define CONFIG_ALLOW_RESIZE        FALSE         // can the main window to be resized? (windowed only)
#define CONFIG_ALLOW_MENU          FALSE         // do we have a default menu
#define CONFIG_ALLOW_VSYNC         FALSE         // do allow the enabling/disabling of vertical sync?
#define CONFIG_DEF_BACKGROUND      RGB(0, 0, 0)  // default background color to clear the screen with
#define CONFIG_DEF_BPP             16            // default bits-per-pixel
#define CONFIG_DEF_FULLSCREEN      FALSE         // should the app default to fullscreen or windowed
#define CONFIG_DEF_WIDTH           1024          // default width of the resolution
#define CONFIG_DEF_HEIGHT          768           // default height of the resolution
#define CONFIG_MAX_REFRESH         120           // default max refresh rate to use for fullscreen mode (in hertz)
#define CONFIG_MIN_REFRESH         60            // default min refresh rate to use for fullscreen mode (in hertz)
#define CONFIG_MIN_WIDTH           0             // minimum width of the main window (zero means no min)
#define CONFIG_MIN_HEIGHT          0             // minimum height of the main window (zero means no min)
#define CONFIG_PAUSE_MINIMIZED     TRUE          // do we pause the render when the main window is minimized
#define CONFIG_SINGLE_INSTANCE     TRUE          // do we allow single or multiple instances of the app

#endif  // APPLICATION_H