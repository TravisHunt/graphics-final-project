#include "sketching.h"

#define RADIANS(deg) (deg * (PI / 180))
#define DEGREES(rad) (rad * (180 / PI))

Trackball trackball;
GLint objLoaded = 0;

GLfloat distance(Vector3f &a, Vector3f &b)
{
    GLfloat xx = pow((b.x() - a.x()), 2);
    GLfloat yy = pow((b.y() - a.y()), 2);
    GLfloat zz = pow((b.z() - a.z()), 2);

    return (sqrt(xx + yy + zz));
}

GLint main(GLint argc, char *argv[])
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_SINGLE| GLUT_DEPTH);

    imageWidth  = IMAGE_WIDTH;
    imageHeight = IMAGE_HEIGHT;

    glutInitWindowSize(imageWidth, imageHeight);
    glutCreateWindow("Sketching Interface");
    glutInitWindowPosition((glutGet(GLUT_SCREEN_WIDTH)-imageWidth)/2,
                           (glutGet(GLUT_SCREEN_HEIGHT)-imageWidth)/2);

    init();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouse);
    glutMotionFunc(mouseMotion);
    glutKeyboardFunc(keyboard);

    glutMainLoop();
    return 0;
}


/********** FUNCTION IMPLEMENTATIONS *****************/

void init(void)
{
    glShadeModel(GL_SMOOTH);
    glEnable(GL_DEPTH_TEST);

    tracking  = 0;
    previousX = 0;
    previousY = 0;

    view.setProjection(0.0f, 1000.0f, 0.0f, 1000.0f, -1000.0f, 1000.0f);

    view.setCOI(0.0f, 0.0f, 0.0f); // set coi to origin
    view.setEyePos(0.0f, 0.0f, view.near); // camera position
    view.setRGBA(VIEW_RGBA_2D);

    trackball.set_eye(view.eye);
    trackball.set_focus(view.coi);
    trackball.set_window_size(imageWidth, imageHeight);

    view.light = OFF;
    view.setLightAmbient(0.0f, 0.0f, 0.0f, 1.0f);
    view.setLightDiffuse(0.75f, 0.75f, 0.75f, 0.75f);
    view.setLightSpecular(0.75f, 0.75f, 0.75f, 0.75f);
    view.setLightPosition(0.0f, 100.0f, view.near, 0.0f);
    view.setLightRGBA(1.0f, 1.0f, 1.0f, 1.0f);

    view.setMatAmbient(0.7f, 0.7f, 0.7f, 1.0f);
    view.setMatDiffuse(0.8f, 0.8f, 0.8f, 1.0f);
    view.setMatSpecular(1.0f, 1.0f, 1.0f, 1.0f);
    view.setMatShininess(100.0f);
}

void enableLighting(void)
{
    GLfloat light2[4];
    light2[0] = 0.0f;
    light2[1] = 100.0f;
    light2[2] = view.far;
    light2[3] = 0.0f;

    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_LIGHTING);

    glLightfv(GL_LIGHT0, GL_AMBIENT,  view.LightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  view.LightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, view.LightSpecular);
    glLightfv(GL_LIGHT0, GL_POSITION, view.LightPosition);

    glLightfv(GL_LIGHT1, GL_AMBIENT,  view.LightAmbient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE,  view.LightDiffuse);
    glLightfv(GL_LIGHT1, GL_SPECULAR, view.LightSpecular);
    glLightfv(GL_LIGHT1, GL_POSITION, light2);

    glMaterialfv(GL_FRONT, GL_AMBIENT,   view.MatAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE,   view.MatDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR,  view.MatSpecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, view.MatShininess);
}

void disableLighting(void)
{
    glDisable(GL_LIGHT0);
    glDisable(GL_NORMALIZE);
    glDisable(GL_COLOR_MATERIAL);
    glDisable(GL_LIGHTING);
}

GLint inWindow(GLint x, GLint y)
{
    return (x > 0 && x < imageWidth && y > 0 && y < imageHeight);
}

void wipeCanvas(void)
{
    glClearColor(view.rgba[0], view.rgba[1], view.rgba[2], view.rgba[3]);
    glClear(GL_COLOR_BUFFER_BIT);
    glFlush();
}

void resetStroke(void)
{
    stroke.clear();
    points_on_curve.clear();
    connected.clear();
    go_back_for.clear();
	mesh_verts.clear();
    check_verts.clear();
    mesh_faces.clear();
	display_triangles = 0;
    triangulated = 0;
    tracking  = 0;
    previousX = 0;
    previousY = 0;
    wipeCanvas();
}

