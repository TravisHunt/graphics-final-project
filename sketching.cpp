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

#define IMAGEWIDTH  500
#define IMAGEHEIGHT 500
#define RGBBLACK 0,0,0
#define RGBGREY  .8,.8,.8
#define MIN_DIST 6

static int tracking = 0;
float previousX, previousY;

std::vector<std::pair<int, int> > stroke;
std::vector<std::pair<int, int> > closing;

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

        /* push vertex into vector */
        stroke.push_back(std::make_pair(x, y));

        /* draw line to scene */
        glFlush();

        /* keep track of previous coordinates */
        previousX = x;
        previousY = y;
    }
}

/**
 * getMidpoint
 * Returns the (x,y) midpoint of the two vertices given, which is
 * calculated with the Midpoint formula ((x1+x2)/2, (y1+y2)/2)
 */
std::pair<int,int> getMidpoint(std::pair<int,int>&v1, std::pair<int,int>&v2) {
    int x = (v1.first  + v2.first ) / 2;
    int y = (v1.second + v2.second) / 2;
    return (std::make_pair(x, y));
}

/**
 * subdivide
 * Recursively calculates vertices needed to connect the start and end
 * point of a stroke when the user completes their stroke.
 */
void subdivide(std::pair<int,int>&v1, std::pair<int,int>&v2) {
    std::pair<int,int> mid;
    int x = v2.first - v1.first, y = v2.second - v1.second;
    int dist = sqrt(x*x + y*y);

    /* if we need to subdivide again */
    if (dist >= MIN_DIST) {
        mid = getMidpoint(v1, v2);
        subdivide(v1, mid);
        closing.push_back(mid);
        subdivide(mid, v2);
    }
}

/**
 * generateClosingPoints
 * Uses the first and last vertices of a user stroke to create 
 * connecting vertices, via the Midpoint formula, to create a closed
 * planar polygon.
 */
void generateClosingPoints() {
    closing.clear();
    closing.push_back(stroke.front());
    subdivide(stroke.front(), stroke.back());
    closing.push_back(stroke.back());

    glBegin(GL_POINTS);
        for (int i = 0; i < closing.size(); i++) {
            glVertex2f(closing[i].first, closing[i].second);
        }
    glEnd();
    glFlush();

    std::cout << closing.size() << std::endl;
}

/**
 * drawLine
 * Draws a line between two given points 
 */
void drawLine(std::pair<int,int>&v1, std::pair<int,int>&v2) {
    glBegin(GL_LINES);
        glVertex2f(v1.first, v1.second);
        glVertex2f(v2.first, v2.second);
    glEnd();
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
    } else { /* stroke complete */
        tracking = 0;

        /* get start & end connecting vertices */
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

int main(int argc, char *argv[]) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(IMAGEWIDTH, IMAGEHEIGHT);
    glutCreateWindow("Sketch Interface");
    glutDisplayFunc(drawMouse);
    glutMouseFunc(handleButtonPress);
    glutMotionFunc(mouseMotion);
    gluOrtho2D(0, IMAGEWIDTH, 0, IMAGEHEIGHT);

    glutMainLoop();
    return 0;
}
