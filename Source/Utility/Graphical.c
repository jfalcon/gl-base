#include "Main\Application.h"   // standard application include
#include "Utility\General.h"    // general utility routines
#include "Utility\Graphical.h"  // include for this file

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////// GRAPHICAL UTILITY ROUTINES /////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*/
/ / PARAMETERS:
/ /     none
/ /
/ / PURPOSE:
/ /     Returns the number of successive CPU cycles in hertz (aka, per second) since the last
/ /     time this routine was called. Typically used for timing animations.
/ /
/ / CREDITS:
/ /        This routine was based on the work of Jeffry J. Brickley.
/*/

double
GetCPUTicks (void)
{
    static LARGE_INTEGER nFreq = {0}, nCount = {0};
    static double dReturn = 0.0, dCheckTime = 5.0;

    // check for a new frequency once every 5 seconds
    // this is in case ACPI, etc. alters it
    if((nFreq.QuadPart == 0) || (dCheckTime < dReturn))
    {
        dCheckTime = dReturn + 5.0;

        // avoid a division by zero by returning zero on error
        if(!QueryPerformanceFrequency(&nFreq)) return 0.0;
    }

    // use the ratio of tick amount divided by frequency to find the hertz
    QueryPerformanceCounter(&nCount);

    dReturn = ((double)nCount.QuadPart / (double)nFreq.QuadPart);
    return dReturn;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*/
/ / PARAMETERS:
/ /     szProcedure = intended to serve as the procedure name the error occurred in
/ /
/ / PURPOSE:
/ /     This routine handles OGL errors in whichever way deemed fit
/ /     by the application, whether it be messaging, logging, etc.
/ /
/ / NOTES:
/ /     This must be called in the context of the thread it belongs to.
/*/

#ifdef _DEBUG
bool
OnGLError (LPCTSTR szProcedure)
{
    GLenum glErr = GL_NO_ERROR;
    bool bReturn = false;
    TCHAR szOutput[MAX_LOADSTRING] = {0}, szError[MAX_LOADSTRING] = {0};

    // to avoid 5 million message boxes, do not use a loop for this
    if((glErr = glGetError()) != GL_NO_ERROR)
    {
        // note: we don't need to use resource strings for this
        // because this is for debugging only, thus it only needs
        // to be in one language - the one being developed in
        switch(glErr)
        {
            case GL_INVALID_ENUM:
                _tcscpy_s(szError, STRING_SIZE(szError), _T("Invalid enumeration value!"));
                break;

            case GL_INVALID_VALUE:
                _tcscpy_s(szError, STRING_SIZE(szError), _T("Invalid value!"));
                break;

            case GL_INVALID_OPERATION:
                _tcscpy_s(szError, STRING_SIZE(szError), _T("Invalid operation!"));
                break;

            case GL_STACK_OVERFLOW:
                _tcscpy_s(szError, STRING_SIZE(szError), _T("Stack Overflow!"));
                break;

            case GL_STACK_UNDERFLOW:
                _tcscpy_s(szError, STRING_SIZE(szError), _T("Stack Underflow!"));
                break;

            case GL_OUT_OF_MEMORY:
                _tcscpy_s(szError, STRING_SIZE(szError), _T("Out of memory!"));
                break;
        }

        // display a message to the user
        _stprintf_s(szOutput, STRING_SIZE(szOutput),
            _T("The OpenGL subsystem has generated the following error.\n\nError Info:\t%s\nProcedure:\t%s"), szError, szProcedure);

        MessageBox(NULL, szOutput, _T("Debug Mode Output"), MB_OK|MB_ICONERROR|MB_TASKMODAL);

        // let the caller know we had an error
        bReturn = true;
    }

    return bReturn;
}
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*/
/ / PARAMETERS:
/ /     bSync = flag to indicate if vsync (vertical sync) will be enabled or disabled
/ /
/ / PURPOSE:
/ /     This is will either enable or disable vertical sync using a wiggle extension.
/ /
/ / NOTE:
/ /     On some video cards (like nVidia), this can be overridden so an application cannot change this.
/ /     Also, this needs to be called after there's a valid RC (Render Context).
/*/

void
SetVerticalSync (bool bSync)
{
    // this does not support Unicode, but that's ok because the user
    // will never see the string data that we test with
    const char *szExt = (char *)glGetString(GL_EXTENSIONS);

    // only attempt this if the extension is supported
    if((szExt != NULL) && (strstr(szExt, "WGL_EXT_swap_control") != NULL))
    {
        typedef bool (APIENTRY *PFNWGLSWAPINTERVALFARPROC) (int);
        PFNWGLSWAPINTERVALFARPROC wglSwapIntervalEXT = NULL;

        // this accepts one parameter, 0 = off and 1 = on
        wglSwapIntervalEXT = (PFNWGLSWAPINTERVALFARPROC)wglGetProcAddress("wglSwapIntervalEXT");

        if(wglSwapIntervalEXT != NULL) wglSwapIntervalEXT(bSync);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////