#include "sketching.h"

#define RADIANS(deg) (deg * (PI / 180))
#define DEGREES(rad) (rad * (180 / PI))

Trackball trackball;
Mesh *test_mesh;

float distance(Vector3f &a, Vector3f &b)
{
    float xx = pow((b.x() - a.x()), 2);
    float yy = pow((b.y() - a.y()), 2);
    float zz = pow((b.z() - a.z()), 2);

    return (sqrt(xx + yy + zz));
}

int main(int argc, char *argv[]) {
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

void init(void) {
    glShadeModel(GL_SMOOTH);
    glEnable(GL_DEPTH_TEST);

    vector<Vector3f> verts;
    vector<Triangle> faces;

    // FIXME: REMOVE THIS
    GLchar fname[] = "ogre.obj";
    //GLchar fname[] = "dodecahedron.obj";
    loadObj(fname, verts, faces);
    test_mesh = new Mesh(verts, faces);

    tracking  = 0;
    previousX = 0;
    previousY = 0;

    //view.setProjection(0.0f, imageWidth, 0.0f, imageHeight, -500.0f, 500.0f);
    view.setProjection(0.0f, 1000.0f, 0.0f, 1000.0f, -1000.0f, 1000.0f);

    view.setCOI(0.0f, 0.0f, 0.0f); // set coi to origin
    view.setEyePos(0.0f, 0.0f, view.near); // camera position
    view.setRGBA(VIEW_RGBA_2D);

    trackball.set_eye(view.eye);
    trackball.set_focus(view.coi);
    trackball.set_window_size(imageWidth, imageHeight);

    view.light = OFF;
    view.setLightAmbient(0.0f, 0.0f, 0.0f, 1.0f);
    view.setLightDiffuse(1.0f, 1.0f, 1.0f, 1.0f);
    view.setLightSpecular(1.0f, 1.0f, 1.0f, 1.0f);
    //view.setLightPosition(imageWidth/2, imageHeight, 0.0f, 0.0f);
    view.setLightPosition(0.0f, view.top, view.far, 0.0f);
    view.setLightRGBA(1.0f, 1.0f, 1.0f, 1.0f);

    view.setMatAmbient(0.7f, 0.7f, 0.7f, 1.0f);
    view.setMatDiffuse(0.8f, 0.8f, 0.8f, 1.0f);
    view.setMatSpecular(1.0f, 1.0f, 1.0f, 1.0f);
    view.setMatShininess(100.0f);

    createGLUTMenus();
}

void enableLighting(void) {
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_LIGHTING);

    glLightfv(GL_LIGHT0, GL_AMBIENT,  view.LightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  view.LightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, view.LightSpecular);
    glLightfv(GL_LIGHT0, GL_POSITION, view.LightPosition);

    glMaterialfv(GL_FRONT, GL_AMBIENT,   view.MatAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE,   view.MatDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR,  view.MatSpecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, view.MatShininess);
}

void disableLighting(void) {
    glDisable(GL_LIGHT0);
    glDisable(GL_NORMALIZE);
    glDisable(GL_COLOR_MATERIAL);
    glDisable(GL_LIGHTING);
}

int inWindow(int x, int y) {
    return (x > 0 && x < imageWidth && y > 0 && y < imageHeight);
}

void wipeCanvas(void) {
    glClearColor(view.rgba[0], view.rgba[1], view.rgba[2], view.rgba[3]);
    glClear(GL_COLOR_BUFFER_BIT);
    glFlush();
}

void resetStroke(void) {
    stroke.clear();
    points_on_curve.clear();
    connected.clear();
    go_back_for.clear();
	vertices_on_shape.clear();
    check_verts.clear();
    mesh_verts.clear();
	display_triangles = 0;
    tracking  = 0;
    previousX = 0;
    previousY = 0;
    wipeCanvas();
}

/********* INTERPOLATION ***************/
void populateMeshVerts(){
    if (mesh_circles.size() == 0 || vertices_on_shape.size() == 0)
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

            mesh_verts.push_back(t0);
            mesh_verts.push_back(t1);
        }
    }

    GLuint first = 0, last = vertices_on_shape.size()-1;
    c1 = mesh_circles.front();
    c2 = mesh_circles.back();
    for (j = 0; j < numCirclePts; j++) {
        GLuint p0 = c1.verts[j];
        GLuint p1 = c1.verts[(j+1) % numCirclePts];
        GLuint p2 = c2.verts[j];
        GLuint p3 = c2.verts[(j+1) % numCirclePts];

        Triangle t0(first, p1, p0);
        Triangle t1(last , p3, p2);

        mesh_verts.push_back(t0);
        mesh_verts.push_back(t1);
    }
}

