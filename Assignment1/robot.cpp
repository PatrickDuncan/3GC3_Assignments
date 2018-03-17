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
int N = -1;				// Size of the grid NxN 
int menu_id;			// The menu with move or turn	
int move_id;			// The submenu for move
int turn_id;			// The submenu for turn

struct point2D {
	int x;
	int y;
};

point2D body;			// bottom left corner for body
point2D head;			// bottom left corner for head
point2D reset_body;		// the initial left corner for body
point2D reset_head;		// the initial left corner for head

void drawLine(float start, float stop, float constant, bool hor) {
    glPointSize(1.0f);
    glBegin(GL_POINTS);
    glColor3ub(255, 255, 255);

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

void drawHead() {
	glPointSize(1.0f);
    glBegin(GL_POINTS);
    glColor3ub(244, 67, 54);
    float x_left = head.x + 0.02;
    float x_right = head.x + 1 - 0.02;
    float y_left = head.y + 0.02;
    float y_right = head.y + 1 - 0.02;

    for (float i=x_left; i<x_right; i+=0.01) {
    	for (float j=y_left; j<y_right; j+=0.01) {
        	glVertex2f(i, j);
        }
    }
    glEnd();
}

void drawBody() {
	glPointSize(1.0f);
    glBegin(GL_POINTS);
    glColor3ub(139,195,74);
    float x_left = body.x + 0.02;
    float x_right = body.x + 1 - 0.02;
    float y_left = body.y + 0.02;
    float y_right = body.y + 1 - 0.02;

    for (float i=x_left; i<x_right; i+=0.01) {
    	for (float j=y_left; j<y_right; j+=0.01) {
        	glVertex2f(i, j);
        }
    }
    glEnd();
}

void display(void) {
	glClearColor(3.0/255, 169.0/255, 244.0/255, 0);
	glClear(GL_COLOR_BUFFER_BIT);
	drawGrid();
	drawBody();
	drawHead();
	glFlush();
}

void reset() {
	body.x = reset_body.x;
	body.y = reset_body.y;
	head.x = reset_head.x;
	head.y = reset_head.y;
	cout << "Board as been reset" << endl;
	display();
}

void keyboard(unsigned char key, int xIn, int yIn) {
	switch (key) {
		case 'q':
		case 'Q':
		case 27:
			exit(0);
			break;
		case 'r':
		case 'R':
			reset();
			break;
	}
}

void turnProc(int value) {
	int M = N - 1;
	switch (value) {
		case 1:		// Right
			if (head.x > body.x && head.y == body.y) {
				if (head.x > 0 && head.y > 0) { 
					head.x -= 1;
					head.y -= 1;
				}
			} else if (head.x < body.x && head.y == body.y) {
				if (head.x < M && head.y < M) {
					head.x += 1;
					head.y += 1;
				}
			} else if (head.x == body.x && head.y > body.y) {
				if (head.x < M && head.y > 0) {
					head.x += 1;
					head.y -= 1;
				}
			} else if (head.x > 0 && head.y < M) {
				head.x -= 1;
				head.y += 1;
			}
			display();
			break;
		case 2:		// Left
			if (head.x > body.x && head.y == body.y) {
				if (head.x > 0 && head.y < M) {
					head.x -= 1;
					head.y += 1;
				}
			} else if (head.x < body.x && head.y == body.y) {
				if (head.x < M && head.y > 0) {
					head.x += 1;
					head.y -= 1;
				}
			} else if (head.x == body.x && head.y > body.y) {
				if (head.x > 0 && head.y > 0) {
					head.x -= 1;
					head.y -= 1;
				}
			} else if (head.x < M && head.y < M) {
				head.x += 1;
				head.y += 1;
			}
			display();
			break;
	}
}

void moveProc(int value) {
	int M = N - 1;
	switch (value) {
		case 1:		// Right
			if (head.x < M) {
				head.x += 1;
				body.x += 1;
			}
			display();
			break;
		case 2:		// Left
			if (head.x > 0) {
				head.x -= 1;
				body.x -= 1;
			}
			display();
			break;
		case 3:		// Up
			if (head.y < M) {
				head.y += 1;
				body.y += 1;
			}
			display();
			break;
		case 4:		// Down
			if (head.y > 0) {
				head.y -= 1;
				body.y -= 1;
			}
			display();
			break;
	}
}

void createMenu() {
	move_id = glutCreateMenu(moveProc);
	glutAddMenuEntry("Right", 1);
	glutAddMenuEntry("Left", 2);
	glutAddMenuEntry("Up", 3);
	glutAddMenuEntry("Down", 4);

	turn_id = glutCreateMenu(turnProc);
	glutAddMenuEntry("Turn Right", 1);
	glutAddMenuEntry("Turn Left", 2);

	menu_id = glutCreateMenu(NULL);
	glutAddSubMenu("Move", move_id);
	glutAddSubMenu("Turn", turn_id);
	glutAttachMenu(GLUT_LEFT_BUTTON);
}

void randomPositions() {
	head.x = head.y = -1;
	srand(time(NULL));
	body.x = rand() % N;
	body.y = rand() % N;
	int rand_head = rand() % 4;
	int M = N - 1;
	while (head.x == -1 && head.y == -1) {
		switch(rand_head) {
			case 0:		// Above
				if (body.y < M) {
					head.x = body.x;
					head.y = body.y + 1;
				}
				break;
			case 1:		// Right
				if (body.x < M) {
					head.x = body.x + 1;
					head.y = body.y;
				}
				break;
			case 2:		// Bottom
				if (body.y > 0) {
					head.x = body.x;
					head.y = body.y - 1;
				}
				break;
			case 3:		// Left
				if (body.x < 0) {
					head.x = body.x - 1;
					head.y = body.y;
				}
				break;
		}
		rand_head = rand() % 4;
	}
	reset_body.x = body.x;
	reset_body.y = body.y;
	reset_head.x = head.x;
	reset_head.y = head.y;
}

void printInstructions() {
	cout << "-----------------------------------"  << endl;
	cout << "DO NOT ENTER A NUMBER"  << endl;
	cout << "INSTRUCITONS: " << endl;
	cout << "KEY     ACTION" << endl;
	cout << "Q       Quit"   << endl;
	cout << "ESC"            << endl;
	cout << "R       Reset"  << endl;
	cout << "-----------------------------------"  << endl;
}

int main(int argc, char** argv) {
	printInstructions();
	while (N < 10 || N > 50) {
		cout << "Enter the size of the grid (10 to 50):" << endl;
		scanf("%d", &N);
	}
	randomPositions();
	glutInit(&argc, argv);		//starts up GLUT
	glutInitWindowSize(WIDTH, HEIGHT);
	glutInitWindowPosition(200, 200);
	
	glutCreateWindow("Robot");	//creates the window
	createMenu();
	glutDisplayFunc(display);	//registers "display" as the display callback function
	glutKeyboardFunc(keyboard);
	gluOrtho2D(-1, N+1, -1, N+1);
	glutMainLoop();				//starts the event loop

	return(0);					//return may not be necessary on all compilers
}
