#include "trackball.h"

static const GLdouble PI = 3.141592653589793238462643383279502884197;

#define todegree(x) ((x) * 180.0 / PI)
#define toradian(x) ((x) * PI / 180.0)

#define DEFAULT_ROTATE_SCALE 0.2f
#define DEFAULT_ZOOM_SCALE   0.002f
#define DEFAULT_DOLLY_SCALE  0.5f
#define DEFAULT_PAN_SCALE    0.1f

#define pow2(x) (x*x)

/*  This function is based on Erich Boleyn (erich@uruk.org)
 *
 *  Arbitrary axis rotation matrix.
 *
 *  This is composed of 5 matrices, Rz, Ry, T, Ry', Rz', multiplied
 *  like so:  Rz * Ry * T * Ry' * Rz'.  T is the final rotation
 *  (which is about the X-axis), and the two composite transforms
 *  Ry' * Rz' and Rz * Ry are (respectively) the rotations necessary
 *  from the arbitrary axis to the X-axis then back.  They are
 *  all elementary rotations.
 */

void rotate_matrix(GLfloat theta, GLfloat x, GLfloat y, GLfloat z, Eigen::Matrix4f &m)
{
    GLfloat xx, yy, zz, xy, yz, zx, xs, ys, zs, one_c;
    GLfloat len = (GLfloat) sqrt(pow2(x) + pow2(x) + pow2(x));

    /* if m length is zero, set m to identity and return */
    if (len == 0.0) {
        m.setIdentity();
        return;
    }

    /* normalize coordinate data */
    x /= len;
    y /= len;
    z /= len;

    GLfloat s = (GLfloat) sin(toradian(theta));
    GLfloat c = (GLfloat) cos(toradian(theta));

    xx = x * x;  yy = y * y;  zz = z * z;
    xy = x * y;  yz = y * z;  zx = z * x;
    xs = x * s;  ys = y * s;  zs = z * s;
    one_c = 1.0f - c;

    /* apply rotation */
    m(0,0) = (one_c * xx) + c;  m(0,1) = (one_c * xy) - zs;  m(0,2) = (one_c * zx) + ys;  m(0,3) = 0.0f;
    m(1,0) = (one_c * xy) + zs; m(1,1) = (one_c * yy) + c;   m(1,2) = (one_c * yz) - xs;  m(1,3) = 0.0f;
	m(2,0) = (one_c * zx) - ys; m(2,1) = (one_c * yz) + xs;  m(2,2) = (one_c * zz) + c;   m(2,3) = 0.0f;
	m(3,0) = 0.0f;              m(3,1) = 0.0f;               m(3,2) = 0.0f;               m(3,3) = 1.0f;
}

/* Default constructor */
Trackball::Trackball() :
    behavior(ALL),
    action(_NONE),
    rotate_scale(DEFAULT_ROTATE_SCALE),
    zoom_scale(DEFAULT_ZOOM_SCALE),
    pan_scale(DEFAULT_PAN_SCALE),
    dolly_scale(DEFAULT_DOLLY_SCALE),
    window_width(800),
    window_height(500)
{
    /* default attach mode */
    attach(PAN,    LBUTTON_DOWN | SHIFT_DOWN);
    attach(ZOOM,   LBUTTON_DOWN | CTRL_DOWN);
    attach(DOLLY,  RBUTTON_DOWN);
    attach(ROTATE, LBUTTON_DOWN);
}

void Trackball::reset()
{
    mouse_mat.setIdentity();
    action = _NONE;
}

void Trackball::attach(GLint behavior, GLint mouse_key_state)
{
    if (behavior == PAN)
        action_table[_PAN] = mouse_key_state;
    else if (behavior == DOLLY)
        action_table[_DOLLY] = mouse_key_state;
    else if (behavior == ROTATE)
        action_table[_ROTATE] = mouse_key_state;
    else if (behavior == ZOOM)
        action_table[_ZOOM] = mouse_key_state;
}

