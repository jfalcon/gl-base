#include "Main\Application.h"       // standard application include
#include "Main\Render.h"            // include for this file
#include "Primitives\Triforce.h"    // include for this file

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////// LEGEEND OF ZELDA TRIFORCE PRIMITIVE /////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*/
/ / PARAMETERS:
/ /     dElapsed = how many CPU cycles per second have passed recently, we use this as
/ /                a factor to help throttle animation either up or down as needed
/ /     nWidth =   width of the redner context in which to draw on
/ /     nHeight =  height of the redner context in which to draw on
/ /
/ / PURPOSE:
/ /     This draws a triforce on the main render context within the scope of the render thread.
/*/

void
TriforcePrimitive (const double dElapsed, const unsigned int nWidth, const unsigned int nHeight)
{
    static GLdouble x = 0.0;
    const static GLfloat MatYellowDiffuse[] = {0.86f, 0.74f, 0.14f, 1.0f};
    const static GLfloat MatOrangeDiffuse[] = {0.78f, 0.59f, 0.0f, 1.0f};

    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);    // clear screen and depth buffers
    glLoadIdentity();                                    // reset modelview matrix

    // get the angle we wish to use
    if(x >= 360.0f) x = 0.0f;

    // move and rotate the cube
    glPushMatrix();
        glTranslatef(-5, -5, -35);
        glRotated(x, 0.0f, 1.0f, 0.0f);

        glColor3ub(220, 190, 35);           // yellow
        glMaterialfv(GL_FRONT, GL_DIFFUSE, MatYellowDiffuse);

        // draw the cube
        glBegin(GL_TRIANGLES);
            glNormal3f(0, 0, -1);
            glVertex3f(-5, -5, -1);         // left
            glVertex3f(0, 5, -1);           // top
            glVertex3f(5, -5, -1);          // right

            glNormal3f(0, 0, 1);
            glVertex3f(-5, -5, 1);          // left
            glVertex3f(0, 5, 1);            // top
            glVertex3f(5, -5, 1);           // right
        glEnd();

        glColor3ub(200, 150, 0);            // dark yellow
        glMaterialfv(GL_FRONT, GL_DIFFUSE, MatOrangeDiffuse);

        glBegin(GL_QUADS);
            glNormal3f(-1, 0, 0);
            glVertex3f(-5, -5, -1);         // bottom left
            glVertex3f(0, 5, -1);           // top left
            glVertex3f(0, 5, 1);            // top right
            glVertex3f(-5, -5, 1);          // bottom right

            glNormal3f(1, 0, 0);
            glVertex3f(5, -5, 1);           // bottom left
            glVertex3f(0, 5, 1);            // top left
            glVertex3f(0, 5, -1);           // top right
            glVertex3f(5, -5, -1);          // bottom right
        glEnd();
    glPopMatrix();

    glPushMatrix();
        glTranslatef(5, -5, -35);
        glRotated(x, 0.0f, 1.0f, 0.0f);

        glColor3ub(220, 190, 35);           // yellow
        glMaterialfv(GL_FRONT, GL_DIFFUSE, MatYellowDiffuse);

        // draw the cube
        glBegin(GL_TRIANGLES);
            glNormal3f(0, 0, -1);
            glVertex3f(-5, -5, -1);         // left
            glVertex3f(0, 5, -1);           // top
            glVertex3f(5, -5, -1);          // right

            glNormal3f(0, 0, 1);
            glVertex3f(-5, -5, 1);          // left
            glVertex3f(0, 5, 1);            // top
            glVertex3f(5, -5, 1);           // right
        glEnd();

        glColor3ub(200, 150, 0);            // dark yellow
        glMaterialfv(GL_FRONT, GL_DIFFUSE, MatOrangeDiffuse);

        glBegin(GL_QUADS);
            glNormal3f(-1, 0, 0);
            glVertex3f(-5, -5, -1);         // bottom left
            glVertex3f(0, 5, -1);           // top left
            glVertex3f(0, 5, 1);            // top right
            glVertex3f(-5, -5, 1);          // bottom right

            glNormal3f(1, 0, 0);
            glVertex3f(5, -5, 1);           // bottom left
            glVertex3f(0, 5, 1);            // top left
            glVertex3f(0, 5, -1);           // top right
            glVertex3f(5, -5, -1);          // bottom right
        glEnd();
    glPopMatrix();

    glPushMatrix();
        glTranslatef(0, 5, -35);
        glRotated(x, 0.0f, 1.0f, 0.0f);

        glColor3ub(220, 190, 35);           // yellow
        glMaterialfv(GL_FRONT, GL_DIFFUSE, MatYellowDiffuse);

        // draw the cube
        glBegin(GL_TRIANGLES);
            glNormal3f(0, 0, -1);
            glVertex3f(-5, -5, -1);         // left
            glVertex3f(0, 5, -1);           // top
            glVertex3f(5, -5, -1);          // right

            glNormal3f(0, 0, 1);
            glVertex3f(-5, -5, 1);          // left
            glVertex3f(0, 5, 1);            // top
            glVertex3f(5, -5, 1);           // right
        glEnd();

        glColor3ub(200, 150, 0);            // dark yellow
        glMaterialfv(GL_FRONT, GL_DIFFUSE, MatOrangeDiffuse);

        glBegin(GL_QUADS);
            glNormal3f(-1, 0, 0);
            glVertex3f(-5, -5, -1);         // bottom left
            glVertex3f(0, 5, -1);           // top left
            glVertex3f(0, 5, 1);            // top right
            glVertex3f(-5, -5, 1);          // bottom right

            glNormal3f(1, 0, 0);
            glVertex3f(5, -5, 1);           // bottom left
            glVertex3f(0, 5, 1);            // top left
            glVertex3f(0, 5, -1);           // top right
            glVertex3f(5, -5, -1);          // bottom right
        glEnd();
    glPopMatrix();

    // rotate 45 degrees every second
    x += 45.0 * dElapsed;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////