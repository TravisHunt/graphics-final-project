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
#include "trackball.h"

using namespace Eigen;
using std::vector;

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
static int DISTANCE_BETWEEN_POINTS = 30;
/**
 * this constant gives the max acceptible angle between two points on the curve that is acceptable
 * smaller numbers give a more accurate mapping
 */
static float VERTEX_LIMIT = 1.0;
/**
 * this constant gives the max acceptible distance between a point across the shape and the normal
 * vector from the opposite side point
 * smaller numbers give a more accurate mapping, however shapes less likely to render correctly
 */
static int DISTANCE_CONSTANT = 50;
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
static int display_triangles = 0;

struct Line {
    Vector3f *p1;
    Vector3f *p2;

    Line():p1(NULL), p2(NULL) {}
    Line(Vector3f &v1, Vector3f &v2)
    : p1(&v1), p2(&v2) {}
};

vector<Vector3f> stroke;          // stroke vertices
vector<Vector3f> points_on_curve; // significant stroke vertices
vector<Vector3f>::iterator it;    // vertex iterator

vector<int> go_back_for;				//list of points to redraw
vector<Line> connected; //list of connected vertices across drawing
vector<Line>::iterator it2; //iterator for connected

vector<Vector3f> vertices_on_shape;

static int recent;						//global variable used in calculating

/*********************************************/


/********** FUNCTION PROTOTYPES *************/

/* INITIALIZATION & HELPERS *************/
/**
 * init
 */
void init(void);

/**
 * enableLighting & disableLighting
 * Sets OpenGL lighting bits to turn lighting effects on and off.
 */
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
 * calculateVerticesDriver
 * @param NONE
 * loops through connected to generate the verticies that make up the shape
 * @return NONE
 */
 void calculateVerticesDriver();

 /**
 * calculateMidpoint
 * @param int index - the index of the two points in connected
 * calculates the midpoint of two verices in connected
 * @return Vector3f - the midpoint of the two points at the given index
 */
 Vector3f calculateMidpoint(int index);

 /**
 * calculateVertices
 * @param int index - the index of the two vertices in connected
 * @param float theta - the degree to rotate the point around the midpoint by
 * Takes the point on the curve and rotates it around the midline in the shape to give a 3d representation of the shape
 * appends this vertex to the vector vertices_on_shape
 * @return NONE
 */
void calculateVertices(int index, float Theta);

/**
 * generateClosingPoints
 * Uses the first and last vertices of a user stroke to create
 * connecting vertices, via the Midpoint formula, to create a closed
 * planar polygon.
 */
void generateClosingPoints(vector<Vector3f> &points);

/**
 * sideLength
 * @param pair a, b - Two vertices.
 * Takes pair a and pair b (2 points on line).
 * returns the distance between the points
 */
float sideLength(Vector3f &a, Vector3f &b);

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

/**
 * populateConnected
 * @param NONE
 * Populates the vector connected with a pairs of points that are across the shape from each other
 * @return NONE
*/
void populateConnected();

/**
 * iterateThrough
 * @param int count_forward - index in points_on_curve to count from
 * @param int count_back - index in points_on_curve to count backward from
 * Iterates over points_on_curve from count forward to count backward
 * @return NONE
*/
void iterateThrough(int count_forward, int count_back);

/**
 * checkPoints_CountForward
 * @param int index1 - the index from the front
 * @param int index2 - the index from the back
 * Finds if a point is within expected area, and if not, recurses until proper point is found
 * Checks from the forward position
 * @return NONE
*/
int checkPoints_CountForward(int index1, int index2);

/**
 * checkPoints_CountBack
 * @param int index1 - the index from the back
 * @param int index2 - the index from the front
 * Finds if a point is within expected area, and if not, recurses until proper point is found
 * Checks from the back position
 * @return int - 0 if not recursed, otherwise the number of indexes skipped
*/
int checkPoints_CountBack(int index1, int index2);

/**
 * isClose
 * @param Vector3f home, normal, point_to_check - the three points necessary to find the distance from the line (home, normal) the the point(point_to_check)
 *
 * Decides if a point is close to a give line. the line is from home point in the normal direction
 * decides if point is close by calculating the point's distance from the line
 * @return bool - true if the point is within DISTANCE_CONSTANT, false if not
*/
bool isClose(Vector3f &home, Vector3f &normal, Vector3f &point_to_check);

/**
 * drawNewLine
 * @param index - The point in points_on_curve from which the line will be drawn
 * @param x - The distance(number of indexes) away the point the line is drawn to.
 * Creates new line given that the given point is too far from the expected position
 * @return NONE
*/
void drawNewLine(Vector3f &a, Vector3f &b, vector<Vector3f> &curve, int offset);
//void drawNewLine(int index, int x);

/**
 * getNormal
 * @param pair a, b - Two vertices.
 * Calculates the normal direction given two points
 * @return pair<int, int> - The point in the direction of the normal vector
*/
Vector3f getNormal(Vector3f &a, Vector3f &b);

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