/********* INTERPOLATION ***************/
void populateMeshFaces()
{
    if (mesh_circles.size() == 0 || mesh_verts.size() == 0)
        return;

    GLuint i, j;
    Circle c1, c2;
    for (i = 0; i < mesh_circles.size()-1; i++) {
        c1 = mesh_circles[i];
        c2 = mesh_circles[i+1];

        for (j = 0; j < numCirclePts; j++) {
            GLuint p0 = c1.verts[j];
            GLuint p1 = c1.verts[(j+1) % numCirclePts];
            GLuint p2 = c2.verts[j];
            GLuint p3 = c2.verts[(j+1) % numCirclePts];

            Triangle t0(p0, p3, p2);
            Triangle t1(p0, p1, p3);

            mesh_faces.push_back(t0);
            mesh_faces.push_back(t1);
        }
    }

    GLuint first = 0, last = mesh_verts.size()-1;
    c1 = mesh_circles.front();
    c2 = mesh_circles.back();
    for (j = 0; j < numCirclePts; j++) {
        GLuint p0 = c1.verts[j];
        GLuint p1 = c1.verts[(j+1) % numCirclePts];
        GLuint p2 = c2.verts[j];
        GLuint p3 = c2.verts[(j+1) % numCirclePts];

        Triangle t0(first, p1, p0);
        Triangle t1(last , p3, p2);

        mesh_faces.push_back(t0);
        mesh_faces.push_back(t1);
    }
}

void calculateVerticesDriver()
{
    mesh_circles.clear();
    // loop through connected
	for (GLuint index = 0; index < connected.size(); index = index+2) {
        Circle c;
		//loop through the different degree values
		for (GLfloat theta = 0; theta < 360; theta += mesh_rotation){
			calculateVertices(index, ((theta*(PI/180))*(pow(-1, index))));
            c.verts.push_back(mesh_verts.size());
		}
        mesh_circles.push_back(c);
	}

	std::vector<Vector3f>::iterator v;
	v = mesh_verts.begin();
    if (points_on_curve.size()) {
        mesh_verts.insert(v, points_on_curve[0]);
        mesh_verts.push_back(last_in_shape);
    }
}

Vector3f calculateMidpoint(GLint index)
{
    GLfloat x1, y1, x2, y2, z1, z2;
    GLfloat xm, ym, zm;

    x1 = connected[index].p1->x();
    x2 = connected[index].p2->x();
    y1 = connected[index].p1->y();
    y2 = connected[index].p2->y();
    z1 = connected[index].p1->z();
    z2 = connected[index].p2->z();

    xm = (x1 + x2) / 2;
    ym = (y1 + y2) / 2;
    zm = (z1 + z2) / 2;

    return (Vector3f(xm, ym, zm));
}

void calculateVertices(GLint index, GLfloat theta)
{
    GLfloat x, y, z, a, b, c, u, v, w;
    GLfloat newx=0, newy=0, newz=0;
    Vector3f A = *(connected[index].p1);
    Vector3f B = *(connected[index].p2);
    Vector3f midpoint = calculateMidpoint(index);
    Vector3f norm_dir = getNormal(A, B);

    x = A.x();
    y = A.y();
    z = A.z();

    a = midpoint.x();
    b = midpoint.y();
    c = midpoint.z();

    Vector3f point(a, y, z);
    GLfloat opposite = distance(A, point);
    GLfloat hypo     = distance(A, midpoint);
    GLfloat angle    = asin(opposite / hypo);

	GLfloat r = sqrt(pow(B.x() - a, 2) + pow(B.y() - b, 2));

	//calculate circle points
	newx = r * cos(theta);
	newy = r * sin(theta);

	//rotate around x axis
    newz = newx;
    GLfloat oldx = newx;
    newx = 0.0f;

	//rotate around z axis
	newx = newx*cos(angle) - newy*sin(angle);
	newy = newx*sin(angle) + newy*cos(angle);

	// //translate circle to place.
	newx = newx+a;
	newy = newy+b;

    //push directly into the vector to save it.
    mesh_verts.push_back(Vector3f(newx, newy, newz));
}

void populateConnected()
{
    if (points_on_curve.size() == 0) return;
	//sets the original size to iterate over
	go_back_for.push_back(1);
	go_back_for.push_back(points_on_curve.size()-1);

	//iterates over the vector, populates the connected vector
	iterateThrough(go_back_for[0], go_back_for[1]);
}

