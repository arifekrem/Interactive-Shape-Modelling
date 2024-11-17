#include <stdio.h>
#include <windows.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <math.h>
#include <string.h>
#include "surfaceModeller.h"
#include "subdivcurve.h"
#include <algorithm>
#include <vector>

GLdouble worldLeft = -12;
GLdouble worldRight = 12;
GLdouble worldBottom = -9;
GLdouble worldTop = 9;
GLdouble worldCenterX = 0.0;
GLdouble worldCenterY = 0.0;
GLdouble wvLeft = -12;
GLdouble wvRight = 12;
GLdouble wvBottom = -9;
GLdouble wvTop = 9;

GLint glutWindowWidth = 800;
GLint glutWindowHeight = 600;
GLint viewportWidth = glutWindowWidth;
GLint viewportHeight = glutWindowHeight;
GLuint vao = 0;
GLuint vboVertices = 0;
GLuint eboIndices = 0;
GLsizei numIndices = 0;

// screen window identifiers
int window2D, window3D;

int window3DSizeX = 800, window3DSizeY = 600;
GLdouble aspect = (GLdouble)window3DSizeX / window3DSizeY;

// Camera Control Variables
float cameraAzimuth = 0.0f;     // Azimuth angle in degrees
float cameraElevation = 30.0f;  // Elevation angle in degrees
float cameraRadius = 10.0f;     // Camera distance from origin
float rotationSpeed = 0.2f;     // Speed of rotation
float zoomSpeed = 0.5f;         // Speed of zooming
float elevationLimit = 60.0f;   // Maximum elevation angle

// Mouse Interaction
bool isRotating = false;
bool isElevating = false;
bool isZooming = false;
int lastMouseX, lastMouseY;

typedef struct VertexData {
	GLdouble x, y, z;   // Position
	GLdouble nx, ny, nz; // Normal
} VertexData;

int main(int argc, char* argv[]) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);  // Added GLUT_DEPTH for 3D rendering
	glutInitWindowSize(glutWindowWidth, glutWindowHeight);
	glutInitWindowPosition(50, 100);

	// The 2D Window
	window2D = glutCreateWindow("Profile Curve");
	glutDisplayFunc(display2D);
	glutReshapeFunc(reshape2D);
	// Initialize the 2D profile curve system
	init2DCurveWindow();
	// A few input handlers
	glutMouseFunc(mouseButtonHandler2D);
	glutMotionFunc(mouseMotionHandler2D);
	glutPassiveMotionFunc(mouseHoverHandler2D);
	glutMouseWheelFunc(mouseScrollWheelHandler2D);
	glutSpecialFunc(specialKeyHandler2D);
	glutKeyboardFunc(keyboardHandler2D);

	// The 3D Window
	window3D = glutCreateWindow("Surface of Revolution");
	glutPositionWindow(900, 100);
	glutDisplayFunc(display3D);
	glutReshapeFunc(reshape3D);
	glutMouseFunc(mouseButtonHandler3D);  // Updated to support camera controls
	glutMouseWheelFunc(mouseScrollWheelHandler3D);  // Updated for zooming
	glutMotionFunc(mouseMotionHandler3D);  // Updated for rotation and elevation
	glutKeyboardFunc(keyboardHandler3D);  // Updated for toggling drawing styles

	// Set the current window to 3D and initialize GLEW
	glutSetWindow(window3D);
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (GLEW_OK != err) {
		// GLEW failed!
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		exit(1);
	}

	// Initialize the 3D system
	init3DSurfaceWindow();

	// Start the GLUT Main Loop
	glutMainLoop();

	return 0;
}

/************************************************************************************
 *
 *
 * 2D Window and Profile Curve Code
 *
 * Fill in the code in the empty functions
 ************************************************************************************/

 // The profile curve is a subdivision curve
SubdivisionCurve subcurve;

int hoveredCtlPt = -1;
int currentCurvePoint = 0;

// Use little circles to draw subdivision curve control points
Circle circles[MAXCONTROLPOINTS];
int numCirclePoints = 30;
double circleRadius = 0.2;

void init2DCurveWindow()
{
	glLineWidth(3.0);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glClearColor(0.4F, 0.4F, 0.4F, 0.0F);
	initSubdivisionCurve();
	initControlPointCircles();
}

