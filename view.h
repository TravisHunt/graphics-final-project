#ifndef _VIEW_H
#define _VIEW_H

enum ProjectionType { PARALLEL, PERSPECTIVE };
enum Lighting { OFF, ON };

class View {
public:
    View();
    ~View();

    void init();
    void setEyePos(float x, float y, float z);
    void getEyePos(float *x, float *y, float *z);
    void setCOI(float x, float y, float z);
    void getCOI(float *x, float *y, float *z);
    void setProjection(float l, float r, float b, float t, float n, float f);
    void setRGBA(float r, float g, float b, float a);
    void getRGBA(float *r, float *g, float *b, float *a);
    void setLightAmbient(float r, float g, float b, float a);
    void setLightDiffuse(float r, float g, float b, float a);
    void setLightSpecular(float r, float g, float b, float a);
    void setLightPosition(float x, float y, float z, float q);
    void setLightRGBA(float r, float g, float b, float a);
    void setMatAmbient(float r, float g, float b, float a);
    void setMatDiffuse(float r, float g, float b, float a);
    void setMatSpecular(float r, float g, float b, float a);
    void setMatShininess(float s);

    ProjectionType type;

    float imagePlane;
    float headTilt;

    // Color values
    float rgba[4];

    // Lighting
    Lighting light;
    float LightAmbient[4];
    float LightDiffuse[4];
    float LightSpecular[4];
    float LightPosition[4];
    float LightRGBA[4];

    float MatAmbient[4];
    float MatDiffuse[4];
    float MatSpecular[4];
    float MatShininess[1];

    // Projection values
    float right, left, top, bottom, near, far;
    // Perspective values
    float viewAngle, aspectRatio;


    float eye[3];
    float coi[3];
};

#endif