void Trackball::mouse_down(GLint state, GLint x, GLint y)
{
    start_x = x;
    start_y = y;
    start = trackball_mapping(x, y);

    if (behavior & ROTATE && action_table[_ROTATE] == state)
        action = _ROTATE;
    else if (behavior & PAN && action_table[_PAN] == state)
        action = _PAN;
    else if (behavior & DOLLY && action_table[_DOLLY] == state)
        action = _DOLLY;
    else if (behavior & ZOOM && action_table[_ZOOM] == state)
        action = _ZOOM;
    else
        action = _NONE;
}

void Trackball::mouse_up(GLint state, GLint x, GLint y)
{
    if (state & BUTTON_UP)
        action = _NONE;
}

void Trackball::mouse_motion(GLint x, GLint y)
{
    switch(action) {
    case _ROTATE:
    {
        /* Rotate around the axis that is perpendicular to and centers at the great circle connecting
		 * the two mouse positions. We use the two points on the great circle to calculate the axis.
		 * Then project the axis on x-y plane as the rotation axis. */

         /* calculate axis for the great circle */
         Vector3f end  = trackball_mapping(x, y);
         Vector3f axis = start.cross(end);

         /* project the axis on x-y plane */
         axis[2] = 0.0f;

         GLfloat span = (GLfloat) sqrt(pow2(x-start_x) + pow2(y-start_y));
         GLfloat rot_angle = span * rotate_scale;
         Matrix4f m;
         rotate_matrix(rot_angle, axis[0], axis[1], axis[2], m);
         Matrix4f rotate(m);
         mouse_mat = focus2world * rotate * world2focus * mouse_mat;
         start = end;
         break;
    }
    case _ZOOM:
    {
        GLfloat zoom = 1.0f + (start_y - y) * zoom_scale;
        if (zoom < 0.0001f) zoom = 0.0001f;
        Matrix4f zoom_mat;
        zoom_mat.setIdentity();
        zoom_mat(0,0) = zoom;
        zoom_mat(1,1) = zoom;
        zoom_mat(2,2) = zoom;
        mouse_mat = focus2world * zoom_mat * world2focus * mouse_mat;
        break;
    }
    case _PAN:
    {
        GLfloat pan_x = (x - start_x) * pan_scale;
        GLfloat pan_y = (start_y - y) * pan_scale;
        Matrix4f pan_mat;
        pan_mat.setIdentity();
        pan_mat(0,3) = pan_x;
        pan_mat(1,3) = pan_y;
        mouse_mat = pan_mat * mouse_mat;

        world2focus(0,3) -= pan_x;
        world2focus(1,3) -= pan_y;

        focus2world(0,3) += pan_x;
        focus2world(1,3) += pan_y;
        break;
    }
    case _DOLLY:
    {
        GLfloat dolly_z = (start_y - y) * dolly_scale;
        Matrix4f dolly_mat;
        dolly_mat.setIdentity();
        dolly_mat(2,3) = dolly_z;
        mouse_mat = dolly_mat * mouse_mat;

        world2focus(2,3) -= dolly_z;
        focus2world(2,3) += dolly_z;
        break;
    }
    default:
        break;
    }

    start_x = x;
    start_y = y;
}

Vector3f Trackball::trackball_mapping(GLint x, GLint y)
{
    Eigen::Vector3f v;
    GLfloat d;

    v << (2.0f * x - window_width)  / window_width,
         (window_height - 2.0f * y) / window_height,
         0.0f;

    d = (GLfloat) sqrt(pow2(v(0)) + pow2(v(1)) + pow2(v(2)));
    d = (d < 1.0f) ? d : 1.0f;
    v[2] = (GLfloat) sqrt(1.01f - d*d);
    v = v.normalized();
    return v;
}

void Trackball::set_eye(const GLfloat *e)
{
    eye[0] = e[0];
    eye[1] = e[1];
    eye[2] = e[2];
}

void Trackball::set_focus(const GLfloat *f)
{
    focus[0] = f[0];
    focus[1] = f[1];
    focus[2] = f[2];

    world2focus(0,3) = -focus(0);
    world2focus(1,3) = -focus(1);
    world2focus(2,3) = -focus(2);

    focus2world(0,3) =  focus(0);
    focus2world(1,3) =  focus(1);
    focus2world(2,3) =  focus(2);
}
