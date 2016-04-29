#include "sketching.h"

#define RADIANS(deg) (deg * (PI / 180))
#define DEGREES(rad) (rad * (180 / PI))

static int teapot = 0;

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

    tracking  = 0;
    previousX = 0;
    previousY = 0;

    view.setCOI(0, 0, 0); // set coi to origin
    view.setEyePos(0, 0, 5); // camera position
    view.setRGBA(VIEW_RGBA_2D);

    view.setProjection(0.0f, imageWidth, 0.0f, imageHeight, -100.0f, 100.0f);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT1);
    view.setLightAmbient(0.0f, 0.0f, 0.0f, 1.0f);
    view.setLightDiffuse(1.0f, 1.0f, 1.0f, 1.0f);
    view.setLightSpecular(1.0f, 1.0f, 1.0f, 1.0f);
    view.setLightPosition(2.0f, 5.0f, 5.0f, 0.0f);
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
	display_triangles = 0;
    tracking  = 0;
    previousX = 0;
    previousY = 0;
    wipeCanvas();
}

/********* INTERPOLATION ***************/

void calculateVerticesDriver(){
	//loop through connected
	for (int index = 0; index < connected.size(); index++){
		//loop through the different degree values
		for (int theta = 0; theta < 330; theta = theta + 30){
			calculateVertices(index, (theta*(180/PI)));
		}
	}
}

// FIXME: This should accept 2 vertices by reference
Vector3f calculateMidpoint(int index){
    int x1, y1, x2, y2, z1, z2;
    int xm, ym, zm;

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

void calculateVertices(int index, float Theta) {
    int x, y, z, a, b, c, u, v, w;
    float newx, newy, newz;
    float theta;
    Vector3f midpoint = calculateMidpoint(index);
    x = connected[index].p1->x();
    y = connected[index].p1->y();
    z = connected[index].p1->z();
    // x = connected[index].first.first;
    // y = connected[index].first.second;
    // z = 0;
    a = midpoint.x();
    b = midpoint.y();
    c = midpoint.z();
    // a = midpoint.first;
    // b = midpoint.second;
    // c = 0;
    u = -a;
    v = b;
    w = 0;

    int L = ((u*u)+(v*v)+(w*w));

    //calculates rotates x
    newx = (a*((v*v)+(w*w))-(u*((b*v)+(c*w)-(u*x)-(v*y)-(w*z))));
    newx = newx*(1-cos(theta))+(L*x*(cos(theta)))+((sqrt(L))*((-c*v)+(b*w)-(w*y)+(v*z))*sin(theta));
    newx = newx/L;
    //calculated rotated y
    newy = (b*((u*u)+(w*w))-(v*((a*u)+(c*w)-(u*x)-(v*y)-(w*z))));
    newy = newy*(1-cos(theta))+(L*y*(cos(theta)))+((sqrt(L))*((c*u)+(a*w)-(w*y)+(u*z))*sin(theta));
    newy = newy/L;
    //calculates rotated z
    newz = (c*((u*u)+(v*v))-(w*((a*u)+(b*v)-(u*x)-(v*y)-(w*z))));
    newz = newz*(1-cos(theta))+(L*z*(cos(theta)))+((sqrt(L))*((-b*u)+(a*v)-(v*x)+(u*y))*sin(theta));
    newz = newz/L;

    //push directly into the vector to save it.
    vertices_on_shape.push_back(Vector3f(newx, newy, newz));
}



/* commented out incomplete code */
void populateConnected(){
    if (points_on_curve.size() == 0) return;
	//sets the original size to iterate over
	go_back_for.push_back(0);
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
				count_forward ++;
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
				count_back --;
			/*  } */
		}
	}
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
	Vector3f a = points_on_curve[index1 +1];
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

void drawNewLine(int index, int x){

	GLfloat t, delta, cx, cy;

	vector<Vector3f > partition;

    // interpolate vertices from Endpoint to Startpoint
    Vector3f P0(stroke[index]);
    Vector3f P1(stroke[index + x]);

	float length = sideLength(P0, P1);

	delta = 1/(length/12);

    // Linearly interpolate points
    t = 0.0;
    glBegin(GL_LINE_STRIP);
    for (t = 0.0; t <= 1.0; t += delta) {
        cx = (1.0 - t) * P0(0) + t * P1(0);
        cy = (P0(1) + (P1(1) - P0(1)) *
            ((cx - P0(0)) / (P1(0) - P0(0))));

        // draw line between new point and last point
        glVertex2f(cx, cy);
        // add interpolated point to stroke
        partition.push_back(Vector3f(cx, cy, 0));
    }
    glEnd();
    // push closing line to screen
    glFlush();

	//vector<Vector3f>::iterator it;
	it = points_on_curve.begin();
	if (x < 0) {
		points_on_curve.insert(it+index, partition.rbegin(), partition.rend());
	} else {
		points_on_curve.insert(it+index, partition.begin(), partition.end());
	}

}

Vector3f getNormal(Vector3f &a, Vector3f &b){

    // NOTE: This is not how you find the normal of 2 vectors
	//int dx = b.first - a.first;
	//int dy = b.second - a.second;

    // NOTE: The cross product of two vectors gives you the normal.
	return a.cross(b);

}

