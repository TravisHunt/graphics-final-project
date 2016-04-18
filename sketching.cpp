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

    float r = imageWidth/2,  l = -r;
    float t = imageHeight/2, b = -t;
    float n = 1.0f, f = 30.0f;
    view.setProjection(l,r,b,t,n,f);

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
    glEnable(GL_LIGHT1);
    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_LIGHTING);

    glLightfv(GL_LIGHT1, GL_AMBIENT,  view.LightAmbient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE,  view.LightDiffuse);
    glLightfv(GL_LIGHT1, GL_SPECULAR, view.LightSpecular);
    glLightfv(GL_LIGHT1, GL_POSITION, view.LightPosition);

    glMaterialfv(GL_FRONT, GL_AMBIENT,   view.MatAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE,   view.MatDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR,  view.MatSpecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, view.MatShininess);
}

void disableLighting(void) {
    glDisable(GL_LIGHT1);
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
    tracking  = 0;
    previousX = 0;
    previousY = 0;
    wipeCanvas();
}


void generateClosingPoints(void) {
    GLfloat t, delta, cx, cy;

    // interpolate vertices from Endpoint to Startpoint
    pair<int,int> P0(stroke.back());
    pair<int,int> P1(stroke.front());

    // Linearly interpolate points
    t = 0.0; delta = 0.01;
    for (t = 0.0; t <= 1.0; t += delta) {
        cx = (1.0 - t) * P0.first + t * P1.first;
        cy = (P0.second + (P1.second - P0.second) *
            ((cx - P0.first) / (P1.first - P0.first)));

        // add interpolated point to stroke
        stroke.push_back(std::make_pair(cx, cy));
    }

    if (stroke.size() % 2) {
        stroke.push_back(stroke.front());
    }
}

float sideLength(pair<int,int> a, pair<int,int> b) {
	return sqrt(pow((a.first - b.second), 2) + pow((a.second - b.second), 2));
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
		pair<int,int> a = stroke[i];
		pair<int,int> b = stroke[i+(distance/2)];
		pair<int,int> c = stroke[i+distance];
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
	//takes first point of stroke and puts it in vector
	points_on_curve.push_back(stroke[0]);
	//sets the counter variable
	int count = 0;
	//loop through each vertex in stroke
	for (int i=0; i < stroke.size(); i = i+(count-i)) {
		//find the next point for the curve
		count = findNextPoint(i, DISTANCE_BETWEEN_POINTS);
		//add to the vector
		points_on_curve.push_back(stroke[count]);
		std::cout << count << std::endl; //test code
	}
	return;
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
            if (view.type == PARALLEL) return;

            view.type = PARALLEL;
            view.setRGBA(VIEW_RGBA_2D);
            glutReshapeWindow(imageWidth, imageHeight);
            glutPostRedisplay();
            break;
        case SWITCH_3D:
            if (view.type == PERSPECTIVE) return;

            view.type = PERSPECTIVE;
            view.setRGBA(VIEW_RGBA_3D);
            glutReshapeWindow(imageWidth, imageHeight);
            glutPostRedisplay();
            break;
        case TRIANGULATE_2D:
            getOutsideEdges();
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

    // Reset view
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity();

    if (view.type == PARALLEL) {
        std::cout << "Parallel View" << std::endl;

        glColor3f(RGBBLACK);
        //glOrtho(0.0, imageWidth, 0.0, imageHeight, 1.0, 30.0);
        glOrtho(view.left,view.right,view.bottom,view.top,view.near,view.far);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
    } else { // perspective
        std::cout << "Perspective View" << std::endl;

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        glColor3f(RGBWHITE);
        glMatrixMode (GL_PROJECTION);
        glLoadIdentity();

        //gluPerspective(60.0, 1.6, 1.0, 30.0);
        glFrustum(view.left,view.right,view.bottom,view.top,view.near,view.far);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        // gluLookAt(view.eye[0], view.eye[1], view.eye[2],
        //           view.coi[0], view.coi[1], view.coi[2],
        //           0, 1, 0);
    }

    if (view.light == ON) {
        enableLighting();
    } else {
        disableLighting();
    }

    if (teapot) {
        glutSolidTeapot(50.0);
    }

    // Draw user stroke's vertices
    glBegin(GL_LINE_LOOP);
    for (it = stroke.begin(); it != stroke.end(); it++) {
        glVertex2f(it->first, it->second);
    }
    glEnd();

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    glFlush();
}

void reshape(int w, int h) {
    int deltaW  = (w - imageWidth)  >> 1;
    int deltaH  = (h - imageHeight) >> 1;
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
        it->first  += deltaW;
        it->second += deltaH;
    }

    float r = imageWidth/2,  l = -r;
    float t = imageHeight/2, b = -t;
    float n = 1.0f, f = 30.0f;
    view.setProjection(l,r,b,t,n,f);

    // reset viewport to the dimensions
    glViewport(0, 0, w, h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
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
        stroke.push_back(std::make_pair(x, y));

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
