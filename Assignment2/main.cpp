#include <stdio.h>
#include <stdlib.h>

#ifdef __APPLE__
#  include <OpenGL/gl.h>
#  include <OpenGL/glu.h>
#  include <GLUT/glut.h>
#else
#  include <GL/gl.h>
#  include <GL/glu.h>
#  include <GL/freeglut.h>
#endif

#include <iostream>
#include <ctime>
#include <math.h>
using namespace std;

bool triangle = true;				// Should we render with triangles or quads?
bool flatShading = true;			// Should we shade flat or Gouraud
bool lighting = false;				// Should we use lighting
bool created = false;				// If the second overview windowo has been created
bool shift = false;					// If the second overview windowo has been created

const int WIDTH = 960;				// Width of the window
const int HEIGHT = 540;				// Height of the window
int N = -1;							// Size of the grid NxN
int wireframe = 0;					// Wireframe choice cycler
int rotating = 0;					// Wireframe choice cycler
int character[] = {0, 0, 0};		// Where the character is on the z
int start[] = {0, 0, 0};			// Start position of the character

const float disp = 3.5;				// The maximum height variation
float maxHeight;;					// Highest hilltop
float translate[2] = {0.0, 0.0};	// The arrow key movement of the camera
float heightMap[300][300];			// The height of every point on the grid
float eye[] = {0, 60, 0};			// first 3 paramters of gluLookAt
float center[] = {0, 0, 0};			// next 3 (4-6) paramters of gluLookAt
float up[] = {0, 1, 0};				// next 3 (7-9) paramters of gluLookAt
float rot[] = {0, 1, 0};			// rotation angle for internal camera
float lightPos[] = {220, 220};		// light positions 

struct point3D {
	float x, y, z;
};

point3D normalFace[300][300];		// rotation angle for internal camera
point3D normalVertex[300][300];		// rotation angle for internal camera

// Draws the terrain using triangles
void drawTerrainTriangle() {
	glFrontFace(GL_CW);
	glBegin(GL_TRIANGLE_STRIP);
	for (int i=1; i<N; i++) {
		if (i%2 == 0) {
			// Odd rows where you go forwards with the strip
			for (int j=0; j<N; j++) {
				glNormal3f(normalFace[i][j].x, normalFace[i][j].y, normalFace[i][j].z);

				glColor3f(heightMap[i-1][j]/maxHeight, 
					      heightMap[i-1][j]/maxHeight, heightMap[i-1][j]/maxHeight);
				glVertex3f(i-1, heightMap[i-1][j], j);
				glColor3f(heightMap[i][j]/maxHeight, 
					      heightMap[i][j]/maxHeight, heightMap[i][j]/maxHeight);
				glVertex3f(i, heightMap[i][j], j);
			}
		} else {
			// Even rows where you go backwards with the strip
			for (int j=N-1; j>-1; j--) {
				glNormal3f(normalFace[i][j].x, normalFace[i][j].y, normalFace[i][j].z);
				
				glColor3f(heightMap[i][j]/maxHeight, 
					      heightMap[i][j]/maxHeight, heightMap[i][j]/maxHeight);
				glVertex3f(i, heightMap[i][j], j);
				glColor3f(heightMap[i-1][j]/maxHeight, 
					      heightMap[i-1][j]/maxHeight, heightMap[i-1][j]/maxHeight);
				glVertex3f(i-1, heightMap[i-1][j], j);
			}
		}
	}
	glEnd();
}

// Draws the terrain using quadrilaterals