void calculateVerticesDriver(){
    mesh_circles.clear();
    // loop through connected
	for (int index = 0; index < connected.size(); index = index+2) {
        Circle c;
		//loop through the different degree values
		for (float theta = 0; theta < 360; theta += mesh_rotation){
			calculateVertices(index, ((theta*(PI/180))*(pow(-1, index))));
            c.verts.push_back(vertices_on_shape.size());
		}
        mesh_circles.push_back(c);
	}

	std::vector<Vector3f>::iterator v;
	v = vertices_on_shape.begin();
    if (points_on_curve.size()) {
        vertices_on_shape.insert(v, points_on_curve[0]);
        vertices_on_shape.push_back(last_in_shape);
    }
}

Vector3f calculateMidpoint(int index){
    float x1, y1, x2, y2, z1, z2;
    float xm, ym, zm;

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

void calculateVertices(int index, float theta) {
    float x, y, z, a, b, c, u, v, w;
    float newx=0, newy=0, newz=0;
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
    float opposite = distance(A, point);
    float hypo     = distance(A, midpoint);
    float angle    = asin(opposite / hypo);

	float r = sqrt(pow(B.x() - a, 2) + pow(B.y() - b, 2));

	//calculate circle points
	newx = r * cos(theta);
	newy = r * sin(theta);

	//rotate around x axis
    newz = newx;
    float oldx = newx;
    newx = 0.0f;

	//rotate around z axis
	newx = newx*cos(angle) - newy*sin(angle);
	newy = newx*sin(angle) + newy*cos(angle);

	// //translate circle to place.
	newx = newx+a;
	newy = newy+b;

    //push directly into the vector to save it.
    vertices_on_shape.push_back(Vector3f(newx, newy, newz));
}



/* commented out incomplete code */
void populateConnected(){
    if (points_on_curve.size() == 0) return;
	//sets the original size to iterate over
	go_back_for.push_back(1);
	go_back_for.push_back(points_on_curve.size()-1);

	//iterates over the vector, populates the connected vector
	/* while (!go_back_for.empty()){ */
		iterateThrough(go_back_for[0], go_back_for[1]);
	/* } */

}

/* commented out incomplete code */
void iterateThrough(int count_forward, int count_back){
	//counter to find which to increment next
	int is_for_or_back = 0;
	//while not on the same index
	while(count_forward != count_back){
		if(is_for_or_back == 0){
			is_for_or_back = 1;
			//finds the corresponding points
			int ret = checkPoints_CountForward(count_forward, count_back);
			//accounts for if indexes were skipped
/* 			if (ret != 0){
				recent = 0;
				go_back_for.push_back(count_forward);
				connected.pop_back();
				drawNewLine(count_forward, ret);
				go_back_for.push_back(count_forward + ret);
				count_forward++;
			}else{ */
				count_forward++;
				/* } */
		}else{
			is_for_or_back = 0;
			//finds corresponding points
			int ret = checkPoints_CountBack(count_back, count_forward);
			//accounts for it indees were skipped
/* 			if (ret != 0){
				recent = 0;
				go_back_for.push_back(count_back);
				connected.pop_back();
				drawNewLine(count_back, -ret);
				go_back_for.push_back(count_back - ret);
				count_back --;
			}else{ */
				count_back--;
			/*  } */
		}
	}
    last_in_shape = *(connected[connected.size()-1].p2);
    connected.pop_back();
}

/* commented out incomplete code */
int checkPoints_CountForward(int index1, int index2){
	Vector3f a = points_on_curve[index1 - 1];
	Vector3f b = points_on_curve[index1];
	Vector3f c = points_on_curve[index1 + 1];

	Vector3f ab = getNormal(a, b);
	Vector3f bc = getNormal(b, c);
	//checks to see if the points are a match. if they are not, the function recursus
/* 	if (isClose(points_on_curve[index1], ab, points_on_curve[index2]) || isClose(points_on_curve[index1], bc, points_on_curve[index2])){
		if (recent == 2){
			recent = 1;
			return (1 + checkPoints_CountForward(index1 + 1, index2));
		}else{ */
			//connected.push_back(std::make_pair(points_on_curve[index1], points_on_curve[index2]));
            connected.push_back(Line(points_on_curve[index1], points_on_curve[index2]));
            recent = 0;
			return 0;
/* 		}
	}else{
		recent = 2;
		return (1 + checkPoints_CountForward(index1 + 1, index2));
	} */
}

/* commented out incomplete code */
int checkPoints_CountBack(int index1, int index2){
	Vector3f a = points_on_curve[index1 + 1];
	Vector3f b = points_on_curve[index1];
	Vector3f c = points_on_curve[index1 - 1];

	Vector3f ab = getNormal(a, b);
	Vector3f bc = getNormal(b, c);
	//checks to see if the points are a match. if they are not, the function recursus
/* 	if (isClose(points_on_curve[index1], ab, points_on_curve[index2]) || isClose(points_on_curve[index1], bc, points_on_curve[index2])){
		if (recent == 2){
			recent = 1;
			return (1 + checkPoints_CountBack(index1 - 1, index2));
		}else{ */
			//connected.push_back(std::make_pair(points_on_curve[index1], points_on_curve[index2]));
            connected.push_back(Line(points_on_curve[index1], points_on_curve[index2]));
            recent = 0;
			return 0;
/* 		}
	}else{
		recent = 2;
		return (1 + checkPoints_CountBack(index1 - 1, index2));
	} */
}

bool isClose(Vector3f &home, Vector3f &normal, Vector3f &point_to_check) {

	bool close = false;

	float slope, intercept;
    float x1, y1, x2, y2;
    float dx, dy;

	x1 = home.x();
	y1 = home.y();

	x2 = normal.x();
	y2 = normal.y();

	dx = x2 - x1;
    dy = y2 - y1;

    slope = dy / dx;
    // y = mx + c
    // intercept c = y - mx
    intercept = y1 - slope * x1; // which is same as y2 - slope * x2

	float distance = (std::abs(slope*point_to_check.x() - point_to_check.y() +intercept) / sqrt(pow(slope, 2) -1));

	if (distance > DISTANCE_CONSTANT){
		return false;
	}else{
		return true;
	}
}

Vector3f getNormal(Vector3f &a, Vector3f &b)
{
    int dx = b.x() - a.x();
    int dy = b.y() - a.y();

    return Vector3f(-dy, dx, 0.0f);
}

void generateClosingPoints(vector<Vector3f> &points)
{
    GLfloat t, delta, cx, cy;

    // interpolate vertices from Endpoint to Startpoint
    Vector3f a = stroke.back();
    Vector3f b = stroke.front();

    float length = sideLength(a, b);

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

float sideLength(Vector3f &a, Vector3f &b) {
	return sqrt(pow((a.x() - b.x()), 2) + pow((a.y() - b.y()), 2));
}

float calcAngle(float BA, float BC, float AC) {
	return acos((pow(BA, 2) + pow(BC, 2) - pow(AC, 2))/(2 * BA * BC));
}

int findNextPoint(int i, int distance)
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
		float ba = sideLength(b, a);
		float bc = sideLength(b, c);
		float ac = sideLength(a, c);

		//if the angle isnt what we want, try again with a shorter distance
		if (calcAngle(ba, bc, ac) > VERTEX_LIMIT) {
			return findNextPoint(i, (distance/2));

		//if it is what we want, return this distance
		} else {
			return i+distance;
		}
	}
}

