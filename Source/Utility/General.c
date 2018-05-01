#include "Main\Application.h"   // standard application include
#include "Utility\General.h"    // include for this file

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////// GENERAL UTILITY ROUTINES //////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// local prototypes
static bool __getRegValue(const HKEY hReg, const LPTSTR szSubkey, const LPTSTR szValue, DWORD dwType, void *pDest, DWORD dwSize);
static bool __setRegValue(const HKEY hReg, const LPTSTR szSubkey, const LPTSTR szValue, DWORD dwType, void *pDest, DWORD dwSize);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*/
/ / PARAMETERS:
/ /     hWnd = handle to the window to align
/ /     nOrientation = bitwise OR with the ALIGN_BOTTOM, ALIGN_CENTER, ALIGN_LEFT,
/ /                    ALIGN_MIDDLE, ALIGN_RIGHT, and ALIGN_TOP constants to position the window with.
/ /     hWndPreferred = handle to the window to move hWnd relative to, doesn't have to be a parent
/ /
/ / PURPOSE:
/ /     Function to move a window in one of nine possible combinations, either
/ /     relative to a preferred window or the work area (screen minus task bar).
/ /
/ / CREDITS:
/ /     This function originated as the CenterWindow function found in the book "Win32 Programming" by Brent
/ /     Brent E. Rector and Joseph M. Newcomer.  I, Jeremy Falcon, modified it to allow for greater movement
/ /     control, and renamed it similar to MoveWindow (Win32 API) to reflect the new functionality.
/*/

bool
AlignWindow (HWND hWnd, unsigned short nOrientation, HWND hWndPreferred)
{
    POINT PreferredPoint = {0};
    RECT  WndRect = {0}, PreferredRect = {0};
    SIZE  WndSize = {0};

    // don't bother unless we have a valid window to align
    if(!IsWindow(hWnd)) return false;

    // move the window around the preferred window if it exists
    // otherwise, move the window relevant to the work area
    if(hWndPreferred != NULL)
    {
        if(!IsWindow(hWndPreferred)) return false;
        GetWindowRect(hWndPreferred, &PreferredRect);
    }
    else
    {
        // attempt to get the dimensions for the work area
        if(!SystemParametersInfo(SPI_GETWORKAREA, sizeof(RECT), &PreferredRect, 0))
        {
            // if the above call failed Explorer is not running correctly; therefore, there is no
            // work area (and task bar). so, we'll use the screen size instead for our coordinates
            SetRect(&PreferredRect, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
        }
    }

    // get the rectangle of the window (to be moved) and compute the width and height
    GetWindowRect(hWnd, &WndRect);
    WndSize.cx = (WndRect.right  - WndRect.left);
    WndSize.cy = (WndRect.bottom - WndRect.top);

    // calculate the left and top positions of the window to be moved base on the flags and its size
    if((nOrientation & ALIGN_BOTTOM) == ALIGN_BOTTOM) PreferredPoint.y = PreferredRect.bottom - WndSize.cy;
    if((nOrientation & ALIGN_CENTER) == ALIGN_CENTER) PreferredPoint.x = (PreferredRect.right - WndSize.cx) / 2;
    if((nOrientation & ALIGN_MIDDLE) == ALIGN_MIDDLE) PreferredPoint.y = (PreferredRect.bottom - WndSize.cy) / 2;
    if((nOrientation & ALIGN_RIGHT) == ALIGN_RIGHT)   PreferredPoint.x = PreferredRect.right - WndSize.cx;

    // reposition the window
    return SetWindowPos(hWnd, NULL, PreferredPoint.x, PreferredPoint.y, 0, 0, SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOZORDER);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*/
