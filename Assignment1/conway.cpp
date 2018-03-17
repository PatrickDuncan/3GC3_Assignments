//opengl cross platform includes
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
using namespace std;

const int WIDTH = 700;
const int HEIGHT = 700;
#define N 50			// constant for array

bool fast = true;
bool startupFix = false;// stops the second tick from happening to fast
bool pause = false;
bool beforeState[N][N];
bool afterState[N][N];

void deepCopy() {		// make before = after without aliasing
	for (int i=0; i<N; i++) {
		for (int j=0; j<N; j++) {
			beforeState[i][j] = afterState[i][j];
		}
	}
}

void drawLine(float start, float stop, float constant, bool hor) {
    glPointSize(1.0f);
    glBegin(GL_POINTS);
    glColor3ub(0, 0, 0);

    for (float i=start; i<stop+0.001; i+=0.01) {
        hor ? glVertex2f(constant, i) : glVertex2f(i, constant);
    }
    glEnd();
}

void drawGrid() {
	// Rows
	for (float i=0.0; i<N+1; i++) {
		drawLine(0, N, i, true);
	}
	// Columns
	for (float i=0.0; i<N+1; i++) {
		drawLine(0, N, i, false);
	}
}

void drawBody(int x, int y) {
	glPointSize(1.0f);
    glBegin(GL_POINTS);
    glColor3ub(50, 50, 50);
    float x_left = x + 0.02;
    float x_right = x + 1 - 0.02;
    float y_left = y + 0.02;
    float y_right = y + 1 - 0.02;

    for (float i=x_left; i<x_right; i+=0.01) {
    	for (float j=y_left; j<y_right; j+=0.01) {
        	glVertex2f(i, j);
        }
    }
    glEnd();
}

void doLogic() {
	for (int i=0; i<N; i++) {
		for (int j=0; j<N; j++) {
			int alive = 0;
			// Ternary statement to check if its out of bounds, edges are 'dead'
			alive += i < N-1 ? beforeState[i + 1][j] : 0;
			alive += i > 0 ? beforeState[i - 1][j] : 0;
			alive += i < N-1 && j < N-1 ? beforeState[i + 1][j + 1] : 0;
			alive += i > 0 && j > 0 ? beforeState[i - 1][j - 1] : 0;
			alive += j < N-1 ? beforeState[i][j + 1] : 0;
			alive += j > 0 ? beforeState[i][j - 1] : 0;
			alive += i < N-1 && j > 0 ? beforeState[i + 1][j - 1] : 0;
			alive += i > 0 && j < N-1 ? beforeState[i - 1][j + 1] : 0;
			if (beforeState[i][j] && (alive > 3 || alive < 2)) {
			  	afterState[i][j] = false;
			}
			else if (!beforeState[i][j] && alive == 3) {
				afterState[i][j] = true;
			}
		}
	}
}

void newGeneration() {	//after logic go through all cells and draw alive
	for (int i=0; i<N; i++) {
		for (int j=0; j<N; j++) {
			if (afterState[i][j]) {
				drawBody(i, j);
			}
		}
	}
}

void display(void) {
	glClearColor(230.0/255, 230.0/255, 230.0/255, 0);
	glClear(GL_COLOR_BUFFER_BIT);
	drawGrid();
	doLogic();
	newGeneration();
	glFlush();
	deepCopy();
}

void setPoints() {		// randomize the cells
	srand(time(NULL));
	for (int i=0; i<N; i++) {
		for (int j=0; j<N; j++) {
			if (rand() % 60 < 10) {
				afterState[i][j] = true;
				beforeState[i][j] = true;
			}
		}
	}
}

void clear() {
	for (int i=0; i<N; i++) {
		for (int j=0; j<N; j++) {
			afterState[i][j] = false;
			beforeState[i][j] = false;
		}
	}
}

void mouse(int btn, int state, int x, int y) {
	x = (x - 15) / 13.4;
	y = -1 * (y - 685) / 13.4;

	if (btn == GLUT_LEFT_BUTTON && state == GLUT_UP) {
		afterState[x][y] = !afterState[x][y];
	}
}

void Redraw(int i) {
	if (!pause) {
		if (startupFix) {
			glutPostRedisplay();
		}
		startupFix = true;
		glutTimerFunc(fast ? 350 : 2000, Redraw, 0);
	}
}

void keyboard(unsigned char key, int xIn, int yIn) {
	switch (key) {
		case 'q':
		case 'Q':
		case 27:
			exit(0);
			break;
		case 'f':
		case 'F':
			fast = !fast;
			break;
		case 'r':
		case 'R':
			setPoints();
			break;
		case 'c':
		case 'C':
			clear();
			break;
		case 'p':
		case 'P':
			pause = !pause;
			if (!pause) Redraw(0);
			break;
	}
}

void printInstructions() {
	cout << "-----------------------------------"  << endl;
	cout << "INSTRUCITONS: " << endl;
	cout << "KEY     ACTION" << endl;
	cout << "Q       Quit"   << endl;
	cout << "ESC"            << endl;
	cout << "C       Clear"   << endl;
	cout << "P       Run/Pause"      << endl;
	cout << "F       Fast or slow (Fast default)"  << endl;
	cout << "R       Randomize the grid"  << endl;
	cout << "Press a cell to alternate alive/dead"  << endl;
	cout << "DO NOT SPAM P"  << endl;
	cout << "-----------------------------------"  << endl;
}

int main(int argc, char** argv) {
	printInstructions();
	setPoints();
	glutInit(&argc, argv);		//starts up GLUT
	glutInitWindowSize(WIDTH, HEIGHT);
	glutInitWindowPosition(200, 200);
	
	glutCreateWindow("Conway");	//creates the window
	glutDisplayFunc(display);	//registers "display" as the display callback function
	glutMouseFunc(mouse);
	glutKeyboardFunc(keyboard);
	Redraw(0);
	gluOrtho2D(-1, N+1, -1, N+1);
	glutMainLoop();				//starts the event loop

	return(0);					//return may not be necessary on all compilers
}