void display2D()
{
	glClear(GL_COLOR_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(wvLeft, wvRight, wvBottom, wvTop);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	draw2DScene();
	glutSwapBuffers();
}


void draw2DScene()
{
	drawAxes();
	drawSubdivisionCurve();
	drawControlPoints();
}

void drawAxes()
{
	glPushMatrix();
	glColor3f(1.0, 0.0, 0);
	glBegin(GL_LINE_STRIP);
	glVertex3f(0, 8.0, 0);
	glVertex3f(0, -8.0, 0);
	glEnd();
	glBegin(GL_LINE_STRIP);
	glVertex3f(-8, 0.0, 0);
	glVertex3f(8, 0.0, 0);
	glEnd();
	glPopMatrix();
}

void drawSubdivisionCurve()
{
	// Subdivide the given curve
	computeSubdivisionCurve(&subcurve);
	glColor3f(0.0, 1.0, 0.0);
	glPushMatrix();
	glBegin(GL_LINE_STRIP);
	for (int i = 0; i < subcurve.numCurvePoints; i++)
	{
		glVertex3f(subcurve.curvePoints[i].x, subcurve.curvePoints[i].y, 0.0);
	}
	glEnd();
	glPopMatrix();
}

void drawControlPoints()
{
	int i, j;
	for (i = 0; i < subcurve.numControlPoints; i++) {
		glPushMatrix();
		glColor3f(1.0f, 0.0f, 0.0f);
		glTranslatef(circles[i].circleCenter.x, circles[i].circleCenter.y, 0);
		// for the hoveredCtlPt, draw an outline and change its color
		if (i == hoveredCtlPt)
		{
			// outline
			glColor3f(0.0, 1.0, 0.0);
			glBegin(GL_LINE_LOOP);
			for (j = 0; j < numCirclePoints; j++) {
				glVertex3f(circles[i].circlePoints[j].x, circles[i].circlePoints[j].y, 0);
			}
			glEnd();
			// color change
			glColor3f(0.5, 0.0, 1.0);
		}
		glBegin(GL_LINE_LOOP);
		for (j = 0; j < numCirclePoints; j++) {
			glVertex3f(circles[i].circlePoints[j].x, circles[i].circlePoints[j].y, 0);
		}
		glEnd();
		glPopMatrix();
	}
}

void initSubdivisionCurve() {
	// Initialize 5 control points of the subdivision curve

	GLdouble x, y;

	x = 2 * cos(M_PI * 0.5);
	y = 2 * sin(M_PI * 0.5);
	subcurve.controlPoints[0].x = x;
	subcurve.controlPoints[0].y = y;

	x = 2 * cos(M_PI * 0.25);
	y = 2 * sin(M_PI * 0.25);
	subcurve.controlPoints[1].x = x;
	subcurve.controlPoints[1].y = y;

	x = 2 * cos(M_PI * 0.0);
	y = 2 * sin(M_PI * 0.0);
	subcurve.controlPoints[2].x = x;
	subcurve.controlPoints[2].y = y;

	x = 2 * cos(-M_PI * 0.25);
	y = 2 * sin(-M_PI * 0.25);
	subcurve.controlPoints[3].x = x;
	subcurve.controlPoints[3].y = y;

	x = 2 * cos(-M_PI * 0.5);
	y = 2 * sin(-M_PI * 0.5);
	subcurve.controlPoints[4].x = x;
	subcurve.controlPoints[4].y = y;

	subcurve.numControlPoints = 5;
	subcurve.subdivisionSteps = 3;
}

void initControlPointCircles()
{
	int num = subcurve.numControlPoints;
	for (int i = 0; i < num; i++) {
		constructCircle(circleRadius, numCirclePoints, circles[i].circlePoints);
		circles[i].circleCenter = subcurve.controlPoints[i];
	}
}

void screenToWorldCoordinates(int xScreen, int yScreen, GLdouble* xw, GLdouble* yw)
{
	GLdouble xView, yView;
	screenToCameraCoordinates(xScreen, yScreen, &xView, &yView);
	cameraToWorldCoordinates(xView, yView, xw, yw);
}

void screenToCameraCoordinates(int xScreen, int yScreen, GLdouble* xCamera, GLdouble* yCamera)
{
	*xCamera = ((wvRight - wvLeft) / glutWindowWidth) * xScreen;
	*yCamera = ((wvTop - wvBottom) / glutWindowHeight) * (glutWindowHeight - yScreen);
}

void cameraToWorldCoordinates(GLdouble xcam, GLdouble ycam, GLdouble* xw, GLdouble* yw)
{
	*xw = xcam + wvLeft;
	*yw = ycam + wvBottom;
}

void worldToCameraCoordiantes(GLdouble xWorld, GLdouble yWorld, GLdouble* xcam, GLdouble* ycam)
{
	double wvCenterX = wvLeft + (wvRight - wvLeft) / 2.0;
	double wvCenterY = wvBottom + (wvTop - wvBottom) / 2.0;
	*xcam = worldCenterX - wvCenterX + xWorld;
	*ycam = worldCenterY - wvCenterY + yWorld;
}

// Functions to Update Camera 
void updateCameraPosition() {
	float radAzimuth = cameraAzimuth * M_PI / 180.0f;  // Convert azimuth to radians
	float radElevation = cameraElevation * M_PI / 180.0f;  // Convert elevation to radians

	float camX = cameraRadius * cos(radElevation) * sin(radAzimuth);
	float camY = cameraRadius * sin(radElevation);
	float camZ = cameraRadius * cos(radElevation) * cos(radAzimuth);

	glLoadIdentity();
	gluLookAt(camX, camY, camZ, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
}

int currentButton;

void mouseButtonHandler2D(int button, int state, int xMouse, int yMouse)
{
	int i;

	currentButton = button;
	if (button == GLUT_LEFT_BUTTON)
	{
		switch (state) {
		case GLUT_DOWN:
			if (hoveredCtlPt > -1)
			{
				screenToWorldCoordinates(xMouse, yMouse, &circles[hoveredCtlPt].circleCenter.x, &circles[hoveredCtlPt].circleCenter.y);
				screenToWorldCoordinates(xMouse, yMouse, &subcurve.controlPoints[hoveredCtlPt].x, &subcurve.controlPoints[hoveredCtlPt].y);
			}
			break;
		case GLUT_UP:
			glutSetWindow(window3D);
			glutPostRedisplay();
			break;
		}
	}
	else if (button == GLUT_MIDDLE_BUTTON)
	{
		switch (state) {
		case GLUT_DOWN:
			break;
		case GLUT_UP:
			if (hoveredCtlPt == -1 && subcurve.numControlPoints < MAXCONTROLPOINTS)
			{
				GLdouble newPointX;
				GLdouble newPointY;
				screenToWorldCoordinates(xMouse, yMouse, &newPointX, &newPointY);
				subcurve.controlPoints[subcurve.numControlPoints].x = newPointX;
				subcurve.controlPoints[subcurve.numControlPoints].y = newPointY;
				constructCircle(circleRadius, numCirclePoints, circles[subcurve.numControlPoints].circlePoints);
				circles[subcurve.numControlPoints].circleCenter = subcurve.controlPoints[subcurve.numControlPoints];
				subcurve.numControlPoints++;
			}
			else if (hoveredCtlPt > -1 && subcurve.numControlPoints > MINCONTROLPOINTS)
			{
				subcurve.numControlPoints--;
				for (i = hoveredCtlPt; i < subcurve.numControlPoints; i++)
				{
					subcurve.controlPoints[i].x = subcurve.controlPoints[i + 1].x;
					subcurve.controlPoints[i].y = subcurve.controlPoints[i + 1].y;
					circles[i].circleCenter = circles[i + 1].circleCenter;
				}
			}

			glutSetWindow(window3D);
			glutPostRedisplay();
			break;
		}
	}

	glutSetWindow(window2D);
	glutPostRedisplay();
}

void mouseMotionHandler2D(int xMouse, int yMouse)
{
	if (currentButton == GLUT_LEFT_BUTTON) {
		if (hoveredCtlPt > -1)
		{
			screenToWorldCoordinates(xMouse, yMouse, &circles[hoveredCtlPt].circleCenter.x, &circles[hoveredCtlPt].circleCenter.y);
			screenToWorldCoordinates(xMouse, yMouse, &subcurve.controlPoints[hoveredCtlPt].x, &subcurve.controlPoints[hoveredCtlPt].y);

		}
	}
	glutPostRedisplay();
	glutSetWindow(window3D);
	glutPostRedisplay();
	glutSetWindow(window2D);
}

void mouseHoverHandler2D(int xMouse, int yMouse)
{
	hoveredCtlPt = -1;
	GLdouble worldMouseX, worldMouseY;
	screenToWorldCoordinates(xMouse, yMouse, &worldMouseX, &worldMouseY);
	// See if we're hovering over a circle
	for (int i = 0; i < subcurve.numControlPoints; i++) {
		GLdouble distToX = worldMouseX - circles[i].circleCenter.x;
		GLdouble distToY = worldMouseY - circles[i].circleCenter.y;
		GLdouble euclideanDist = sqrt(distToX * distToX + distToY * distToY);
		if (euclideanDist < 0.5)
			hoveredCtlPt = i;
	}
	glutPostRedisplay();
}

void mouseScrollWheelHandler2D(int button, int dir, int xMouse, int yMouse)
{
	GLdouble worldViewableWidth;
	GLdouble worldViewableHeight;
	GLdouble cameraOnCenterX;
	GLdouble cameraOnCenterY;
	GLdouble anchorPointX, anchorPointY;
	double clipWindowWidth;
	double clipWindowHeight;
	double wvCenterX = wvLeft + (wvRight - wvLeft) / 2.0;
	double wvCenterY = wvBottom + (wvTop - wvBottom) / 2.0;
	double wvWidth = wvRight - wvLeft;
	double wvHeight = wvTop - wvBottom;

	worldToCameraCoordiantes(worldCenterX, worldCenterY, &cameraOnCenterX, &cameraOnCenterY);
	if (wvWidth >= (worldRight - worldLeft) * 1.2)
	{
		anchorPointX = cameraOnCenterX;
		anchorPointY = cameraOnCenterY;
	}
	else
	{
		// else, anchor the zoom to the mouse
		screenToWorldCoordinates(xMouse, yMouse, &anchorPointX, &anchorPointY);
	}
	GLdouble anchorToCenterX = anchorPointX - wvCenterX;
	GLdouble anchorToCenterY = anchorPointY - wvCenterY;

	// Set up maximum shift
	GLdouble maxPosShift = 50;
	GLdouble maxNegShift = -50;
	anchorToCenterX = (anchorToCenterX > maxPosShift) ? maxPosShift : anchorToCenterX;
	anchorToCenterX = (anchorToCenterX < maxNegShift) ? maxNegShift : anchorToCenterX;
	anchorToCenterY = (anchorToCenterY > maxPosShift) ? maxPosShift : anchorToCenterY;
	anchorToCenterY = (anchorToCenterY < maxNegShift) ? maxNegShift : anchorToCenterY;

	// Move the world centre closer to this point.
	wvCenterX += anchorToCenterX / 4;
	wvCenterY += anchorToCenterY / 4;

	if (dir > 0)
	{
		// Zoom in to mouse point
		clipWindowWidth = wvWidth * 0.8;
		clipWindowHeight = wvHeight * 0.8;
		wvRight = wvCenterX + clipWindowWidth / 2.0;
		wvTop = wvCenterY + clipWindowHeight / 2.0;
		wvLeft = wvCenterX - clipWindowWidth / 2.0;
		wvBottom = wvCenterY - clipWindowHeight / 2.0;
	}
	else
	{
		// Zoom out
		clipWindowWidth = wvWidth * 1.25;
		clipWindowHeight = wvHeight * 1.25;
		wvRight = wvCenterX + clipWindowWidth / 2.0;
		wvTop = wvCenterY + clipWindowHeight / 2.0;
		wvLeft = wvCenterX - clipWindowWidth / 2.0;
		wvBottom = wvCenterY - clipWindowHeight / 2.0;
	}
	glutPostRedisplay();
}

void keyboardHandler2D(unsigned char key, int x, int y)
{
	int i;

	double clipWindowWidth;
	double clipWindowHeight;
	double wvCenterX = wvLeft + (wvRight - wvLeft) / 2.0;
	double wvCenterY = wvBottom + (wvTop - wvBottom) / 2.0;
	double wvWidth = wvRight - wvLeft;
	double wvHeight = wvTop - wvBottom;

	switch (key)
	{
	case 'q':
	case 'Q':
	case 27:
		// Esc, q, or Q key = Quit 
		exit(0);
		break;
	case 107:
	case '+':
		clipWindowWidth = wvWidth * 0.8;
		clipWindowHeight = wvHeight * 0.8;
		wvRight = wvCenterX + clipWindowWidth / 2.0;
		wvTop = wvCenterY + clipWindowHeight / 2.0;
		wvLeft = wvCenterX - clipWindowWidth / 2.0;
		wvBottom = wvCenterY - clipWindowHeight / 2.0;
		break;
	case 109:
	case '-':
		clipWindowWidth = wvWidth * 1.25;
		clipWindowHeight = wvHeight * 1.25;
		wvRight = wvCenterX + clipWindowWidth / 2.0;
		wvTop = wvCenterY + clipWindowHeight / 2.0;
		wvLeft = wvCenterX - clipWindowWidth / 2.0;
		wvBottom = wvCenterY - clipWindowHeight / 2.0;
		break;

	default:
		break;
	}
	glutPostRedisplay();
}

void specialKeyHandler2D(int key, int x, int y)
{
	double clipWindowWidth;
	double clipWindowHeight;
	double wvCenterX = wvLeft + (wvRight - wvLeft) / 2.0;
	double wvCenterY = wvBottom + (wvTop - wvBottom) / 2.0;
	double wvWidth = wvRight - wvLeft;
	double wvHeight = wvTop - wvBottom;

	switch (key)
	{
	case GLUT_KEY_LEFT:
		wvLeft -= 5.0;
		wvRight -= 5.0;
		break;
	case GLUT_KEY_RIGHT:
		wvLeft += 5.0;
		wvRight += 5.0;
		break;
	case GLUT_KEY_UP:
		wvTop += 5.0;
		wvBottom += 5.0;
		break;
	case GLUT_KEY_DOWN:
		wvTop -= 5.0;
		wvBottom -= 5.0;
		break;
		// Want to zoom in/out and keep  aspect ratio = 2.0
	case GLUT_KEY_F1:
		clipWindowWidth = wvWidth * 0.8;
		clipWindowHeight = wvHeight * 0.8;
		wvRight = wvCenterX + clipWindowWidth / 2.0;
		wvTop = wvCenterY + clipWindowHeight / 2.0;
		wvLeft = wvCenterX - clipWindowWidth / 2.0;
		wvBottom = wvCenterY - clipWindowHeight / 2.0;
		break;
	case GLUT_KEY_F2:
		clipWindowWidth = wvWidth * 1.25;
		clipWindowHeight = wvHeight * 1.25;
		wvRight = wvCenterX + clipWindowWidth / 2.0;
		wvTop = wvCenterY + clipWindowHeight / 2.0;
		wvLeft = wvCenterX - clipWindowWidth / 2.0;
		wvBottom = wvCenterY - clipWindowHeight / 2.0;
		break;
	}
	glutPostRedisplay();
}


void reshape2D(int w, int h)
{
	glutWindowWidth = (GLsizei)w;
	glutWindowHeight = (GLsizei)h;
	glViewport(0, 0, glutWindowWidth, glutWindowHeight);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(wvLeft, wvRight, wvBottom, wvTop);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}



/************************************************************************************
 *
 *
 * 3D Window and Surface of Revolution Code
 *
 * Fill in the code in the empty functions
 ************************************************************************************/
 // Ground Mesh material
GLfloat groundMat_ambient[] = { 0.4, 0.4, 0.4, 1.0 };
GLfloat groundMat_specular[] = { 0.01, 0.01, 0.01, 1.0 };
GLfloat groundMat_diffuse[] = { 0.4, 0.4, 0.7, 1.0 };
GLfloat groundMat_shininess[] = { 1.0 };

GLfloat light_position0[] = { 4.0, 8.0, 8.0, 1.0 };
GLfloat light_diffuse0[] = { 1.0, 1.0, 1.0, 1.0 };

GLfloat light_position1[] = { -4.0, 8.0, 8.0, 1.0 };
GLfloat light_diffuse1[] = { 1.0, 1.0, 1.0, 1.0 };

GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat model_ambient[] = { 0.5, 0.5, 0.5, 1.0 };

//
// Surface of Revolution consists of vertices and quads
//
// Set up lighting/shading and material properties for surface of revolution
GLfloat quadMat_ambient[] = { 0.0, 0.0, 0.0, 1.0 };
GLfloat quadMat_specular[] = { 0.45, 0.55, 0.45, 1.0 };
GLfloat quadMat_diffuse[] = { 0.1, 0.35, 0.1, 1.0 };
GLfloat quadMat_shininess[] = { 10.0 };


// Quads and Vertices of the surface of revolution
typedef struct Vertex
{
	GLdouble x, y, z;
	Vector3D normal;
	int numQuads;
	int quadIndex[4];
} Vertex;

typedef struct Quad
{
	int vertexIndex[4]; // 4 vertex indices in clockwise order
	Vector3D normal;
} Quad;

// Quads
Quad* qarray;
boolean quadArrayAllocated = false;

// Vertices 
#define NUMBEROFSIDES 16 // You may want to lower this to 4 or 5 when debugging

Vertex* varray; // 
boolean varrayAllocated = false;

GLdouble fov = 60.0;

boolean drawAsLines = false;
boolean drawAsPoints = false;
boolean drawNormals = false;

GLdouble eyeX = 0.0, eyeY = 3.0, eyeZ = 10.0;
GLdouble radius = eyeZ;
GLdouble zNear = 0.1, zFar = 40.0;

void updateBuffers();

void init3DSurfaceWindow() {
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse0);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position0);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT0, GL_AMBIENT, model_ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse1);
	glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT1, GL_AMBIENT, model_ambient);
	glLightfv(GL_LIGHT1, GL_POSITION, light_position1);

	glShadeModel(GL_SMOOTH);
	glEnable(GL_NORMALIZE);    // Renormalize normal vectors 
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);

	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LINE_SMOOTH);
	glClearColor(0.4F, 0.4F, 0.4F, 0.0F);  // Color and depth for glClear

	glViewport(0, 0, (GLsizei)window3DSizeX, (GLsizei)window3DSizeY);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fov, aspect, zNear, zFar);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(eyeX, eyeY, eyeZ, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

	// Generate VAO and VBOs
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vboVertices);
	glGenBuffers(1, &eboIndices);
}