/ / PARAMETERS:
/ /     hWnd = parent window handle the message box belongs to (can be NULL)
/ /     uText = resource identifier of the text string to show
/ /     uCaption = resource identifier of the caption string to show (can be zero)
/ /     uType = the style bits for the message box to determine its appearance
/ /
/ / PURPOSE:
/ /     Simplify the usage of the MessageBox() function but with using resource identifiers.
/ /
/ / NOTES:
/ /     This assumes that the resource file belongs to the module (executable) that started the calling process.
/*/

int
ResourceMessage(HWND hWnd, unsigned int uText, unsigned int uCaption, unsigned int uType)
{
    TCHAR szText[MAX_LOADSTRING] = {0};
    HMODULE hModule = GetModuleHandle(NULL);

    if((LoadString(hModule, uText, szText, STRING_SIZE(szText)) > 0))
    {
        TCHAR szCaption[MAX_LOADSTRING] = {0};

        // if hWnd is NULL, make sure the type MB_TASKMODAL is specified
        if((hWnd == NULL) && ((uType & MB_TASKMODAL) != MB_TASKMODAL)) uType |= MB_TASKMODAL;

        if((uCaption > 0) && (LoadString(hModule, uCaption, szCaption, STRING_SIZE(szCaption)) > 0))
            return MessageBox(hWnd, szText, szCaption, uType);
        else
            // default the caption to simply use the application name
            return MessageBox(hWnd, szText, APP_NAME, uType);
    }
    else
        return 0; // indicates an error, same as MessageBox()
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*/
/ / PARAMETERS:
/ /     szSubkey = name of the sub key to look for the values in
/ /     szValue = name of the value to read data from (use NULL for default value)
/ /     dwType = type of data to pull: REG_DWORD, REG_SZ, etc.
/ /     pDest = destination buffer to return the data in, can be of any type
/ /     dwSize = size of the destination buffer (buffer size always, not length of chars for string data)
/ /
/ / PURPOSE:
/ /     Function intended to serve as a means to repeat the repetitive operations
/ /     needed to access registry value data frequently from the local user hive.
/ /
/ / NOTE:
/ /     This will prepend company and software name before the sub key that's passed to automatically
/ /     keep the data organized in the registry and so the caller needn't worry about it.
/*/

bool
GetUserValue (const LPTSTR szSubkey, const LPTSTR szValue, DWORD dwType, void *pDest, DWORD dwSize)
{
    TCHAR szPath[MAX_LOADSTRING] = {0};

    // we need to create the string to prepend to the passed sub key (saves time for the caller)
    // to do this, concatenate the passed value after the company name and app name constants
    _tcscpy_s(szPath, STRING_SIZE(szPath), _T("Software\\"));
    _tcscat_s(szPath, STRING_SIZE(szPath), COMPANY_NAME);
    _tcscat_s(szPath, STRING_SIZE(szPath), _T("\\"));
    _tcscat_s(szPath, STRING_SIZE(szPath), APP_NAME);

    // this can be left null if company/software name is sufficient
    if(szSubkey != NULL)
    {
        _tcscat_s(szPath, STRING_SIZE(szPath), _T("\\"));
        _tcscat_s(szPath, STRING_SIZE(szPath), szSubkey);
    }

    // since this is a wrapper function, call the real one to get the data
    return __getRegValue(HKEY_CURRENT_USER, szPath, szValue, dwType, pDest, dwSize);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*/
/ / PARAMETERS:
/ /     szSubkey = name of the sub key to look for the values in
/ /     szValue = name of the value to read data from (use NULL for default value)
/ /     dwType = type of data to pull: REG_DWORD, REG_SZ, etc.
/ /     pDest = destination buffer to return the data in, can be of any type
/ /     dwSize = size of the destination buffer (buffer size always, not length of chars for string data)
/ /
/ / RETURNS:
/ /     Returns a boolean value to indicate whether or not the argument
/ /     was found, not to be mistaken with the value of the argument.
/ /
/ / PURPOSE:
/ /     Function intended to serve as a means to repeat the repetitive operations needed
/ /     to access registry value data frequently from the local machine hive.
/*/

bool
GetSysValue (const LPTSTR szSubkey, const LPTSTR szValue, DWORD dwType, void *pDest, DWORD dwSize)
{
    // since this is a wrapper function, call the real one to get the data
    return __getRegValue(HKEY_LOCAL_MACHINE, szSubkey, szValue, dwType, pDest, dwSize);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*/