void iterateThrough(GLint count_forward, GLint count_back)
{
	// counter to find which to increment next
	GLint is_for_or_back = 0;
	// while not on the same index
	while (count_forward != count_back) {
		if (is_for_or_back == 0) {
			is_for_or_back = 1;
			// finds the corresponding points
			GLint ret = checkPoints_CountForward(count_forward, count_back);
			// accounts for if indexes were skipped
			count_forward++;
		} else {
			is_for_or_back = 0;
			// finds corresponding points
			GLint ret = checkPoints_CountBack(count_back, count_forward);
			// accounts for it indees were skipped
			count_back--;
		}
	}
    last_in_shape = *(connected[connected.size()-1].p2);
    connected.pop_back();
}

GLint checkPoints_CountForward(GLint index1, GLint index2)
{
	Vector3f a = points_on_curve[index1 - 1];
	Vector3f b = points_on_curve[index1];
	Vector3f c = points_on_curve[index1 + 1];

	Vector3f ab = getNormal(a, b);
	Vector3f bc = getNormal(b, c);

    connected.push_back(Line(points_on_curve[index1], points_on_curve[index2]));
    recent = 0;
	return 0;
}

GLint checkPoints_CountBack(GLint index1, GLint index2)
{
	Vector3f a = points_on_curve[index1 + 1];
	Vector3f b = points_on_curve[index1];
	Vector3f c = points_on_curve[index1 - 1];

	Vector3f ab = getNormal(a, b);
	Vector3f bc = getNormal(b, c);

    connected.push_back(Line(points_on_curve[index1], points_on_curve[index2]));
    recent = 0;
	return 0;
}

Vector3f getNormal(Vector3f &a, Vector3f &b)
{
    GLint dx = b.x() - a.x();
    GLint dy = b.y() - a.y();

    return Vector3f(-dy, dx, 0.0f);
}

void generateClosingPoints(vector<Vector3f> &points)
{
    GLfloat t, delta, cx, cy;

    // interpolate vertices from Endpoint to Startpoint
    Vector3f a = stroke.back();
    Vector3f b = stroke.front();

    GLfloat length = sideLength(a, b);

    // Linearly interpolate points
    t = 0.0; delta = 1/(length/12);
    for (t = 0.0; t <= 1.0; t += delta) {
        cx = (1.0 - t) * a.x() + t * b.x();
        cy = (a.y() + (b.y() - a.y()) *
            ((cx - a.x()) / (b.x() - a.x())));

        // add interpolated point to stroke
        points.push_back(Vector3f(cx, cy, 0));
    }
}

GLfloat sideLength(Vector3f &a, Vector3f &b)
{
	return sqrt(pow((a.x() - b.x()), 2) + pow((a.y() - b.y()), 2));
}

GLfloat calcAngle(GLfloat BA, GLfloat BC, GLfloat AC)
{
	return acos((pow(BA, 2) + pow(BC, 2) - pow(AC, 2))/(2 * BA * BC));
}

GLint findNextPoint(GLint i, GLint distance)
{
	//base case, if the distance exceeds the bounds
	if (i+distance > stroke.size() && distance % 2 == 0) {
		return findNextPoint(i, distance/2);
	//if the distance does not exceed the bounds but the distance cannot be divided by 2
	} else if (distance % 2 != 0 && i+distance <= stroke.size()) {
		return i+distance;
	//if the distance cannot be divided by 2 and the distance exceeds bounds
	} else if (distance % 2 != 0 && i+distance > stroke.size()) {
		return stroke.size();
	//if everything is okay
	} else {
		//get three points
		Vector3f a = stroke[i];
		Vector3f b = stroke[i+(distance/2)];
		Vector3f c = stroke[i+distance];
		//find distance between each point around triangle
		GLfloat ba = sideLength(b, a);
		GLfloat bc = sideLength(b, c);
		GLfloat ac = sideLength(a, c);

		//if the angle isnt what we want, try again with a shorter distance
		if (calcAngle(ba, bc, ac) > VERTEX_LIMIT) {
			return findNextPoint(i, (distance/2));

		//if it is what we want, return this distance
		} else {
			return i+distance;
		}
	}
}

void getOutsideEdges()
{
    if (stroke.size() == 0) return;
	//takes first point of stroke and puts it in vector
	points_on_curve.push_back(stroke[0]);
	//sets the counter variable
	GLint count = 0;
	//loop through each vertex in stroke
	for (GLuint i = 0; i < stroke.size()-1; i += (count - i)) {
		//find the next point for the curve
		count = findNextPoint(i, DISTANCE_BETWEEN_POINTS);
		//add to the vector
        if (stroke[count].x() != 0 && stroke[count].y() != 0) {
		    points_on_curve.push_back(stroke[count]);
        }
	}
	return;
}

