#ifndef _OBJ_IO_H_
#define _OBJ_IO_H_

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#include <errno.h>
#include <fstream>
#include <cstring>
#include <string>
#include <vector>
#include <iostream>
#include <Eigen/Core>
#include "mesh.h"

using std::vector;
using Eigen::Vector3f;

#define BUFF_SIZE 512

/**
 * loadObj
 * .obj file parsing function accepts a .obj file and loads the vertex
 * and triangle face information.
 */
GLboolean
loadObj(GLchar *fname, vector<Vector3f> &verts, vector<Triangle> &faces)
{
    GLchar cbuff[BUFF_SIZE];
    GLuint numVertices = 0, numFaces = 0;
    GLboolean normals = GL_FALSE, texture = GL_FALSE;
    GLint tint;
    GLchar ch;
    GLint p, q, r;
    GLdouble x, y, z;
    vector<Vector3f>::iterator v;
    vector<Triangle>::iterator f;
    std::ifstream in1(fname, std::ios::in);

    if (!in1.is_open()) return false;
    in1.flags(in1.flags() & ~std::ios::skipws);

    while (in1 >> ch) {
        if (ch == 'v') {
            in1 >> ch;
            if (ch == ' ') numVertices++;
            else if (ch == 'n') normals = GL_TRUE;
            else if (ch == 't') texture = GL_TRUE;
            else std::cerr << "ERROR \'" << ch << "\'" << std::endl;
        } else if (ch == '#') {
            // Read to the end of the line
            while (in1 >> ch && ch != '\n');
        } else if (ch == 'f') numFaces++;
    }
    in1.close();

    verts.resize(numVertices);
    faces.resize(numFaces);
    v = verts.begin();
    f = faces.begin();

    std::ifstream in(fname, std::ios::in);
    if (!in.is_open()) return GL_FALSE;

    while (in >> ch) {
        switch (ch) {
        case '#':
        case 'g':
        case 's':
        case 'm':
        case 'u':
            in.getline(cbuff, BUFF_SIZE);
            continue;

        case 'v':
            ch = in.peek();
            if (ch != 't' && ch != 'n') {
                in >> x >> y >> z;
                *(v++) = Vector3f(x, y, z);
            } else {
                in.getline(cbuff, BUFF_SIZE);
            }
            continue;

        case 'f':
            if (normals && texture) {
                in >> p >> ch >> tint >> ch >> tint >> q >> ch >> tint
                    >> ch >> tint >> r >> ch >> tint >> ch >> tint;
            } else if (normals) {
                in >> p >> ch >> ch >> tint >> q >> ch >> tint >> r
                    >> ch >> ch >> tint;
            } else if (texture) {
                in >> p >> ch >> tint >> q >> ch >> tint >> r >> ch >> tint;
            } else {
                in >> p >> q >> r;
            }
            *f = Triangle(p-1, q-1, r-1);
            f++;
            continue;
        }
    }
    in.close();
    return GL_TRUE;
}



/**
 * writeObj
 * Writes the new mesh vertices and triangle faces to the output obj file.
 */
GLvoid
writeObj(GLchar *fname, const vector<Vector3f> &verts,
    const vector<Triangle> &faces)
{
    std::ofstream out;
    vector<Vector3f>::const_iterator v;
    vector<Triangle>::const_iterator f;

    out.open(fname);

    if (out.is_open()) {
        // Write mesh vertices
        for (v = verts.begin(); v != verts.end(); v++)
            out <<"v "<<(*v)(0)<<" "<<(*v)(1)<<" "<<(*v)(2)<<std::endl;

        // Write triangle faces
        for (f = faces.begin(); f != faces.end(); f++)
            out <<"f "<<f->vertex1<<" "<<f->vertex2<<" "<<f->vertex3<<std::endl;
    } else {
        std::cerr << "FILE ERROR: " << strerror(errno) << std::endl;
    }
    out.close();
}

#endif
