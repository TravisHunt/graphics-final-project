#include "view.h"

View::View() {
    init();
}

View::~View() {

}

void View::init() {
    type = DRAWING;
    near = 0.001;
    far  = 1000.0;
    headTilt = 0.0;

    viewAngle = 45.0;
    aspectRatio = 1.6;

    left = right = top = bottom = 1.0;

    setEyePos(1,1,1);
    setCOI(0,0,0);
    setRGBA(0.0f, 0.0f, 0.0f, 0.0f);

    // Set lighting
    setLightAmbient(0.5f, 0.5f, 0.5f, 1.0f);
    setLightDiffuse(0.5f, 0.5f, 0.5f, 1.0f);
    setLightSpecular(1.0f, 1.0f, 1.0f, 1.0f);
    setLightPosition(5.0f, 25.0f, 15.0f, 1.0f);
    setLightRGBA(1.0f, 1.0f, 1.0f, 1.0f);
    // Set material lighting
    setMatAmbient(0.7f, 0.7f, 0.7f, 1.0f);
    setMatDiffuse(0.8f, 0.8f, 0.8f, 1.0f);
    setMatSpecular(1.0f, 1.0f, 1.0f, 1.0f);
    setMatShininess(100.0f);
}

void View::setEyePos(GLfloat x, GLfloat y, GLfloat z) {
    eye[0] = x; eye[1] = y; eye[2] = z;
}

void View::getEyePos(GLfloat *x, GLfloat *y, GLfloat *z) {
    *x = eye[0]; *y = eye[1]; *z = eye[2];
}

void View::setCOI(GLfloat x, GLfloat y, GLfloat z) {
    coi[0] = x; coi[1] = y; coi[2] = z;
}

void View::getCOI(GLfloat *x, GLfloat *y, GLfloat *z) {
    *x = coi[0]; *y = coi[1]; *z = coi[2];
}

void View::setProjection(GLfloat l, GLfloat r, GLfloat b, GLfloat t, GLfloat n, GLfloat f) {
    left = l; right = r; bottom = b; top = t; near = n; far = f;
}

void View::setRGBA(GLfloat r, GLfloat g, GLfloat b, GLfloat a) {
    rgba[0] = r; rgba[1] = g; rgba[2] = b; rgba[3] = a;
}

void View::getRGBA(GLfloat *r, GLfloat *g, GLfloat *b, GLfloat *a) {
    *r = rgba[0]; *g = rgba[1]; *b = rgba[2]; *a = rgba[3];
}

void View::setLightAmbient(GLfloat r, GLfloat g, GLfloat b, GLfloat a) {
    LightAmbient[0] = r; LightAmbient[1] = g;
    LightAmbient[2] = b; LightAmbient[3] = a;
}

void View::setLightDiffuse(GLfloat r, GLfloat g, GLfloat b, GLfloat a) {
    LightDiffuse[0] = r; LightDiffuse[1] = g;
    LightDiffuse[2] = b; LightDiffuse[3] = a;
}

void View::setLightSpecular(GLfloat r, GLfloat g, GLfloat b, GLfloat a) {
    LightSpecular[0] = r; LightSpecular[1] = g;
    LightSpecular[2] = b; LightSpecular[3] = a;
}

void View::setLightPosition(GLfloat x, GLfloat y, GLfloat z, GLfloat q) {
    LightPosition[0] = x; LightPosition[1] = y;
    LightPosition[2] = z; LightPosition[3] = q;
}

void View::setLightRGBA(GLfloat r, GLfloat g, GLfloat b, GLfloat a) {
    LightRGBA[0] = r; LightRGBA[1] = g;
    LightRGBA[2] = b; LightRGBA[3] = a;
}

void View::setMatAmbient(GLfloat r, GLfloat g, GLfloat b, GLfloat a) {
    MatAmbient[0] = r; MatAmbient[1] = g;
    MatAmbient[2] = b; MatAmbient[3] = a;
}

void View::setMatDiffuse(GLfloat r, GLfloat g, GLfloat b, GLfloat a) {
    MatDiffuse[0] = r; MatDiffuse[1] = g;
    MatDiffuse[2] = b; MatDiffuse[3] = a;
}

void View::setMatSpecular(GLfloat r, GLfloat g, GLfloat b, GLfloat a) {
    MatSpecular[0] = r; MatSpecular[1] = g;
    MatSpecular[2] = b; MatSpecular[3] = a;
}

void View::setMatShininess(GLfloat s) {
    MatShininess[0] = s;
}