/ / PARAMETERS:
/ /     szSubkey = name of the sub key to look for the values in
/ /     szValue = name of the value to read data from (use NULL for default value)
/ /     dwType = type of data to set; REG_DWORD, REG_SZ, etc.
/ /     pDest = destination buffer to return the data in, can be of any type
/ /     dwSize = size of the destination buffer (buffer size always, not length of chars for string data)
/ /
/ / PURPOSE:
/ /     Function intended to serve as a means to repeat the repetitive operations
/ /     needed to access registry value data frequently from the local user hive.
/ /
/ / NOTE:
/ /     This will prepend company and software name before the sub key that's pass to automatically
/ /     keep the data organized in the registry and so the caller needn't worry about it.
/*/

bool
SetUserValue (const LPTSTR szSubkey, const LPTSTR szValue, DWORD dwType, void *pDest, DWORD dwSize)
{
    TCHAR szPath[MAX_LOADSTRING] = {0};

    // we need to create the string to prepend to the passed sub key (saves time for the caller)
    // to do this, we'll concatenate the passed value after the company name and app name constants
    _tcscpy_s(szPath, STRING_SIZE(szPath), _T("Software\\"));
    _tcscat_s(szPath, STRING_SIZE(szPath), COMPANY_NAME);
    _tcscat_s(szPath, STRING_SIZE(szPath), _T("\\"));
    _tcscat_s(szPath, STRING_SIZE(szPath), APP_NAME);

    // this can be left null if company/software name is sufficient
    if(szSubkey != NULL)
    {
        _tcscat_s(szPath, STRING_SIZE(szPath), _T("\\"));
        _tcscat_s(szPath, STRING_SIZE(szPath), szSubkey);
    }

    // since this is a wrapper function, call the real one to get the data
    return __setRegValue(HKEY_CURRENT_USER, szPath, szValue, dwType, pDest, dwSize);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*/
/ / PARAMETERS:
/ /     szSubkey = name of the sub key to look for the values in
/ /     szValue = name of the value to read data from (use NULL for default value)
/ /     dwType = type of data to set: REG_DWORD, REG_SZ, etc.
/ /     pDest = destination buffer to return the data in, can be of any type
/ /     dwSize = size of the destination buffer (buffer size always, not length of chars for string data)
/ /
/ / PURPOSE:
/ /     Function intended to serve as a means to repeat the repetitive operations
/ /     needed to access registry value data frequently from the local machine hive.
/*/

bool
SetSysValue (const LPTSTR szSubkey, const LPTSTR szValue, DWORD dwType, void *pDest, DWORD dwSize)
{
    // since this is a wrapper function, call the real one to get the data
    return __setRegValue(HKEY_LOCAL_MACHINE, szSubkey, szValue, dwType, pDest, dwSize);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*/
/ / PARAMETERS:
/ /     szArg = the name of the switch / argument to search for
/ /     szDest = pointer to a valid buffer to store the result in
/ /     nLen = length of the destination buffer
/ /
/ / RETURNS:
/ /     Returns a boolean value to indicate whether or not the argument
/ /     was found, not to be mistaken with the value of the argument.
/ /
/ / PURPOSE:
/ /     Searches the command line for a particular argument / value pair in a Unicode
/ /     friendly fashion and 9x support -- unlike CommandLineToArgvW().
/*/

bool
GetCmdLineValue (const LPTSTR szArg, LPTSTR szDest, size_t nLen)
{
    LPTSTR szCmd=NULL;          // pointer to the command line
    LPTSTR szTest=NULL;         // worker variable
    bool bRetVal = false;       // return value

    szCmd = GetCommandLine();   // unicode safe

    // validate our data before continuing
    if(((szArg != NULL) && (_tcslen(szArg) > 0)) && ((szCmd != NULL) && (_tcslen(szCmd) > 0)))
    {
        // look for the / or - delimiters and move back on char in the buffer, this is because we must test
        // for a space before the argument so we can safely assume it's real and not contained in another string
        szTest = _tcspbrk(szCmd, _T("/-"));

        // validate and also make sure this isn't the last char in the string
        if(szTest != NULL && _tcslen(szTest) != 0)
        {
            // if we have a space before the delimiter, then we have an argument, so test it
            szTest = _tcsdec(szCmd, szTest);
            if(STRING_NMATCH(szTest, _T(" "), 1))
            {
                // pass up the space and delimiter to test the argument name
                szTest = _tcsinc(szTest);
                szTest = _tcsinc(szTest);

                if(STRING_NMATCH(szArg, szTest, _tcslen(szArg)))
                {
                    // we have a match, now we need to also check to see to see if the argument has a
                    // value associated with it, so check to see if there is an = after it
                    szTest = _tcsninc(szTest, _tcslen(szArg));
                    if(szTest != NULL && STRING_NMATCH(szTest, _T("="), 1))
                    {
                        // we have one, see what data (until the next space) is there
                        szTest = _tcsinc(szTest);
                        if(szTest != NULL)
                        {
                            LPTSTR szTemp = NULL;

                            szTemp = (LPTSTR)_tcschr(szTest, _T(' '));
                            if(szTemp != NULL)
                            {
                                // take everything the buffer will hold, up until the space
                                if(nLen > (size_t)(szTemp - szTest))
                                    _tcsncpy_s(szDest, nLen, szTest, szTemp - szTest);
                                else
                                    _tcsncpy_s(szDest, nLen, szTest, nLen);
                            }
                            else
                            {
                                // take everything the buffer will hold
                                _tcsncpy_s(szDest, nLen, szTest, nLen);
                            }
                        }
                    }

                    bRetVal = true;
                }
            }
        }
    }

    return bRetVal;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*/