void drawTerrainQuad() {
	glFrontFace(GL_CCW);
	for (int i=0; i<N; i++) {
		glBegin(GL_QUAD_STRIP);
		for (int j=0; j<N; j++) {
			// Draw with the better vertex shading
			glNormal3f(normalVertex[i][j].x, normalVertex[i][j].y, normalVertex[i][j].z);

			if (j+1 < N) {				// Bounds
				glColor3f(heightMap[i][j+1]/maxHeight, 
					      heightMap[i][j+1]/maxHeight, heightMap[i][j+1]/maxHeight);
				glVertex3f(i, heightMap[i][j+1], j+1);
			}
			if (i+1 < N && j+1 < N) {	// Bounds
				glColor3f(heightMap[i+1][j+1]/maxHeight, 
					      heightMap[i+1][j+1]/maxHeight, heightMap[i+1][j+1]/maxHeight);
				glVertex3f(i+1, heightMap[i+1][j+1], j+1);
			}
			glColor3f(heightMap[i][j]/maxHeight, 
				      heightMap[i][j]/maxHeight, heightMap[i][j]/maxHeight);
			glVertex3f(i, heightMap[i][j], j);
			if (i+1 < N) {				// Bounds
				glColor3f(heightMap[i+1][j]/maxHeight, 
					      heightMap[i+1][j]/maxHeight, heightMap[i+1][j]/maxHeight);
				glVertex3f(i+1, heightMap[i+1][j], j);
			}
		}
		glEnd();
	}
}

// Draws the terrain using lines
void drawTerrainLines() {
	glLineWidth(2);
	glColor3f(1, 1, 1);
	glBegin(GL_LINES);	
	// basically drawing Ls over and over
	for (int i=0; i<N; i++) {
		for (int j=0; j<N; j++) {
			glVertex3f(i, heightMap[i][j], j);
			if (j+1 < N)
				glVertex3f(i, heightMap[i][j+1], j+1);
			glVertex3f(i, heightMap[i][j], j);
			if (i+1 < N)
				glVertex3f(i+1, heightMap[i+1][j], j);
		}
	}
	glEnd();
}

// Lighting setup
void doLighting() {
	glEnable(GL_LIGHTING); 
	glEnable(GL_LIGHT0);	// First lightbulb
	glEnable(GL_LIGHT1);	// Second lightbulb

	//upload first light data to gpu
 	float pos1[4] = {lightPos[0], N, 0, 1};
 	float lightColor1[4] = {1, 0.5, 0.5, 1};
 	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor1);
 	glLightfv(GL_LIGHT0, GL_POSITION, pos1);

	//upload second light data to gpu
	float pos2[4] = {0, N, lightPos[1], 1};
	float lightColor2[4] = {0.5, 0.5, 1, 1};
 	glLightfv(GL_LIGHT1, GL_DIFFUSE, lightColor2);
	glLightfv(GL_LIGHT1, GL_POSITION, pos2);
}

// from lecture code - draws a beautiful snoman
void drawSnowman() {
	glPushMatrix();
		glTranslatef(0, 1.5, 0);	// fixes clipping//draw body
		
		glColor3f(1,1,1);
		glutSolidSphere(1, 16, 16);
		//draw buttons
		glPushMatrix();
			glTranslatef(0, 0.35, 0.9);
			glColor3f(0, 0, 0);
			glutSolidSphere(0.1, 10, 10);
		glPopMatrix();
		glPushMatrix();
			glTranslatef(0, 0.15, 0.95);
			glColor3f(0, 0, 0);
			glutSolidSphere(0.1, 10, 10);
		glPopMatrix();
		glPushMatrix();
			glTranslatef(0, -0.05, 0.95);
			glColor3f(0, 0, 0);
			glutSolidSphere(0.1, 10, 10);
		glPopMatrix();
		glPushMatrix();
			//translate relative to body, and draw head
			glTranslatef(0, 1.25, 0);
			glColor3f(1,1,1);
			glutSolidSphere(0.5, 16, 16);
			//translate and draw right eye
			glPushMatrix();
				glTranslatef(0.2, 0.15, 0.45);
				glColor3f(0,0,0);
				glutSolidSphere(0.1, 10, 10);
			glPopMatrix();
			//translate and draw left eye
			glPushMatrix();
				glTranslatef(-0.2, 0.15, 0.45);
				glColor3f(0,0,0);
				glutSolidSphere(0.1, 10, 10);
			glPopMatrix();
			//translate and draw nose
			glPushMatrix();
				glTranslatef(0, 0, 0.5);
				glColor3f(1,0.4,0);
				glutSolidSphere(0.1, 10, 10);
			glPopMatrix();
		glPopMatrix();	//body
	glPopMatrix();	//snowman
}