void reshape3D(int w, int h)
{
	glutWindowWidth = (GLsizei)w;
	glutWindowHeight = (GLsizei)h;
	glViewport(0, 0, glutWindowWidth, glutWindowHeight);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fov, (float)w / (float)h, zNear, zFar);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void display3D() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();
	// Set up the Viewing Transformation (V matrix)    
	gluLookAt(eyeX, eyeY, eyeZ, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

	// Update Camera Position
	updateCameraPosition();

	drawGround();

	// Build and Draw Surface of Revolution (Quad Mesh)
	buildVertexArray();
	buildQuadArray();
	computeQuadNormals();
	computeVertexNormals();
	updateBuffers(); // Update the VBOs with new data

	// Draw quad mesh
	glPushMatrix();
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, quadMat_ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, quadMat_specular);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, quadMat_diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, quadMat_shininess);

	if (drawAsLines)
		drawQuadsAsLines();
	else if (drawAsPoints)
		drawQuadsAsPoints();
	else
		drawQuads();

	glPopMatrix();
	glutSwapBuffers();
}

void drawGround()
{
	glPushMatrix();
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, groundMat_ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, groundMat_specular);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, groundMat_diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, groundMat_shininess);
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);
	glVertex3f(-22.0f, -4.0f, -22.0f);
	glVertex3f(-22.0f, -4.0f, 22.0f);
	glVertex3f(22.0f, -4.0f, 22.0f);
	glVertex3f(22.0f, -4.0f, -22.0f);
	glEnd();
	glPopMatrix();
}