void transition_2D(void)
{
    if (view.type == DRAWING) return;

    /* set new view data */
    view.type = DRAWING;
    view.setRGBA(VIEW_RGBA_2D);

    glutReshapeWindow(imageWidth, imageHeight);
    glutPostRedisplay();
}

void transition_3D(void)
{
    if (view.type == VIEWING) return;

    /* set new view data */
    view.type = VIEWING;
    view.light = ON;
    view.setRGBA(VIEW_RGBA_3D);

    /* launch 3D mesh creation if a stroke is given */
    if (stroke.size()) {
        if (!triangulated) {
            getOutsideEdges();
            populateConnected();
            calculateVerticesDriver();
        }
        populateMeshFaces();
    }

    glutReshapeWindow(imageWidth, imageHeight);
    glutPostRedisplay();
}

GLint test_mesh_pts = 0;
/******** GLUT CALLBACKS **********/
void display(void)
{
    glClearColor(view.rgba[0], view.rgba[1], view.rgba[2], view.rgba[3]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    vector<Vector3f>::const_iterator v;
    vector<Line>::const_iterator l;

    if (view.type == DRAWING) {
        glColor3f(RGBBLACK);
        // reset 3D view trackball matrix
        trackball.setIdentity();

        // Draw user stroke's vertices
        glBegin(GL_LINE_LOOP);
        for (v = stroke.begin(); v != stroke.end(); v++) {
            glVertex2f(v->x(), v->y());
        }
        glEnd();

        // Draw mesh triangles
        if (display_triangles) {
    		glBegin(GL_LINES);
    		for (l = connected.begin(); l != connected.end(); l++) {
    			glVertex2f(l->p1->x(), l->p1->y());
    			glVertex2f(l->p2->x(), l->p2->y());
    		}
    		glEnd();
    		glPointSize(5);
    		glBegin(GL_POINTS);
            for (v = mesh_verts.begin(); v != mesh_verts.end(); v++) {
                glVertex2f(v->x(), v->y());
            }
    		for (v = points_on_curve.begin(); v != points_on_curve.end(); v++) {
    			glVertex2f(v->x(), v->y());
    		}
    		glEnd();
    	}

    } else if (view.type == VIEWING) {
        glColor3f(RGBWHITE);

        GLfloat *m = trackball.get_matrix().data();
        glPushMatrix();
        glMultMatrixf(m);

        if (view.light == ON)
            enableLighting();
        else
            disableLighting();

        if (test_mesh_pts) {
            glPointSize(3);
            glBegin(GL_POINTS);
            for (v = mesh_verts.begin(); v != mesh_verts.end(); v++) {
                glVertex3f(v->x(), v->y(), v->z());
            }
            glEnd();
        }

        Mesh object(mesh_verts, mesh_faces);

        // FIXME: NASTY HACK BECAUSE THE OBJ FILE HAS ORIGIN AT CENTER
        // INSTEAD OF (0,0)
        if (objLoaded) {
            glPushMatrix();
            glTranslatef(imageWidth/2, imageHeight/2, 0.0f);
            object.draw();
            glPopMatrix();
        } else {
            object.draw();
        }

        glPopMatrix();
    }

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    glFlush();
}

void reshape(int w, int h)
{
    GLint deltaW  = (w - imageWidth)  / 2;
    GLint deltaH  = (h - imageHeight) / 2;
    imageWidth  = w;
    imageHeight = h;
    vector<Vector3f>::iterator v;

    // Force window to maintain minimum size
    if (w < IMAGE_WIDTH && h < IMAGE_HEIGHT)
        glutReshapeWindow(IMAGE_WIDTH, IMAGE_HEIGHT);
    else if (w < IMAGE_WIDTH)
        glutReshapeWindow(IMAGE_WIDTH, h);
    else if (h < IMAGE_HEIGHT)
        glutReshapeWindow(w, IMAGE_HEIGHT);

    // Update stroke vertices to keep stroke centered
    for (v = stroke.begin(); v != stroke.end(); v++) {
        v->x() += deltaW;
        v->y() += deltaH;
    }

    // Update bounding box parameters
    view.setProjection(0.0f, imageWidth, 0.0f, imageHeight, view.near, view.far);
    glViewport(0, 0, w, h);

    if (view.type == DRAWING) {
        // Reset view to remove any VIEWING elements
        glMatrixMode (GL_PROJECTION);
        glLoadIdentity();

        // Create orthographic projection
        glOrtho(view.left, view.right, view.bottom, view.top, view.near, view.far);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
    } else if (view.type == VIEWING) {
        // Enable culling of unseen polygons
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glMatrixMode (GL_PROJECTION);

        // Add VIEWING to current orthographic matrix
        gluPerspective(60.0, 1.6, view.near, view.far);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        gluLookAt(view.eye[0], view.eye[1], view.eye[2],
                  view.coi[0], view.coi[1], view.coi[2],
                  0, 1, 0);
    }
}

void mouse(int button, int state, int x, int y)
{
    y = imageHeight - y;

    // In 3D viewing mode, we want the trackball to be active
    if (view.type == VIEWING) {
        long s = 0x00000000;
        s |= (button == GLUT_LEFT_BUTTON)   ? ((state == GLUT_DOWN) ? Trackball::LBUTTON_DOWN : Trackball::LBUTTON_UP) : 0;
        s |= (button == GLUT_MIDDLE_BUTTON) ? ((state == GLUT_DOWN) ? Trackball::MBUTTON_DOWN : Trackball::MBUTTON_UP) : 0;
        s |= (button == GLUT_RIGHT_BUTTON)  ? ((state == GLUT_DOWN) ? Trackball::RBUTTON_DOWN : Trackball::RBUTTON_UP) : 0;

        GLint key_state = glutGetModifiers();
        s |= (key_state & GLUT_ACTIVE_CTRL)  ? Trackball::CTRL_DOWN  : 0;
        s |= (key_state & GLUT_ACTIVE_ALT)   ? Trackball::ALT_DOWN   : 0;
        s |= (key_state & GLUT_ACTIVE_SHIFT) ? Trackball::SHIFT_DOWN : 0;

        if (s & Trackball::BUTTON_DOWN) {
            trackball.mouse_down(s, x, -y);
        }

        if (s & Trackball::BUTTON_UP) {
            trackball.mouse_up(s, x, -y);
        }
    } else if (view.type == DRAWING) {
        if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
            if (inWindow(x, y)) {
                // New stroke
                if (!tracking) {
                    resetStroke();
                    tracking = 1;
                }
                // Update coordinates
                previousX = x;
                previousY = y;
            }
        } else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
            // stroke complete
            tracking = 0;
            previousX = 0;
            previousY = 0;
            // get start & end connecting vertices
            generateClosingPoints(stroke);
            // redraw stroke
            glutPostRedisplay();
        }
    }
}