// Draws 3 cute snowmen
void drawCharacters() {
	// A for loop here crashes the program
	// times 10 to slow it down
	glPushMatrix();
		// Translate then draw
		float h = heightMap[start[0]][character[0]/10];
		glTranslatef(start[0], h, character[0]/10);
		drawSnowman();
	glPopMatrix();
	if (character[0] < 10 * (N-2))
		++character[0];
	glPushMatrix();
		h = heightMap[character[1]/10][start[1]];
		glTranslatef(character[1]/10, h, start[1]);
		drawSnowman();
	glPopMatrix();
	if (character[1] < 10 * (N-2))
		++character[1];
	glPushMatrix();
		h = heightMap[start[2]][character[2]/10];
		glTranslatef(start[2], h, character[2]/10);
		drawSnowman();
	glPopMatrix();
	if (character[2] < 10 * (N-2))
		++character[2];
}

// Call display function for the main window
void display(void) {
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glMatrixMode(GL_MODELVIEW); 
	glLoadIdentity();

	// internal camera rotation
	glRotatef(rot[0], 1, 0, 0);
	glRotatef(rot[1], 0, 1, 0);
	glRotatef(rot[2], 0, 0, 1);

	gluLookAt(eye[0], eye[1], eye[2], center[0], center[1], center[2], up[0], up[1], up[2]);

	if (lighting) doLighting();
	else glDisable(GL_LIGHTING);	// Disable lighting
	glShadeModel(flatShading ? GL_FLAT : GL_SMOOTH);  // define flat shading or smooth(gauroud)

	if (wireframe == 0)			// polygons
		triangle ? drawTerrainTriangle() : drawTerrainQuad();
	else if (wireframe == 1)	// lines
		drawTerrainLines();
	else if (wireframe == 2) {	// both
		triangle ? drawTerrainTriangle() : drawTerrainQuad();
		drawTerrainLines();
	}
	
	drawCharacters();	// Character moving on screen
	
	glutSwapBuffers();
}

// Display function for the overview window
void display2(void) {
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);
	glPointSize(1.0);
	glBegin(GL_POINTS);
		for (int i=0; i<N; i++) {
			for (int j=0; j<N; j++) {
				float color = heightMap[i][j]/maxHeight;
				glColor3f(color, color, color);
				glVertex2f(i, j);
			}
		}
	glEnd();
	glutSwapBuffers();
}

// Keyboard function for the overview window
void keyboard2(unsigned char key, int x, int y) {
	if (key == 'q' || key == 'Q')
		exit(0);
}

// Window logic for the overview
void createOverview() {
	// Create the overview window if hasn't been created yet
	if (!created) {
		glutInitWindowSize(N, N);
		glutInitDisplayMode(GLUT_DOUBLE);
		glutInitWindowPosition(0, 0);
		glutCreateWindow("Terrain Overview");
		glutDisplayFunc(display2);
		glutKeyboardFunc(keyboard2);
		gluOrtho2D(0, N, 0, N);
		created = true;
	} 
	// If it has been created change focus, redisplay then restore focus
	else {
		glutSetWindow(2);		// set focus to overview 
		glutPostRedisplay();
		glutSetWindow(1);
	}
}

// Calculates the cross product
point3D crossProduct(point3D a, point3D b) {
	point3D cross;
	cross.x = a.y * b.z - a.z * b.y;
	cross.y = a.z * b.x - a.x * b.z;
	cross.z = a.x * b.y - a.y * b.x;
	return cross;
}

// Get normal for faces
void getNormalFaces() {
	for (int i=0; i<N-1; i++) {
		for (int j=0; j<N-1; j++) {
			point3D v1, v2;
			// v1 = t2 - t1
			v1.x = 1;
			v1.y = heightMap[i + 1][j] - heightMap[i][j];
			v1.z = 0;
			// v2 = t3 - t1
			v2.x = 1;
			v2.y = heightMap[i + 1][j + 1] - heightMap[i][j];
			v2.z = 1;

			// get normal vector then normalize it
			point3D normal = crossProduct(v1, v2);
			float len = sqrtf(pow(normal.x, 2) + pow(normal.y, 2) + pow(normal.z, 2));
			normalFace[i][j].x = normal.x/len;
			normalFace[i][j].y = normal.y/len;
			normalFace[i][j].z = normal.z/len;
		}
	}
}