void getOutsideEdges() {
    if (stroke.size() == 0) return;
	//takes first point of stroke and puts it in vector
	points_on_curve.push_back(stroke[0]);
	//sets the counter variable
	int count = 0;
	//loop through each vertex in stroke
	for (int i=0; i < stroke.size()-1; i = i+(count-i)) {
		//find the next point for the curve
		count = findNextPoint(i, DISTANCE_BETWEEN_POINTS);
		//add to the vector
        if (stroke[count].x() != 0 && stroke[count].y() != 0) {
		    points_on_curve.push_back(stroke[count]);
        }
	}
	return;
}

void transition_2D(void) {
    if (view.type == PARALLEL) return;

    /* set new view data */
    view.type = PARALLEL;
    view.setRGBA(VIEW_RGBA_2D);

    glutReshapeWindow(imageWidth, imageHeight);
    glutPostRedisplay();
}

void transition_3D(void) {
    if (view.type == PERSPECTIVE) return;

    /* set new view data */
    view.type = PERSPECTIVE;
    view.light = ON;
    view.setRGBA(VIEW_RGBA_3D);

    /* launch 3D mesh creation */
    getOutsideEdges();
    populateConnected();
    calculateVerticesDriver();
    populateMeshVerts();

    glutReshapeWindow(imageWidth, imageHeight);
    glutPostRedisplay();
}

