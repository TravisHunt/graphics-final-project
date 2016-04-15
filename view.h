#ifndef _VIEW_H
#define _VIEW_H

enum ProjectionType { PARALLEL, PERSPECTIVE };

class View {
public:
    View();
    ~View();

    void init();
    void setEyePos(float x, float y, float z);
    void getEyePos(float *x, float *y, float *z);
    void setCOI(float x, float y, float z);
    void getCOI(float *x, float *y, float *z);

    ProjectionType type;
    float hither;
    float yon;
    float imagePlane;
    float headTilt;

    // Orthographic values
    float right, left, top, bottom;
    // Perspective values
    float viewAngle, aspectRatio;

private:
    float eye[3];
    float coi[3];
};

#endif