void buildVertexArray()
{
	int row, col;
	double newX, newZ;
	GLdouble cumulativeTheta = 0;
	GLdouble theta = 360 / (GLdouble)NUMBEROFSIDES; // = 18

	// Allocate memeory for the vertex array - needs to be done only once
	if (!varrayAllocated)
	{
		varray = (Vertex*)malloc(subcurve.numCurvePoints * NUMBEROFSIDES * sizeof(Vertex));
		varrayAllocated = true;
	}
	/*  -------------
		|  |  |  |  |
		-------------
		|  |  |  |  |
		-------------
		|  |  |  |  |
		-------------
		|  |  |  |  |
		-------------
	*/
	for (row = 0; row < subcurve.numCurvePoints; row++)
	{
		for (col = 0; col < NUMBEROFSIDES; col++)
		{
			Vector3D newVector = rotateAroundY(subcurve.curvePoints[row].x, 0, cumulativeTheta);
			varray[row * NUMBEROFSIDES + col].numQuads = 0;
			varray[row * NUMBEROFSIDES + col].y = subcurve.curvePoints[row].y;
			varray[row * NUMBEROFSIDES + col].x = newVector.x;
			varray[row * NUMBEROFSIDES + col].z = newVector.z;
			cumulativeTheta += theta;
		}
	}
}

