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
#define IMAGE_WIDTH   800
#define IMAGE_HEIGHT  500
#define RGBWHITE      1.0f, 1.0f, 1.0f
#define RGBBLACK      0.0f, 0.0f, 0.0f
#define RGBGREY       0.8f, 0.8f, 0.8f
#define VIEW_RGBA_2D  0.8f, 0.8f, 0.8f, 1.0f
#define VIEW_RGBA_3D  0.3f, 0.3f, 0.3f, 1.0f

/**
 * this constant gives the space between the important points on the curve
 * must be divisible by 2 at least twice
 * smaller numbers give smaller meshes
 * number cannot be a power of 2
 */
static int DISTANCE_BETWEEN_POINTS = 50;
/**
 * this constant gives the max acceptible angle between two points on the curve that is acceptable
 * smaller numbers give a more accurate mapping
 */
static float VERTEX_LIMIT = 0.5;
/*******************************************/

/********** GLOBAL VARIABLES ***************/
const double PI = 3.141592653589793238462643383279502884197;

View view;
int menu_2Dview;
int menu_3Dview;
enum MenuOption { CLEAR, SWITCH_2D, SWITCH_3D, TRIANGULATE_2D };

static int tracking;                    // state of stroke tracking
static int imageWidth, imageHeight;     // window pixel dimensions
static int previousX, previousY;        // previous (x,y) for stroke tracking

vector<pair<int,int> > stroke;          // stroke vertices
vector<pair<int,int> > points_on_curve; // significant stroke vertices
vector<pair<int,int> >::iterator it;    // vertex iterator

/*********************************************/


/********** FUNCTION PROTOTYPES *************/

/* INITIALIZATION & HELPERS *************/
/**
 * init
 */
void init(void);

void enableLighting(void);
void disableLighting(void);

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

/*****************************************/
/* INTERPOLATION *************************/

/**
 * generateClosingPoints
 * Uses the first and last vertices of a user stroke to create
 * connecting vertices, via the Midpoint formula, to create a closed
 * planar polygon.
 */
void generateClosingPoints(void);

/**
 * sideLength
 * @param pair a, b - Two vertices.
 * Takes pair a and pair b (2 points on line).
 * returns the distance between the points
 */
float sideLength(pair<int,int> a, pair<int,int> b);

/**
 * calcAngle
 * @param float BA, BC, AC - Distances between points A,B,C on a triangle
 * Takes three lengths of a triangle and calculates the angle at one vertex.
 * @return float - Angle in degrees
 */
float calcAngle(float BA, float BC, float AC);

/**
 * findNextPoint
 * @param int i - Index of stroke vertex
 * @param int distance - distance to next stroke vertex
 * Uses a stroke vertex and the distance for the next point location
 * to find the next significant vertex in the stroke.
 * @return int - Index of next significant stroke vertex
 */
int findNextPoint(int i, int distance);

/**
 * getOutsideEdges
 * @param NONE
 * Populates the vector points_on_curve with points from the vector stroke.
 * @return NONE
*/
void getOutsideEdges(void);

/******************************************/
/* MENU HANDLING **************************/

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


/*****************************************/
/* GLUT CALLBACK FUNCTIONS ***************/

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
/*********************************************/

#endif
