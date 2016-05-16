/**
 * mesh.h
 * This file contains a Mesh class that handles the construction and rendering
 * of a mesh given the vertex and triangle face data.
 */

#ifndef _MESH_H_
#define _MESH_H_

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#endif

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <Eigen/Dense>

using std::string;
using std::vector;
using Eigen::Vector2f;
using Eigen::Vector3f;

/**
 * triangle struct
 * Datatype containing triangle face vertices. The struct contains 3
 * integers, which are indices into the array containing all of the
 * vertices within the mesh.
 */
struct Triangle {
    GLuint vertex1, vertex2, vertex3;

    Triangle(void)
        :vertex1(0), vertex2(0), vertex3(0) {}
    Triangle(GLuint v1, GLuint v2, GLuint v3)
        :vertex1(v1), vertex2(v2), vertex3(v3) {}
};

/**
 * vertex struct
 * Datatype containing a mesh vertex position and surface normal vector.
 */
struct Vertex {
    Vector3f position;
    Vector3f normal;
};

class Mesh
{
public:
    // Mesh Data
    vector<Vector3f> vertices;
    vector<Triangle> tri_indices;

    // Constructors
    Mesh(void) {}
    Mesh(vector<Vector3f> &vertices, vector<Triangle> &indices)
    {
        this->vertices    = vertices;
        this->tri_indices = indices;
    }

    // Render mesh
    void draw(void)
    {
        Vector3f v0, v1, v2, u, v;
        GLuint i;
        GLfloat weight = 1.0f;

        glBegin(GL_TRIANGLES);
        for (i = 0; i < tri_indices.size(); i++) {
            // Get triangle vertices
            v0 = vertices[ tri_indices[i].vertex1 ];
            v1 = vertices[ tri_indices[i].vertex2 ];
            v2 = vertices[ tri_indices[i].vertex3 ];

            // Compute triangle surface normal
            u = v1 - v0;
            v = v2 - v0;
            GLfloat nx = u.y() * v.z() - u.z() * v.y();
            GLfloat ny = u.z() * v.x() - u.x() * v.z();
            GLfloat nz = u.x() * v.y() - u.y() * v.x();

            // Check if weight should be applied
            GLfloat check = sqrt(v0(0)*v0(0) + v0(1)*v0(1) + v0(2)*v0(2));
            if (check <= 1 )
                weight = 200.0f;
            else if (check <= 20)
                weight = 50.0f;

            // Apply weights
            v0 *= weight; v1 *= weight; v2 *= weight;

            glNormal3f(nx, ny, nz);
            glVertex3f(v0.x(), v0.y(), v0.z());
            glVertex3f(v1.x(), v1.y(), v1.z());
            glVertex3f(v2.x(), v2.y(), v2.z());
        }
        glEnd();
    }

private:

};

#endif