Vector3D rotateAroundY(double x, double z, double theta)
{
	Vector3D newVector;

	newVector.x = cos(theta / 180 * M_PI) * x + sin(theta / 180.0 * M_PI) * z;
	newVector.z = -sin(theta / 180 * M_PI) * x + cos(theta / 180.0 * M_PI) * z;
	return newVector;
}

void buildQuadArray()
{
	int col, row, numQuads;

	if (!quadArrayAllocated)
	{
		qarray = (Quad*)malloc(sizeof(Quad) * (subcurve.numCurvePoints - 1) * NUMBEROFSIDES);
	}
	/*  -------------
		|  |  |  |  |
		-------------
		|  |  |  |  |
		-------------
		|  |  |  |  |
		-------------
		|  |  |  |  |
		-------------
	*/
	for (row = 0; row < subcurve.numCurvePoints - 1; row++)
	{
		for (col = 0; col < NUMBEROFSIDES; col++)
		{
			int nextCol;

			// if the we are at last column, then the next column must wrap around back to 0th column
			if (col == NUMBEROFSIDES - 1)
				nextCol = 0;
			else
				nextCol = col + 1;

			qarray[row * NUMBEROFSIDES + col].vertexIndex[0] = row * NUMBEROFSIDES + col;
			numQuads = varray[row * NUMBEROFSIDES + col].numQuads;
			varray[row * NUMBEROFSIDES + col].quadIndex[numQuads] = row * NUMBEROFSIDES + col;
			varray[row * NUMBEROFSIDES + col].numQuads++;

			qarray[row * NUMBEROFSIDES + col].vertexIndex[1] = (row + 1) * NUMBEROFSIDES + col;
			numQuads = varray[(row + 1) * NUMBEROFSIDES + col].numQuads;
			varray[(row + 1) * NUMBEROFSIDES + col].quadIndex[numQuads] = row * NUMBEROFSIDES + col;
			varray[(row + 1) * NUMBEROFSIDES + col].numQuads++;

			qarray[row * NUMBEROFSIDES + col].vertexIndex[2] = (row + 1) * NUMBEROFSIDES + nextCol;
			numQuads = varray[(row + 1) * NUMBEROFSIDES + nextCol].numQuads;
			varray[(row + 1) * NUMBEROFSIDES + nextCol].quadIndex[numQuads] = row * NUMBEROFSIDES + col;
			varray[(row + 1) * NUMBEROFSIDES + nextCol].numQuads++;

			qarray[row * NUMBEROFSIDES + col].vertexIndex[3] = row * NUMBEROFSIDES + nextCol;
			numQuads = varray[row * NUMBEROFSIDES + nextCol].numQuads;
			varray[row * NUMBEROFSIDES + nextCol].quadIndex[numQuads] = row * NUMBEROFSIDES + col;
			varray[row * NUMBEROFSIDES + nextCol].numQuads++;
		}
	}
}

