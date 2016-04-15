#ifndef _SKETCHING_H
#define _SKETCHING_H

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

/************* MACROS **************/
#define IMAGE_WIDTH  800
#define IMAGE_HEIGHT 500
#define RGBBLACK     0,0,0
#define RGBGREY     .8,.8,.8

/********** GLOBAL VARIABLES ***************/
//View view;
int menu_2Dview;
int menu_3Dview;
enum MenuOption { CLEAR, SWITCH_2D, SWITCH_3D };

static int tracking;
static int imageWidth, imageHeight;
static int previousX, previousY;

vector<pair<int,int> > stroke;
vector<pair<int,int> >::iterator it;


/*********** FUNCTION PROTOTYPES *****************/

/**
 * init
 */
void init(void);

/**
 * inWindow
 * Returns true if (x, y) is within the drawing window.
 * Returns false otherwise.
 */
int inWindow(int x, int y);

/**
 * wipeCanvas
 * Clears the screen of any user strokes.
 */
void wipeCanvas(void);

/**
 * resetStroke
 * Destroys stored stroke vertices and resets the canvas.
 */
void resetStroke(void);

/**
 * generateClosingPoints
 * Uses the first and last vertices of a user stroke to create
 * connecting vertices, via the Midpoint formula, to create a closed
 * planar polygon.
 */
void generateClosingPoints(void);

/**
 * createGLUTMenus
 * Initializes glut menus.
 */
void createGLUTMenus(void);

/**
 * handleMenuEvents
 * @param int option - Menu option identifier
 * Performs desired action when a menu option is selected.
 */
void handleMenuEvents(int option);

/**
 * destroyGLUTMenus
 * Frees resources allocated for glut menus.
 */
void destroyGLUTMenus(void);


/*******************************************/
/******** GLUT CALLBACK FUNCTIONS **********/

/**
 * display
 */
void display(void);

/**
 * reshape
 */
void reshape(int w, int h);

/**
 * mouse
 * Updates mouse tracking data when a stroke is being drawn.
 */
void mouse(int button, int state, int x, int y);

/**
 * mouseMotion
 * Tracks the user's mouse when drawing a stroke, pushes the current
 * vertex into the stroke vector, and draws the new line segment.
 */
void mouseMotion(int x, int y);

/**
* keyboard
*/
void keyboard(unsigned char key, int x, int y);


/********************************************/

#endif