/************ MENU FUNCTIONS *****************/
void createGLUTMenus(void) {
    menu_2Dview = glutCreateMenu(handleMenuEvents);

    glutAddMenuEntry("Clear Canvas", CLEAR);
    glutAddMenuEntry("Switch to 2D View", SWITCH_2D);
    glutAddMenuEntry("Switch to 3D View", SWITCH_3D);
    glutAddMenuEntry("Show 2D Triangle Mesh", TRIANGULATE_2D);

    glutAttachMenu(GLUT_RIGHT_BUTTON);
}

void handleMenuEvents(int option) {
    unsigned int i, limit;

    switch(option) {
        case CLEAR:
            resetStroke();
            break;
        case SWITCH_2D:
            transition_2D();
            break;
        case SWITCH_3D:
            transition_3D();
            break;
        case TRIANGULATE_2D:
            display_triangles = 1;
            getOutsideEdges();
            populateConnected();
			calculateVerticesDriver();
            glutPostRedisplay();
            break;
        default:
            break;
    }
}

void destroyGLUTMenus(void) {
    glutDestroyMenu(menu_2Dview);
}

int tmesh = 0;
int test_mesh_pts = 0;
/******** GLUT CALLBACKS **********/
void display(void)
{
    glClearColor(view.rgba[0], view.rgba[1], view.rgba[2], view.rgba[3]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    if (view.type == PARALLEL) {
        glColor3f(RGBBLACK);
        // Draw user stroke's vertices
        glBegin(GL_LINE_LOOP);
        for (it = stroke.begin(); it != stroke.end(); it++) {
            glVertex2f(it->x(), it->y());
        }
        glEnd();

        // Draw mesh triangles
        if (display_triangles) {
    		glBegin(GL_LINES);
    		for (it2 = connected.begin(); it2 != connected.end(); it2++) {
    			glVertex2f(it2->p1->x(), it2->p1->y());
    			glVertex2f(it2->p2->x(), it2->p2->y());
    		}
    		glEnd();
    		glPointSize(5);
    		glBegin(GL_POINTS);
            vector<Vector3f>::const_iterator v;
            for (v = vertices_on_shape.begin(); v != vertices_on_shape.end(); v++) {
                glVertex2f(v->x(), v->y());
            }
    		for (it = points_on_curve.begin(); it != points_on_curve.end(); it++){
    			glVertex2f(it->x(), it->y());
    		}
    		glEnd();
    	}

    } else if (view.type == PERSPECTIVE) {
        glColor3f(RGBWHITE);

        GLfloat *m = trackball.get_matrix().data();
        glPushMatrix();
        glMultMatrixf(m);

        if (view.light == ON)
            enableLighting();
        else
            disableLighting();

        if (tmesh) {
            glPushMatrix();
            glTranslatef(imageWidth/2, imageHeight/2, 0.0f);
            test_mesh->draw();
            glPopMatrix();
        }

        if (test_mesh_pts) {
            glPointSize(3);
            glBegin(GL_POINTS);
            vector<Vector3f>::const_iterator v;
            for (v = vertices_on_shape.begin(); v != vertices_on_shape.end(); v++) {
                glVertex3f(v->x(), v->y(), v->z());
            }
            glEnd();
        }

        Mesh object(vertices_on_shape, mesh_verts);
        object.draw();

        glPopMatrix();
    }

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    glFlush();
}

void reshape(int w, int h) {
    int deltaW  = (w - imageWidth)  / 2;
    int deltaH  = (h - imageHeight) / 2;
    imageWidth  = w;
    imageHeight = h;

    // Force window to maintain minimum size
    if (w < IMAGE_WIDTH && h < IMAGE_HEIGHT)
        glutReshapeWindow(IMAGE_WIDTH, IMAGE_HEIGHT);
    else if (w < IMAGE_WIDTH)
        glutReshapeWindow(IMAGE_WIDTH, h);
    else if (h < IMAGE_HEIGHT)
        glutReshapeWindow(w, IMAGE_HEIGHT);

    // Update stroke vertices to keep stroke centered
    for (it = stroke.begin(); it != stroke.end(); it++) {
        it->x() += deltaW;
        it->y() += deltaH;
    }

    // Update bounding box parameters
    view.setProjection(0.0f, imageWidth, 0.0f, imageHeight, view.near, view.far);
    glViewport(0, 0, w, h);

    if (view.type == PARALLEL) {
        // Reset view to remove any perspective elements
        glMatrixMode (GL_PROJECTION);
        glLoadIdentity();

        // Create orthographic projection
        glOrtho(view.left, view.right, view.bottom, view.top, view.near, view.far);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
    } else if (view.type == PERSPECTIVE) {
        // Enable culling of unseen polygons
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glMatrixMode (GL_PROJECTION);

        // Add perspective to current orthographic matrix
        gluPerspective(60.0, 1.6, view.near, view.far);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        gluLookAt(view.eye[0], view.eye[1], view.eye[2],
                  view.coi[0], view.coi[1], view.coi[2],
                  0, 1, 0);
    }
}

void mouse(int button, int state, int x, int y) {
    y = imageHeight - y;

    // In 3D viewing mode, we want the trackball to be active
    if (view.type == PERSPECTIVE) {
        long s = 0x00000000;
        s |= (button == GLUT_LEFT_BUTTON)   ? ((state == GLUT_DOWN) ? Trackball::LBUTTON_DOWN : Trackball::LBUTTON_UP) : 0;
        s |= (button == GLUT_MIDDLE_BUTTON) ? ((state == GLUT_DOWN) ? Trackball::MBUTTON_DOWN : Trackball::MBUTTON_UP) : 0;
        s |= (button == GLUT_RIGHT_BUTTON)  ? ((state == GLUT_DOWN) ? Trackball::RBUTTON_DOWN : Trackball::RBUTTON_UP) : 0;

        int key_state = glutGetModifiers();
        s |= (key_state & GLUT_ACTIVE_CTRL)  ? Trackball::CTRL_DOWN  : 0;
        s |= (key_state & GLUT_ACTIVE_ALT)   ? Trackball::ALT_DOWN   : 0;
        s |= (key_state & GLUT_ACTIVE_SHIFT) ? Trackball::SHIFT_DOWN : 0;

        if (s & Trackball::BUTTON_DOWN) {
            trackball.mouse_down(s, x, -y);
        }

        if (s & Trackball::BUTTON_UP) {
            trackball.mouse_up(s, x, -y);
        }
    } else if (view.type == PARALLEL) {
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

void mouseMotion(int x, int y) {
    y = imageHeight - y;

    // Update trackball and exit, because there's no drawing in 3D view.
    if (view.type == PERSPECTIVE) {
        trackball.mouse_motion(x, -y);
        glutPostRedisplay();
    }

    // Only record point if the distance between the new point and the last
    // point is >= the minimum distance.
    float mindist = 0.1;
    float distance = sqrt(pow(x - previousX, 2) + pow(y - previousY, 2));
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

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 27: // escape key
        destroyGLUTMenus();
        exit(0);
        break;
    case 50: // '2' for 2D transition
        transition_2D();
        break;
    case 51: // '3' for 3D transition
        transition_3D();
        break;
    case 99: // 'c' to clear the stroke
        resetStroke();
        break;
    case 108: // 'l' for lighting
        view.light = (view.light == ON) ? OFF : ON;
        glutPostRedisplay();
        break;
    case 109: // 'm' for mesh
        tmesh = tmesh ^ 1;
        glutPostRedisplay();
        break;
    case 118: // 'v' for vertices on mesh
        test_mesh_pts = test_mesh_pts ^ 1;
        glutPostRedisplay();
        break;
    }
}