void computeQuadNormals()
{
	for (int row = 0; row < subcurve.numCurvePoints - 1; row++)
	{
		for (int col = 0; col < NUMBEROFSIDES; col++)
		{
			double normal_x = 0.0;
			double normal_y = 0.0;
			double normal_z = 0.0;

			int i, j;
			Vertex* vi;
			Vertex* vj;
			Vector3D normal;
			normal.x = normal.y = normal.z = 0;

			for (i = 0, j = 1; i < 4; i++, j++)
			{
				if (j == 4) j = 0;
				vi = &varray[qarray[row * NUMBEROFSIDES + col].vertexIndex[i]];
				vj = &varray[qarray[row * NUMBEROFSIDES + col].vertexIndex[j]];

				normal.x += (((vi->z) + (vj->z)) * ((vj->y) - (vi->y)));
				normal.y += (((vi->x) + (vj->x)) * ((vj->z) - (vi->z)));
				normal.z += (((vi->y) + (vj->y)) * ((vj->x) - (vi->x)));
			}
			// Flip them - must be a problem in code above
			normal.x *= -1.0;
			normal.y *= -1.0;
			normal.z *= -1.0;
			Vector3D normalized = normalize(normal);
			qarray[row * NUMBEROFSIDES + col].normal = normalized;
		}
	}
}

