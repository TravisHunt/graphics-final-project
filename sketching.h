/**
 * sketching.h
 * This file contains the Sketching program's main functions the do the work
 * necessary in generating the 3D model given a 2D freeform drawing.
 */

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
#include <string>
#include <cmath>
#include <vector>
#include <Eigen/Dense>
#include "view.h"
#include "trackball.h"
#include "mesh.h"
#include "objIO.h"

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
static GLint DISTANCE_BETWEEN_POINTS = 5;
/**
 * this constant gives the max acceptible angle between two points on the curve that is acceptable
 * smaller numbers give a more accurate mapping
 */
static GLfloat VERTEX_LIMIT = 1.0;
/**
 * this constant gives the max acceptible distance between a point across the shape and the normal
 * vector from the opposite side point
 * smaller numbers give a more accurate mapping, however shapes less likely to render correctly
 */
static GLint DISTANCE_CONSTANT = 50;
/*******************************************/

/********** GLOBAL VARIABLES ***************/
const GLdouble PI = 3.141592653589793238462643383279502884197;
const GLint mesh_rotation = 6;
const GLint numCirclePts = 360 / mesh_rotation;

View view;

static GLint tracking;                    // state of stroke tracking
static GLint imageWidth, imageHeight;     // window pixel dimensions
static GLint previousX, previousY;        // previous (x,y) for stroke tracking
static GLint display_triangles = 0;
static GLint triangulated = 0;

struct Line {
    Vector3f *p1;
    Vector3f *p2;

    Line():p1(NULL), p2(NULL) {}
    Line(Vector3f &v1, Vector3f &v2)
    : p1(&v1), p2(&v2) {}
};

struct Circle {
    vector<GLuint> verts;
};

vector<Vector3f> stroke;            // stroke vertices
vector<Vector3f> points_on_curve;   // significant stroke vertices
vector<GLint> go_back_for;			// list of points to redraw
vector<Line> connected;             // list of connected vertices
vector<Circle> mesh_circles;        // interpolated 3D geometry
vector<GLint> check_verts;          // indices of vertices to check
vector<Vector3f> mesh_verts;        // mesh vertices
vector<Triangle> mesh_faces;        // mesh faces
Vector3f last_in_shape;

static GLint recent;		//global variable used in calculating

/*********************************************/


/********** FUNCTION PROTOTYPES *************/

/* INITIALIZATION & HELPERS *************/
/**
 * init
 * Initialization function
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
GLint inWindow(GLint x, GLint y);

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
 * populateMeshFaces
 * Populates the mesh_faces vector with instances of Triangle, detailing
 * which vertices form triangle faces on the mesh.
 */
void populateMeshFaces(void);

/**
 * calculateVerticesDriver
 * @param NONE
 * loops through connected to generate the verticies that make up the shape
 * @return NONE
 */
 void calculateVerticesDriver();

 /**
 * calculateMidpoint
 * @param GLint index - the index of the two points in connected
 * calculates the midpoint of two verices in connected
 * @return Vector3f - the midpoint of the two points at the given index
 */
 Vector3f calculateMidpoint(GLint index);

 /**
 * calculateVertices
 * @param GLint index - the index of the two vertices in connected
 * @param GLfloat theta - the degree to rotate the point around the midpoint by
 * Takes the point on the curve and rotates it around the midline in the shape to give a 3d representation of the shape
 * appends this vertex to the vector mesh_verts
 * @return NONE
 */
void calculateVertices(GLint index, GLfloat Theta);

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
GLfloat sideLength(Vector3f &a, Vector3f &b);

/**
 * calcAngle
 * @param GLfloat BA, BC, AC - Distances between points A,B,C on a triangle
 * Takes three lengths of a triangle and calculates the angle at one vertex.
 * @return GLfloat - Angle in degrees
 */
GLfloat calcAngle(GLfloat BA, GLfloat BC, GLfloat AC);

/**
 * findNextPoint
 * @param GLint i - Index of stroke vertex
 * @param GLint distance - distance to next stroke vertex
 * Uses a stroke vertex and the distance for the next point location
 * to find the next significant vertex in the stroke.
 * @return GLint - Index of next significant stroke vertex
 */
GLint findNextPoint(GLint i, GLint distance);

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
 * @param GLint count_forward - index in points_on_curve to count from
 * @param GLint count_back - index in points_on_curve to count backward from
 * Iterates over points_on_curve from count forward to count backward
 * @return NONE
*/
void iterateThrough(GLint count_forward, GLint count_back);

/**
 * checkPoints_CountForward
 * @param GLint index1 - the index from the front
 * @param GLint index2 - the index from the back
 * Finds if a point is within expected area, and if not, recurses until proper point is found
 * Checks from the forward position
 * @return NONE
*/
GLint checkPoints_CountForward(GLint index1, GLint index2);

/**
 * checkPoints_CountBack
 * @param GLint index1 - the index from the back
 * @param GLint index2 - the index from the front
 * Finds if a point is within expected area, and if not, recurses until proper point is found
 * Checks from the back position
 * @return GLint - 0 if not recursed, otherwise the number of indexes skipped
*/
GLint checkPoints_CountBack(GLint index1, GLint index2);

/**
 * getNormal
 * @param pair a, b - Two vertices.
 * Calculates the normal direction given two points
 * @return pair<GLint, GLint> - The point in the direction of the normal vector
*/
Vector3f getNormal(Vector3f &a, Vector3f &b);

/*****************************************/
/* GLUT CALLBACK FUNCTIONS ***************/
void display(void);
void reshape(int w, int h);
void mouse(int button, int state, int x, int y);
void mouseMotion(int x, int y);
void keyboard(unsigned char key, int x, int y);
/*********************************************/

#endif