// Get normal for vertices
// http://www.lighthouse3d.com/opengl/terrain/index.php?normals
void getNormalVertices() {
	for (int i=1; i<N-1; i++) {
		for (int j=1; j<N-1; j++) {
			// Get the vertex for the 4 squares around the middle point
			point3D v1, v2, v3, v4;

			v1.x = 0;
			v1.y = heightMap[i][j - 1] - heightMap[i][j];
			v1.z = -1;

			v2.x = 1;
			v2.y = heightMap[i + 1][j] - heightMap[i][j];
			v2.z = 0;
			
			v3.x = 0;
			v3.y = heightMap[i][j + 1] - heightMap[i][j];
			v3.z = 1;

			v4.x = -1;
			v4.y = heightMap[i - 1][j] - heightMap[i][j];
			v4.z = 0;

			// Get the normal of each square around the middle point
			// Then normalize it
			point3D v12, v23, v34, v41;
			float len12, len23, len34, len41;
			v12 = crossProduct(v1, v2);
			len12 = sqrtf(pow(v12.x, 2) + pow(v12.y, 2) + pow(v12.z, 2));
			v12.x /= len12;		// normalzing
			v12.y /= len12;
			v12.z /= len12;			

			v23 = crossProduct(v2, v3);
			len23 = sqrtf(pow(v23.x, 2) + pow(v23.y, 2) + pow(v23.z, 2));
			v23.x /= len23;		// normalizing
			v23.y /= len23;
			v23.z /= len23;

			v34 = crossProduct(v3, v4);
			len34 = sqrtf(pow(v34.x, 2) + pow(v34.y, 2) + pow(v34.z, 2));
			v34.x /= len34;		// normalizing
			v34.y /= len34;
			v34.z /= len34;

			v41 = crossProduct(v4, v1);
			len41 = sqrtf(pow(v41.x, 2) + pow(v41.y, 2) + pow(v41.z, 2));
			v41.x /= len41;
			v41.y /= len41;		// normalizing
			v41.z /= len41;

			// Get the sum of the 4 vectors and find the length
			float sumX, sumY, sumZ, total;
			sumX = v12.x + v23.x + v34.x + v41.x;
			sumY = v12.y + v23.y + v34.y + v41.y;
			sumZ = v12.z + v23.z + v34.z + v41.z;
			total = sqrtf(pow(sumX, 2) + pow(sumY, 2) + pow(sumZ, 2));

			// Normalize then done!
			normalVertex[i][j].x = sumX/total;
			normalVertex[i][j].y = sumY/total;
			normalVertex[i][j].z = sumZ/total;
		}
	}
}

// Get the heights using The Circles Algorithm: 
// www.lighthouse3d.com/opengl/terrain/index.php?circles
void circles() {
	int circles = (int)N/5; 	// Number of circles
	// For each circle compute the circle
	for (int i=0; i<circles; i++) {
		int centerX = rand() % N,	// The circle's center
		    centerZ = rand() % N,
			diameter = rand() % (int)N/3 + 5;	// The radius of the circle
		for (int j=0; j<N; j++) {
			for (int k=0; k<N; k++) {
				int xSquared = pow(centerX - j, 2),
					ySquared = pow(centerZ - k, 2);
				float dist = sqrtf(xSquared + ySquared),	// Distance from circle center
					  pd = (dist * 2) / diameter;
				if (fabs(pd) <= 5.0) {
					heightMap[j][k] += disp/2 + cos(pd*3.14) * disp/2;
					if (heightMap[j][k] > maxHeight) maxHeight = heightMap[j][k];
				}
			}
		}
	}
	// Get normals for triangles and quads
	getNormalFaces();
	getNormalVertices();

	createOverview();	// Called here for init and randomize
}