void computeVertexNormals()
{
	int col, row, i;
	Vector3D vn;

	for (row = 0; row < subcurve.numCurvePoints; row++)
	{
		for (col = 0; col < NUMBEROFSIDES; col++)
		{
			int numQuads = varray[row * NUMBEROFSIDES + col].numQuads;
			vn.x = vn.y = vn.z = 0;

			for (int i = 0; i < numQuads; i++)
			{
				int quadIndex = varray[row * NUMBEROFSIDES + col].quadIndex[i];
				vn.x += qarray[quadIndex].normal.x;
				vn.y += qarray[quadIndex].normal.y;
				vn.z += qarray[quadIndex].normal.z;
			}
			varray[row * NUMBEROFSIDES + col].normal = normalize(vn);
		}
	}
}

void updateBuffers() {
	// Create VertexData array
	int numVertices = subcurve.numCurvePoints * NUMBEROFSIDES;
	VertexData* vertexData = (VertexData*)malloc(sizeof(VertexData) * numVertices);
	for (int i = 0; i < numVertices; i++) {
		vertexData[i].x = varray[i].x;
		vertexData[i].y = varray[i].y;
		vertexData[i].z = varray[i].z;
		vertexData[i].nx = varray[i].normal.x;
		vertexData[i].ny = varray[i].normal.y;
		vertexData[i].nz = varray[i].normal.z;
	}

	// Update vertex data VBO
	glBindBuffer(GL_ARRAY_BUFFER, vboVertices);
	glBufferData(GL_ARRAY_BUFFER, sizeof(VertexData) * numVertices, vertexData, GL_STATIC_DRAW);
	free(vertexData);

	// Update indices
	int numQuads = (subcurve.numCurvePoints - 1) * NUMBEROFSIDES;
	numIndices = numQuads * 6; // 6 indices per quad (2 triangles)

	GLuint* indices = (GLuint*)malloc(sizeof(GLuint) * numIndices);
	int idx = 0;
	for (int i = 0; i < numQuads; i++) {
		// First triangle
		indices[idx++] = qarray[i].vertexIndex[0];
		indices[idx++] = qarray[i].vertexIndex[1];
		indices[idx++] = qarray[i].vertexIndex[2];

		// Second triangle
		indices[idx++] = qarray[i].vertexIndex[2];
		indices[idx++] = qarray[i].vertexIndex[3];
		indices[idx++] = qarray[i].vertexIndex[0];
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboIndices);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * numIndices, indices, GL_STATIC_DRAW);
	free(indices);

	// Setup VAO
	glBindVertexArray(vao);

	// Bind vertex data
	glBindBuffer(GL_ARRAY_BUFFER, vboVertices);
	// Positions at offset 0
	glVertexAttribPointer(0, 3, GL_DOUBLE, GL_FALSE, sizeof(VertexData), (GLvoid*)offsetof(VertexData, x));
	glEnableVertexAttribArray(0);

	// Normals at offset of nx
	glVertexAttribPointer(1, 3, GL_DOUBLE, GL_FALSE, sizeof(VertexData), (GLvoid*)offsetof(VertexData, nx));
	glEnableVertexAttribArray(1);

	// Bind index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboIndices);

	glBindVertexArray(0);
}