void generateClosingPoints(void) {
    GLfloat t, delta, cx, cy;

    // interpolate vertices from Endpoint to Startpoint
    Vector3f P0(stroke.back());
    Vector3f P1(stroke.front());

    float length = sideLength(P0, P1);

    // Linearly interpolate points
    t = 0.0; delta = 1/(length/12);
    for (t = 0.0; t <= 1.0; t += delta) {
        cx = (1.0 - t) * P0.x() + t * P1.y();
        cy = (P0.y() + (P1.y() - P0.y()) *
            ((cx - P0.x()) / (P1.x() - P0.x())));

        // add interpolated point to stroke
        stroke.push_back(Vector3f(cx, cy, 0));
        //stroke.push_back(std::make_pair(cx, cy));
    }
}

float sideLength(Vector3f &a, Vector3f &b) {
	return sqrt(pow((a.x() - b.y()), 2) + pow((a.y() - b.y()), 2));
}

float calcAngle(float BA, float BC, float AC) {
	return acos((pow(BA, 2) + pow(BC, 2) - pow(AC, 2))/(2 * BA * BC));
}

int findNextPoint(int i, int distance){
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
            std::cout << "Angle: " << calcAngle(ba, bc, ac) << std::endl;
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
	for (int i=0; i < stroke.size(); i = i+(count-i)) {
		//find the next point for the curve
		count = findNextPoint(i, DISTANCE_BETWEEN_POINTS);
		//add to the vector
        if (stroke[count].x() != 0 && stroke[count].y() != 0) {
		    points_on_curve.push_back(stroke[count]);
        }
		std::cout << count << std::endl; //test code
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
    view.setRGBA(VIEW_RGBA_3D);

    /* launch 3D mesh creation */
    getOutsideEdges();
    populateConnected();
    calculateVerticesDriver();

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



/******** GLUT CALLBACKS **********/
void display(void) {
    glClearColor(view.rgba[0], view.rgba[1], view.rgba[2], view.rgba[3]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    if (view.type == PARALLEL) {
        // Reset view to remove any perspective elements
        glMatrixMode (GL_PROJECTION);
        glLoadIdentity();
        glColor3f(RGBBLACK);

        // Create orthographic projection
        glOrtho(view.left, view.right, view.bottom, view.top, view.near, view.far);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        // Draw user stroke's vertices
        glBegin(GL_LINE_LOOP);
        for (it = stroke.begin(); it != stroke.end(); it++) {
            glVertex2f(it->x(), it->y());
        }
        glEnd();

        if (display_triangles) {
    		glBegin(GL_LINES);
    		//display_triangles = 0;
    		std::cout << vertices_on_shape.size() << std::endl;
    		for (it2 = connected.begin(); it2 != connected.end(); it2++) {
    			glVertex2f(it2->p1->x(), it2->p1->y());
    			glVertex2f(it2->p2->x(), it2->p2->y());
    			//std::cout << it2->first.x() << " , " << it2->first.second << std::endl; //test code
    			//std::cout << it2->second.x() << " , " << it2->second.second << std::endl; //test code
    		}
    		glEnd();
    		glPointSize(5);
    		glBegin(GL_POINTS);
    		for (it = points_on_curve.begin(); it != points_on_curve.end(); it++){
    			glVertex2f(it->x(), it->y());
    		}
    		glEnd();
    	}

    } else { // perspective
        // Enable culling of unseen polygons
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        glColor3f(RGBWHITE);
        glMatrixMode (GL_PROJECTION);

        // Add perspective to current orthographic matrix
        gluPerspective(60.0, 1.6, -100.0, 100.0);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        gluLookAt(view.eye[0], view.eye[1], view.eye[2],
                  view.coi[0], view.coi[1], view.coi[2],
                  0, 1, 0);

        glPushMatrix();
        glPointSize(3);
        glBegin(GL_POINTS);
        vector<Vector3f>::const_iterator v;
        for (v = vertices_on_shape.begin(); v != vertices_on_shape.end(); v++) {
            //std::cout << *v << std::endl;
            glVertex3f(v->x(), v->y(), v->z());
        }
        glEnd();
        glPopMatrix();
    }

    if (view.light == ON) {
        enableLighting();
    } else {
        disableLighting();
    }

    if (teapot) {
        glPushMatrix();
        glTranslatef(imageWidth/2, imageHeight/2, 0.0f);
        glutSolidTeapot(50.0);
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

    view.setProjection(0.0f, imageWidth, 0.0f, imageHeight, -100.0f, 100.0f);

    // reset viewport to the dimensions
    glViewport(0, 0, w, h);
}

void mouse(int button, int state, int x, int y) {
    y = imageHeight - y;

    if (button != GLUT_LEFT_BUTTON) return;

    if (state == GLUT_DOWN) {
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
    } else { // stroke complete
        tracking = 0;
        previousX = 0;
        previousY = 0;
        // get start & end connecting vertices
        generateClosingPoints();
        // redraw stroke
        glutPostRedisplay();
    }
}

void mouseMotion(int x, int y) {
    y = imageHeight - y;

    if (tracking && inWindow(x, y)) {
        // push vertex into vector
        stroke.push_back(Vector3f(x, y, 0));
        //stroke.push_back(std::make_pair(x, y));

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
        case 108: // 'l' for lighting
            view.light = view.light == ON ? OFF : ON;
            glutPostRedisplay();
            break;
        case 116: // 't' for teapot
            teapot = teapot^1;
            glutPostRedisplay();
            break;
    }
}
