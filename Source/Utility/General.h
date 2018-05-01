#if !defined (GENERAL_H_8CBAD4BE_8078_455A_9073_20CEF563CC0A_)
#define GENERAL_H_8CBAD4BE_8078_455A_9073_20CEF563CC0A_

#pragma once // in case the compiler supports it

// increases readability for c-style string comparisons (case insensitive and Unicode safe)
#define STRING_MATCH(szOne, szTwo)        (_tcsicmp(szOne, szTwo) == 0)
#define STRING_NMATCH(szOne, szTwo, nLen) (_tcsnicmp(szOne, szTwo, nLen) == 0)

// perform safe string buffer size operations regardless if we use Unicode or not
#define STRING_SIZE(szBuffer) (sizeof(szBuffer) / sizeof(TCHAR))

// simplify the testing of a minimum version of Windows requirement being met
#define WIN_MIN_VERSION(nMajor, nMinor) (((BYTE)LOBYTE(LOWORD(GetVersion())) >= nMajor) && ((BYTE)HIBYTE(LOWORD(GetVersion())) >= nMinor))

// simplify the testing of whether or not the host OS is NT-based or 9x
#define IS_WINNT (GetVersion() < (DWORD)0x80000000)

// HWND_BOTTOM, HWND_TOP, etc. are already defined in Winuser.h, and we don't want to cause possible
// compatibility issues. so instead, define our own constants for use with AlignWindow().
#define ALIGN_BOTTOM	0x0001
#define ALIGN_CENTER	0x0002
#define ALIGN_LEFT		0x0004
#define ALIGN_MIDDLE	0x0008
#define ALIGN_RIGHT     0x0010
#define ALIGN_TOP		0x0020

// windowing functions
bool AlignWindow (HWND hWnd, unsigned short nOrientation, HWND hWndPreferred);

// message box function to simplify reading of resource strings
int ResourceMessage(HWND hWnd, unsigned int uText, unsigned int uCaption, unsigned int uType);

// registry functions
bool GetUserValue (const LPTSTR szSubkey, const LPTSTR szValue, DWORD dwType, void *pDest, DWORD dwSize);
bool GetSysValue  (const LPTSTR szSubkey, const LPTSTR szValue, DWORD dwType, void *pDest, DWORD dwSize);
bool SetUserValue (const LPTSTR szSubkey, const LPTSTR szValue, DWORD dwType, void *pDest, DWORD dwSize);
bool SetSysValue  (const LPTSTR szSubkey, const LPTSTR szValue, DWORD dwType, void *pDest, DWORD dwSize);

// command line functions
bool GetCmdLineValue (const LPTSTR szArg, LPTSTR szDest, size_t nLen);

#endif  // GENERAL_H