void drawQuads() {
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, quadMat_ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, quadMat_specular);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, quadMat_diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, quadMat_shininess);

	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void drawQuadsAsPoints() {
	glPushMatrix();
	glPointSize(5.0f);
	glBegin(GL_POINTS);

	for (int row = 0; row < subcurve.numCurvePoints; row++) {
		for (int col = 0; col < NUMBEROFSIDES; col++) {
			Vertex* vertex = &varray[row * NUMBEROFSIDES + col];

			glNormal3f(vertex->normal.x, vertex->normal.y, vertex->normal.z);

			glVertex3f(vertex->x, vertex->y, vertex->z);
		}
	}

	glEnd();
	glPopMatrix();
}

void drawQuadsAsLines() {
	// Iterate through all quads in the quad array
	for (int row = 0; row < subcurve.numCurvePoints - 1; row++) {
		for (int col = 0; col < NUMBEROFSIDES; col++) {
			Quad* quad = &qarray[row * NUMBEROFSIDES + col];

			// Begin drawing lines
			glBegin(GL_LINE_LOOP);
			for (int i = 0; i < 4; i++) {
				Vertex* vertex = &varray[quad->vertexIndex[i]];
				glVertex3f(vertex->x, vertex->y, vertex->z);
			}
			glEnd();
		}
	}

	// If drawNormals flag is enabled, draw the normals
	if (drawNormals) {
		glColor3f(1.0, 0.0, 0.0); // Set normal vector color to red
		for (int row = 0; row < subcurve.numCurvePoints - 1; row++) {
			for (int col = 0; col < NUMBEROFSIDES; col++) {
				Vertex* vertex = &varray[row * NUMBEROFSIDES + col];
				Vector3D normal = vertex->normal;

				// Draw normal vectors as lines originating from the vertex
				glBegin(GL_LINES);
				glVertex3f(vertex->x, vertex->y, vertex->z);
				glVertex3f(vertex->x + normal.x, vertex->y + normal.y, vertex->z + normal.z);
				glEnd();
			}
		}
	}
}

// A few utility functions - use VECTOR3D.h or glm if you prefer
Vector3D crossProduct(Vector3D a, Vector3D b)
{
	Vector3D cross;

	cross.x = a.y * b.z - b.y * a.z;
	cross.y = a.x * b.z - b.x * a.z;
	cross.z = a.x * b.y - b.x * a.y;
	return cross;
}

float DotProduct(Vector3D lhs, Vector3D rhs)
{
	return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
}

Vector3D fourVectorAverage(Vector3D a, Vector3D b, Vector3D c, Vector3D d)
{
	Vector3D average;
	average.x = (a.x + b.x + c.x + d.x) / 4.0;
	average.y = (a.y + b.y + c.y + d.y) / 4.0;
	average.z = (a.z + b.z + c.z + d.z) / 4.0;
	return average;
}

Vector3D normalize(Vector3D a)
{
	GLdouble norm = sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
	Vector3D normalized;
	normalized.x = a.x / norm;
	normalized.y = a.y / norm;
	normalized.z = a.z / norm;
	return normalized;
}

// Mouse and Key Handling
//
//
void mouseButtonHandler3D(int button, int state, int x, int y)
{
	currentButton = button;
	lastMouseX = x;
	lastMouseY = y;

	if (button == GLUT_LEFT_BUTTON) {
		isRotating = (state == GLUT_DOWN);
	}
	else if (button == GLUT_RIGHT_BUTTON) {
		isElevating = (state == GLUT_DOWN);
	}
	else if (button == GLUT_MIDDLE_BUTTON) {
		isZooming = (state == GLUT_DOWN);
	}
}

void mouseScrollWheelHandler3D(int button, int dir, int xMouse, int yMouse)
{
	if (dir > 0) {
		cameraRadius -= zoomSpeed;  // Zoom In
	}
	else {
		cameraRadius += zoomSpeed;  // Zoom Out
	}
	cameraRadius = (std::max)(1.0f, cameraRadius);  // Prevent zooming too close
	glutPostRedisplay();
}

void mouseMotionHandler3D(int x, int y)
{
	int dx = x - lastMouseX;
	int dy = y - lastMouseY;

	if (isRotating) {
		cameraAzimuth += dx * rotationSpeed;
	}

	if (isElevating) {
		cameraElevation += dy * rotationSpeed;
		cameraElevation = std::clamp(cameraElevation, 0.0f, elevationLimit);
	}

	if (isZooming) {
		cameraRadius += dy * zoomSpeed;
		cameraRadius = (std::max)(1.0f, cameraRadius);  // Prevent zooming too close
	}

	lastMouseX = x;
	lastMouseY = y;
	glutPostRedisplay();
}

void keyboardHandler3D(unsigned char key, int x, int y)
{

	switch (key)
	{
	case 'q':
	case 'Q':
	case 27:
		// Esc, q, or Q key = Quit 
		exit(0);
		break;
	case 'l':
		drawAsLines = !drawAsLines;
		break;
	case 'p':
		drawAsPoints = !drawAsPoints;
		break;
	case 'n':
		drawNormals = !drawNormals;
		break;
	default:
		break;
	}
	glutPostRedisplay();
}
