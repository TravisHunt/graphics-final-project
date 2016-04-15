#include "view.h"

View::View() {
    init();
}

View::~View() {

}

void View::init() {
    type = PARALLEL;
    hither = 0.001;
    yon = 1000.0;
    imagePlane = hither;
    headTilt = 0.0;

    viewAngle = 45.0;
    aspectRatio = 1.0;

    left = right = top = bottom = 1.0;
}

void View::setEyePos(float x, float y, float z) {
    eye[0] = x; eye[1] = y; eye[2] = z;
}

void View::getEyePos(float *x, float *y, float *z) {
    *x = eye[0]; *y = eye[1]; *z = eye[2];
}

void View::setCOI(float x, float y, float z) {
    coi[0] = x; coi[1] = y; coi[2] = z;
}

void View::getCOI(float *x, float *y, float *z) {
    *x = coi[0]; *y = coi[1]; *z = coi[2];
}
