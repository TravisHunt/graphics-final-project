/**
 * trackball.h
 * This file contains the definition of the Trackball class, which is used
 * to calculate the rotation, pan, dolly and zoom of the mesh model.
 *
 * This trackball implementation is a modified version of Prof. Jian Chen's
 * ivTrackball class.
 */
#ifndef _TRACKBALL_H_
#define _TRACKBALL_H_

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include <iostream>
#include <cmath>
#include <Eigen/Dense>

using namespace Eigen;

class Trackball
{
public:
    /* Constructors */
    Trackball();

    virtual void mouse_down(GLint state, GLint x, GLint y);
    virtual void mouse_up(GLint state, GLint x, GLint y);
    virtual void mouse_motion(GLint x, GLint y);

    void attach(GLint behavior, GLint mouse_key_state);
    void reset();
    void set_eye(const GLfloat *e);
    void set_focus(const GLfloat *f);

    GLfloat get_behavior() const
        { return behavior; }

    void set_behavior(GLfloat flag)
        { behavior = flag; }

    Matrix4f get_matrix()
        { return mouse_mat; }

    void set_matrix(const Matrix4f &m)
        { mouse_mat = m; }

    void setIdentity(void)
        { mouse_mat.setIdentity(); }

    void set_window_size(GLint w, GLint h)
        { window_width = w; window_height = h; }

    void set_rotate_scale(GLfloat s)
        { rotate_scale = s; }

    void set_zoom_scale(GLfloat s)
        { zoom_scale = s; }

    void set_pan_scale(GLfloat s)
        { pan_scale = s; }

    void set_dolly_scale(GLfloat s)
        { dolly_scale = s; }

    void print()
        { std::cout << mouse_mat << std::endl; }

    enum {
        SILENT         = 0x00000000,
		LBUTTON_DOWN   = 0x00000001,
		MBUTTON_DOWN   = 0x00000002,
		RBUTTON_DOWN   = 0x00000004,
        BUTTON_DOWN    = 0x00000007,
		LBUTTON_UP     = 0x00000010,
		MBUTTON_UP     = 0x00000020,
		RBUTTON_UP     = 0x00000040,
        BUTTON_UP      = 0x00000070,
		ALT_DOWN       = 0x00000100,
		CTRL_DOWN	   = 0x00000200,
		SHIFT_DOWN     = 0x00000400,
        KEY_DOWN       = 0x00000700,
    } State;

    typedef enum {
        PAN         = 0x00000001,
		DOLLY       = 0x00000002,
		ROTATE      = 0x00000004,
		ZOOM        = 0x00000008,
		ALL         = 0x0000000F,
    } Behavior;

private:
    Vector3f trackball_mapping(GLint x, GLint y);

protected:
    typedef enum {
        _NONE   = 0,
        _PAN    = 1,
        _ZOOM   = 2,
		_DOLLY  = 3,
        _ROTATE = 4,
    } Action;

    GLint behavior;                // the trackball behavior type
    Action action;                 // the active action
	Vector3f eye;                  // eye pos in world
	Vector3f focus;                // objec rotation center
    Matrix4f mouse_mat;            // the mouse operation matrix
	Matrix4f world2focus;          // the translate from world to focus
	Matrix4f focus2world;          // the translate from focus to world
    Vector3f start;                // the mouse start pos on the great cicle
	GLint start_x, start_y;        // start point in screen space

    GLfloat rotate_scale;
    GLfloat dolly_scale;
    GLfloat zoom_scale;
    GLfloat pan_scale;

    GLint window_width;
    GLint window_height;

    GLint action_table[5];  // mouse/key action to action mode table.
};

#endif