// Cycles through polygon, lines or both
void wireframeChange() {
	switch (wireframe) {
		case 0:	// solid polygons
			++wireframe;
			break;
		case 1:	// wireframe version of the terrain
			++wireframe;
			break;
		case 2:	// solid polgons and the wireframe
			wireframe = 0;
			break;
	}
}

// generate a new random terrain using the heightmap generation algorithm
void randomize() {
	for (int i=0; i<N; i++) {
		for (int j=0; j<N; j++) {
			heightMap[i][j] = 0.0;
		}
	}
	circles();	
}

// bring camera/light/snowmen to the original state
void reset() {
	eye[0] = 0;
	eye[1] = 60;
	eye[2] = 0;
	rot[0] = 0;
	rot[1] = 1;
	rot[2] = 0;
	center[0] = 50 * cos(rot[1]);
	center[1] = 0;
	center[2] = 50 * sin(rot[1]);
	character[0] = 0;
	character[1] = 0;
	character[2] = 0;
	triangle = true;
	flatShading = true;
	lighting = false;
	wireframe = 0;
	rotating = 0;
	lightPos[0] = 220;
	lightPos[1] = 220;
}

// rotate camera around its internal xyz axis
void internalCamera(unsigned char key) {
	float change;	// postive or negative
	switch (key) {
		case 'x':	// rotate positively on the internal x-axis
		case 'X':	// rotate negatively on the internal x-axis
			change = key == 'x' ? 1 : -1;					  // postive or negative
			rot[0] = rot[0] < 360 ? rot[0] + change * 4 : 0;  // increment or reset to 0
			
			break;
		case 'c':	// rotate positively on the internal y-axis
		case 'C':	// rotate negatively on the internal y-axis
			change = key == 'c' ? 1 : -1;					  // postive or negative
			rot[1] = rot[1] < 360 ? rot[1] + change * 4 : 0;  // increment or reset to 0
			
			break;
		case 'z':	// rotate positively on the internal z-axis
		case 'Z':	// rotate negatively on the internal z-axis
			change = key == 'z' ? 1 : -1;					  // postive or negative
			rot[2] = rot[2] < 360 ? rot[2] + change * 4 : 0;  // increment or reset to 0
			break;
	}
}

// Allow the user to set an arbitrary camera position
void arbitraryCamera() {
	cout << "Enter the 9 parameters of gluLookAt(eye[0-2], center[0-2], up[0-2])" << endl;
	cout << "Current coordinates are:" << endl;
	for (int i=0; i<3; i++) {
		cout << "eye." << i << " " << eye[i] <<endl;
		cout << "center." << i << " " << center[i] << endl;
		cout << "up." << i << " " << up[i] << endl;
	}
	// Enter the paramters
	scanf("%f", &eye[0]);
	scanf("%f", &eye[1]);
	scanf("%f", &eye[2]);
	scanf("%f", &center[0]);
	scanf("%f", &center[1]);
	scanf("%f", &center[2]);
	scanf("%f", &up[0]);
	scanf("%f", &up[1]);
	scanf("%f", &up[2]);
}

// Adding functionality with the keyboard
void keyboard(unsigned char key, int xIn, int yIn) {
	switch (key) {
		case 'q':		// Quit
		case 'Q':
		case 27:
			exit(0);
			break;
		case 'b':		// Do arbitrary camera
		case 'B':
			arbitraryCamera();
			break;
		case 'w':		// Call the wireframe function that cycles the wireframe
		case 'W':
			wireframeChange();
			break;
		case 'l':		// toggle lighting
		case 'L':
			lighting = !lighting;
			break;
		case 'n':		// move first light
		case 'N':
			lightPos[0] += key == 'n' ? 5 : -5; // postive or negative
			break;
		case 'm':		// move second light
		case 'M':
			lightPos[1] += key == 'm' ? 5 : -5; // postive or negative
			break;
		case 'f':		// toggle flat shading
		case 'F':
			flatShading = !flatShading;
			break;
		case 'r':		// randomize the terrain (circles)
			randomize();
			break;
		case 'R':		// reset the the camera/light/snowmen to the initial
			reset();
			break;
		case 't':		// set polygon creation to triangles
		case 'T':
			triangle = true;
			break;
		case 'y':		// set polygon creation to quads
		case 'Y':
			triangle = false;
			break;
		default:
			internalCamera(key);	// internal camera stuff
			break;
	}
}

