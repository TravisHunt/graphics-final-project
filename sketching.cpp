#include "sketching.h"

int main(int argc, char *argv[]) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);

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
    //view.setCOI(0, 0, 0); // set coi to origin
    //view.setEyePos(1, 1, 1); // camera position
    tracking = 0;
    previousX = 0;
    previousY = 0;

    createGLUTMenus();
}

int inWindow(int x, int y) {
    return (x > 0 && x < imageWidth && y > 0 && y < imageHeight);
}

void wipeCanvas(void) {
    glClearColor(RGBGREY, 1);
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
		if ((PI - calcAngle(ba, bc, ac)) > VERTEX_LIMIT) {
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
    switch(option) {
        case CLEAR:
            resetStroke();
            break;
        case SWITCH_2D:
            // ...
            break;
        case SWITCH_3D:
            // ...
            break;
        case TRIANGULATE_2D:
            getOutsideEdges();
            glBegin(GL_LINES);
            for (it = points_on_curve.begin(); it != points_on_curve.end(); it++) {
                glVertex2f(it->first, it->second);
            }
            glEnd();
            glFlush();
        default:
            break;
    }
}

void destroyGLUTMenus(void) {
    glutDestroyMenu(menu_2Dview);
}


/******** GLUT CALLBACKS **********/
void display(void) {
    glClearColor(RGBGREY, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glColor3f(RGBBLACK);

    // Draw user stroke's vertices
    glBegin(GL_LINE_LOOP);
    for (it = stroke.begin(); it != stroke.end(); it++) {
        glVertex2f(it->first, it->second);
    }
    glEnd();

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

    // reset viewport to the dimensions
    glViewport(0, 0, w, h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluOrtho2D(0, imageWidth, 0, imageHeight);

    glMatrixMode(GL_MODELVIEW);
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
    }
}
