#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#endif

#include <iostream>
#include <cmath>
#include <vector>

using std::vector;
using std::pair;

#define IMAGEWIDTH  500
#define IMAGEHEIGHT 500
#define RGBBLACK 0,0,0
#define RGBGREY  .8,.8,.8
#define MIN_DIST 6

static int tracking = 0;
float previousX, previousY;

/**
* stroke
* Vector of int tuples that represent the user stroke's vertices.
*/
vector<pair<int,int> > stroke;

/**
 * inWindow
 * Returns true if (x, y) is within the drawing window.
 * Returns false otherwise.
 */
int inWindow(int x, int y) {
    return (x > 0 && x < IMAGEWIDTH && y > 0 && y < IMAGEHEIGHT);
}

/**
 * mouseMotion
 * Tracks the user's mouse when drawing a stroke, pushes the current
 * vertex into the stroke vector, and draws the new line segment.
 */
void mouseMotion(int x, int y) {
    y = IMAGEHEIGHT - y;

    if (tracking && inWindow(x, y)) {
        glBegin(GL_LINES);
            glVertex2f(previousX, previousY);
            glVertex2f(x, y);
        glEnd();

        // push vertex into vector
        stroke.push_back(std::make_pair(x, y));

        // draw line to scene
        glFlush();

        // keep track of previous coordinates
        previousX = x;
        previousY = y;
    }
}

/**
 * generateClosingPoints
 * Uses the first and last vertices of a user stroke to create
 * connecting vertices, via the Midpoint formula, to create a closed
 * planar polygon.
 */
void generateClosingPoints() {
    GLfloat t, delta, cx, cy;

    // interpolate vertices from Endpoint to Startpoint
    pair<int,int> P0(stroke.back());
    pair<int,int> P1(stroke.front());

    // Linearly interpolate points
    t = 0.0; delta = 0.01;
    glBegin(GL_LINE_STRIP);
    for (t = 0.0; t <= 1.0; t += delta) {
        cx = (1.0 - t) * P0.first + t * P1.first;
        cy = (P0.second + (P1.second - P0.second) *
            ((cx - P0.first) / (P1.first - P0.first)));

        // draw line between new point and last point
        glVertex2f(cx, cy);
        // add interpolated point to stroke
        stroke.push_back(std::make_pair(cx, cy));
    }
    glEnd();
    // push closing line to screen
    glFlush();
}

/**
 * handleButtonPress
 * Updates mouse tracking data when a stroke is being drawn.
 */
void handleButtonPress(int button, int state, int x, int y) {
    y = IMAGEHEIGHT - y;

    if (button != GLUT_LEFT_BUTTON) return;

    if (state == GLUT_DOWN) {
        if (inWindow(x, y)) {
            tracking = 1;
            previousX = x;
            previousY = y;
        }
    } else { // stroke complete
        tracking = 0;
        // get start & end connecting vertices
        generateClosingPoints();
        stroke.clear();
    }
}

void drawMouse(void) {
    glClearColor(RGBGREY, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    glColor3f(RGBBLACK);

    glFlush();
}

/* clears the screen of any user strokes */
void wipeCanvas(void) {
    glClearColor(RGBGREY, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    glFlush();
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 27: // escape key
            exit(0);
            break;
    }
}

int main(int argc, char *argv[]) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(IMAGEWIDTH, IMAGEHEIGHT);
    glutCreateWindow("Sketch Interface");
    glutDisplayFunc(drawMouse);
    glutMouseFunc(handleButtonPress);
    glutMotionFunc(mouseMotion);
    glutKeyboardFunc(keyboard);
    gluOrtho2D(0, IMAGEWIDTH, 0, IMAGEHEIGHT);

    glutMainLoop();
    return 0;
}