// Camera controls for moving
void special(int key, int x, int y) {
	bool shift = glutGetModifiers() & GLUT_ACTIVE_SHIFT;
	switch(key) {
		case GLUT_KEY_LEFT:		// Move the camera to the x
			eye[0] += shift ? -2 : 2; 
			center[0] += shift ? -2 : 2;
			break; 
		case GLUT_KEY_RIGHT:	// Move the camera to the z
			eye[2] += shift ? -2 : 2; 
			center[2] += shift ? -2 : 2; 
			break;
		case GLUT_KEY_UP: 		// Move the camera to the up
			eye[1] += 2; 
			center[1] += 2;
			break; 
		case GLUT_KEY_DOWN: 	// Move the camera to the down
			eye[1] -= 2; 
			center[1] -= 2; 
			break;
    }
}

// Redraws the screen with 60 fps
void redraw(int i) {
	glutPostRedisplay();
	glutTimerFunc(17, redraw, 0);
}

// Print the program's instructionsn to the console
void printInstructions() {
	cout << "-----------------------------------"  << endl;
	cout << "S == SHIFT";
	cout << "INSTRUCITONS: " << endl;
	cout << "KEY     ACTION" << endl;
	cout << "Q       Quit"   << endl;
	cout << "ESC"            << endl;
	cout << "B       Input an arbitrary camera position" << endl;
	cout << "F       Toggle between flat and Gouraud shading"  << endl;
	cout << "L       Toggle lighting"  << endl;
	cout << "n       Move light 0 positively"  << endl;
	cout << "N       Move light 0 negatively"  << endl;
	cout << "m       Move light 1 positively"  << endl;
	cout << "M       Move light 1 negatively"  << endl;
	cout << "r       Randomize the grid"  << endl;
	cout << "R       Reset the grid"  << endl;
	cout << "T       Draw with triangles"  << endl;
	cout << "W       Cycle through polygon, wireframe, and both"  << endl;
	cout << "Y       Draw with quads"  << endl;
	cout << "CAMERA CONTROLS:"  << endl;
	cout << "left    Move camera along the x axis positively"  << endl;
	cout << "LEFT    Move camera along the x axis positively"  << endl;
	cout << "right   Move camera along the z axis positively"  << endl;
	cout << "RIGHT   Move camera along the z axis positively"  << endl;
	cout << "UP      Move camera upwards"  << endl;
	cout << "DOWN    Move camera downwards"  << endl;
	cout << "x       Rotate positively on the internal x-axis"  << endl;
	cout << "X       Rotate negatively on the internal x-axis"  << endl;
	cout << "c       Rotate positively on the internal y-axis"  << endl;
	cout << "C       Rotate negatively on the internal y-axis"  << endl;
	cout << "z       Rotate positively on the internal z-axis"  << endl;
	cout << "Z       Rotate negatively on the internal z-axis"  << endl;
	cout << "-----------------------------------"  << endl;
}

//Initialize the scene
void init() {	
    glMatrixMode(GL_PROJECTION);
	gluPerspective(90, 1, 1, 400);

	while (N < 50 || N > 300) {
		cout << "Enter the size of the grid (50 to 300): " << endl;
		scanf("%d", &N);
	}
	srand(time(0));

	// Random character start positions
	for (int i=0; i<3; ++i)
		start[i] = rand() % N-1 + 2;

	// Set camera up
	center[0] = 50 * cos(rot[1]);	// circular motion
	center[2] = 50 * sin(rot[1]);

	circles();	// get the heights initially
}

int main(int argc, char** argv) {
	printInstructions();
	
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutInitWindowPosition(300, 300);
	glutCreateWindow("Terrain");
	// Interactivity
	glDepthFunc(GL_LESS);
	glEnable(GL_DEPTH_TEST);
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(special);
	init();
	// Graphics start
	redraw(0);
	glutMainLoop();
	return(0);
}