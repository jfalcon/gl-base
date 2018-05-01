#if !defined (GRAPHICAL_H_0E27CE88_B088_4DD3_AB1C_D28C05189A82_)
#define GRAPHICAL_H_0E27CE88_B088_4DD3_AB1C_D28C05189A82_

double GetCPUTicks     (void);
void   SetVerticalSync (bool bSync);

#ifdef _DEBUG
    // helper function(s) for OGL error reporting
    bool OnGLError (LPCTSTR szProcedure);

    // helper macro(s) for OGL error reporting
    #define ENTER_GL          while((glGetError() != GL_NO_ERROR) && (glGetError() != GL_INVALID_OPERATION));
    #define LEAVE_GL(szProc)  OnGLError(szProc);
#else
    #define ENTER_GL
    #define LEAVE_GL(szProc)  false
#endif

#endif  // GRAPHICAL_H