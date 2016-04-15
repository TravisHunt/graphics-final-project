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
#include <Eigen/Dense>
#include "view.h"

using std::vector;
using std::pair;

#define IMAGE_WIDTH  800
#define IMAGE_HEIGHT 500
#define RGBBLACK     0,0,0
#define RGBGREY     .8,.8,.8

//View view;

static int tracking;
static int imageWidth, imageHeight;
static int previousX, previousY;

/**
* stroke
* Vector of int tuples that represent the user stroke's vertices.
*/
vector<pair<int,int> > stroke;
vector<pair<int,int> >::iterator it;

/**
 * inWindow
 * Returns true if (x, y) is within the drawing window.
 * Returns false otherwise.
 */
int inWindow(int x, int y) {
    return (x > 0 && x < imageWidth && y > 0 && y < imageHeight);
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
    for (t = 0.0; t <= 1.0; t += delta) {
        cx = (1.0 - t) * P0.first + t * P1.first;
        cy = (P0.second + (P1.second - P0.second) *
            ((cx - P0.first) / (P1.first - P0.first)));

        // add interpolated point to stroke
        stroke.push_back(std::make_pair(cx, cy));
    }
}

/* clears the screen of any user strokes */
void wipeCanvas(void) {
    glClearColor(RGBGREY, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    glFlush();
}

/* Destroys stored stroke vertices and resets the canvas */
void resetStroke(void) {
    stroke.clear();
    tracking  = 0;
    previousX = 0;
    previousY = 0;
    wipeCanvas();
}

void init(void) {
    //view.setCOI(0, 0, 0); // set coi to origin
    //view.setEyePos(1, 1, 1); // camera position
    tracking = 0;
    previousX = 0;
    previousY = 0;
}

/*============= GLUT CALLBACK FUNCTIONS */

/**
* display
*/
void display(void) {
    glClearColor(RGBGREY, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glColor3f(RGBBLACK);

    // Draw user stroke's vertices
    glBegin(GL_LINE_LOOP);
    for (it = stroke.begin(); it != stroke.end(); it++) {
        glVertex2f(it->first, it->second);
    }
    glEnd();

    glFlush();
}

/**
* reshape
*/
void reshape(int w, int h) {
    int deltaW  = (w - imageWidth)  >> 1;
    int deltaH  = (h - imageHeight) >> 1;
    imageWidth  = w;
    imageHeight = h;

    // Force window to maintain minimum size
    if (w < IMAGE_WIDTH && h < IMAGE_HEIGHT)
        glutReshapeWindow(IMAGE_WIDTH, IMAGE_HEIGHT);
    else if (w < IMAGE_WIDTH)
        glutReshapeWindow(IMAGE_WIDTH, h);
    else if (h < IMAGE_HEIGHT)
        glutReshapeWindow(w, IMAGE_HEIGHT);

    // Update stroke vertices to keep stroke centered
    for (it = stroke.begin(); it != stroke.end(); it++) {
        it->first  += deltaW;
        it->second += deltaH;
    }

    // reset viewport to the dimensions
    glViewport(0, 0, w, h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluOrtho2D(0, imageWidth, 0, imageHeight);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

/**
 * mouse
 * Updates mouse tracking data when a stroke is being drawn.
 */
void mouse(int button, int state, int x, int y) {
    y = imageHeight - y;

    if (button != GLUT_LEFT_BUTTON) return;

    if (state == GLUT_DOWN) {
        if (inWindow(x, y)) {
            // New stroke
            if (!tracking) {
                resetStroke();
                tracking = 1;
            }
            // Update coordinates
            previousX = x;
            previousY = y;
        }
    } else { // stroke complete
        tracking = 0;
        previousX = 0;
        previousY = 0;
        // get start & end connecting vertices
        generateClosingPoints();
        // redraw stroke
        glutPostRedisplay();
    }
}

/**
 * mouseMotion
 * Tracks the user's mouse when drawing a stroke, pushes the current
 * vertex into the stroke vector, and draws the new line segment.
 */
void mouseMotion(int x, int y) {
    y = imageHeight - y;

    if (tracking && inWindow(x, y)) {
        // push vertex into vector
        stroke.push_back(std::make_pair(x, y));

        glBegin(GL_LINES);
            glVertex2f(previousX, previousY);
            glVertex2f(x, y);
        glEnd();

        // draw line to scene
        glFlush();

        // keep track of previous coordinates
        previousX = x;
        previousY = y;
    }
}

/**
* keyboard
*/
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

    imageWidth  = IMAGE_WIDTH;
    imageHeight = IMAGE_HEIGHT;

    glutInitWindowSize(imageWidth, imageHeight);
    glutCreateWindow("Sketching Interface");
    glutInitWindowPosition((glutGet(GLUT_SCREEN_WIDTH)-imageWidth)/2,
                           (glutGet(GLUT_SCREEN_HEIGHT)-imageWidth)/2);

    init();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouse);
    glutMotionFunc(mouseMotion);
    glutKeyboardFunc(keyboard);

    glutMainLoop();
    return 0;
}