/ / PARAMETERS:
/ /     hReg = handle to the registry key we wish to pull value data from
/ /     szSubkey = name of the sub key in the key pointed to by the handle we wish to pull value data from
/ /     szValue = name of value to pull the value data from
/ /     dwType = data type of the value data specified by the value name
/ /     pDest = buffer will receive the value data specified by the value name
/ /     dwSize = size of the return buffer
/ /
/ / RETURNS:
/ /     Returns a boolean value to indicate whether or not the argument
/ /     was found, not to be mistaken with the value of the argument.
/ /
/ / PURPOSE:
/ /     Local function intended to serve as a means to repeat the repetitive operations
/ /     needed to access registry value data frequently. It should not be called directly.
/*/

static bool
__getRegValue (const HKEY hReg, const LPTSTR szSubkey, const LPTSTR szValue, DWORD dwType, void *pDest, DWORD dwSize)
{
    HKEY hKey = NULL;
    bool bReturn = false;
    DWORD dwRealType = 0;

    // open the key in the registry
    if((RegOpenKeyEx(hReg, szSubkey, 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
        && (hKey != NULL))
    {
        if(pDest != NULL)
        {
            // do not forget, the buffer size for string data must also include a terminating zero
            if(RegQueryValueEx(hKey, szValue, NULL, &dwRealType, (LPBYTE)pDest, &dwSize) == ERROR_SUCCESS)
            {
                // here we add our own bit of type safety (b/c the API really doesn't), if the returned type
                // doesn't match the type passed, we wipe the buffer and return false to avoid a casting err
                if(dwRealType != dwType)
                    SecureZeroMemory(pDest, dwSize);
                else
                    bReturn = true;
            }
        }

        // clean up
        RegCloseKey(hKey);
    }

    return bReturn;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*/
/ / PARAMETERS:
/ /     hReg = handle to the registry key we wish to pull value data from
/ /     szSubkey = name of the sub key in the key pointed to by the handle we wish to pull value data from
/ /     szValue = name of value to pull the value data from
/ /     dwType = data type of the value data specified by the value name
/ /     pDest = buffer will receive the value data specified by the value name
/ /     dwSize = size of the return buffer
/ /
/ / PURPOSE:
/ /     Local function intended to serve as a means to repeat the repetitive operations
/ /     needed to access registry value data frequently. It should not be called directly.
/*/

static bool
__setRegValue (const HKEY hReg, const LPTSTR szSubkey, const LPTSTR szValue, DWORD dwType, void *pDest, DWORD dwSize)
{
    HKEY hKey = NULL;
    bool bReturn = false;

    // open (or create if it doesn't exist) the key in the registry
    if((RegCreateKeyEx(hReg, szSubkey, 0, NULL, REG_OPTION_NON_VOLATILE,
        KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS) && (hKey != NULL))
    {
        if(pDest != NULL)
        {
            // do not forget, the buffer size for string data must also include a terminating zero
            if(RegSetValueEx(hKey, szValue, 0, dwType, pDest, dwSize) == ERROR_SUCCESS)
                bReturn = true;
        }

        // clean up
        RegCloseKey(hKey);
    }

    return bReturn;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////