void mouseMotion(int x, int y)
{
    y = imageHeight - y;

    // Update trackball and exit, because there's no drawing in 3D view.
    if (view.type == VIEWING) {
        trackball.mouse_motion(x, -y);
        glutPostRedisplay();
    }

    // Only record point if the distance between the new point and the last
    // point is >= the minimum distance.
    GLfloat mindist = 0.1;
    GLfloat distance = sqrt(pow(x - previousX, 2) + pow(y - previousY, 2));
    if (distance < mindist) return;

    if (tracking && inWindow(x, y)) {
        // push vertex into vector
        stroke.push_back(Vector3f(x, y, 0));

        glBegin(GL_LINES);
            glVertex2f(previousX, previousY);
            glVertex2f(x, y);
        glEnd();

        // draw line to scene
        glFlush();

        // keep track of previous coordinates
        previousX = x;
        previousY = y;
    }
}

GLboolean loadUserFile(void)
{
    //std::string fname = "";
    GLchar fname[256];

    // Reset everything before loading
    resetStroke();

    std::cout << "Enter file name: ";
    std::cin.getline(fname, 256, '\n');

    // loadObj returns true if the file was successfully loaded,
    // or false otherwise.
    return (loadObj(fname, mesh_verts, mesh_faces));
}

void keyboard(unsigned char key, int x, int y)
{
    switch (key) {
    case 27: // escape key
        exit(0);
        break;
    case 50: // '2' for 2D transition
        transition_2D();
        break;
    case 51: // '3' for 3D transition
        transition_3D();
        break;
    case 76: // 'L' for Load file
        if (!loadUserFile()) {
            printf("ERROR::LOAD::FILE LOAD FAILED\n");
            return;
        }
        objLoaded = 1;
        transition_3D();
        break;
    case 99: // 'c' to clear the stroke
        objLoaded = 0;
        resetStroke();
        break;
    case 108: // 'l' for lighting
        view.light = (view.light == ON) ? OFF : ON;
        glutPostRedisplay();
        break;
    case 116: // 't' toggle triangulation
        display_triangles ^= 1;
        if (!triangulated) {
            triangulated = 1;
            getOutsideEdges();
            populateConnected();
            calculateVerticesDriver();
        }
        glutPostRedisplay();
        break;
    case 118: // 'v' for vertices on mesh
        test_mesh_pts = test_mesh_pts ^ 1;
        glutPostRedisplay();
        break;
    }
}
