/**
 * view.h
 * This file contains the definition of the View class, which is used to
 * Camera, Lighting and Material data.
 */

#ifndef _VIEW_H
#define _VIEW_H

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

enum ProjectionType { DRAWING, VIEWING };
enum Lighting { OFF, ON };

class View
{
public:
    View();
    ~View();

    void init();
    void setEyePos(GLfloat x, GLfloat y, GLfloat z);
    void getEyePos(GLfloat *x, GLfloat *y, GLfloat *z);
    void setCOI(GLfloat x, GLfloat y, GLfloat z);
    void getCOI(GLfloat *x, GLfloat *y, GLfloat *z);
    void setProjection(GLfloat l, GLfloat r, GLfloat b, GLfloat t, GLfloat n, GLfloat f);
    void setRGBA(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
    void getRGBA(GLfloat *r, GLfloat *g, GLfloat *b, GLfloat *a);
    void setLightAmbient(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
    void setLightDiffuse(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
    void setLightSpecular(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
    void setLightPosition(GLfloat x, GLfloat y, GLfloat z, GLfloat q);
    void setLightRGBA(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
    void setMatAmbient(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
    void setMatDiffuse(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
    void setMatSpecular(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
    void setMatShininess(GLfloat s);

    ProjectionType type;

    GLfloat imagePlane;
    GLfloat headTilt;

    // Color values
    GLfloat rgba[4];

    // Lighting
    Lighting light;
    GLfloat LightAmbient[4];
    GLfloat LightDiffuse[4];
    GLfloat LightSpecular[4];
    GLfloat LightPosition[4];
    GLfloat LightRGBA[4];

    GLfloat MatAmbient[4];
    GLfloat MatDiffuse[4];
    GLfloat MatSpecular[4];
    GLfloat MatShininess[1];

    // Viewing Volume Values
    GLfloat right, left, top, bottom, near, far;
    // Perspective Element Values
    GLfloat viewAngle, aspectRatio;

    GLfloat eye[3];
    GLfloat coi[3];
};

#endif
