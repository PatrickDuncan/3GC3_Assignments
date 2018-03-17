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
#include <vector>
#include <cmath>
using namespace std;

// Include project files
#include "SceneObject.h"

const int WIDTH = 700;				// screen width
const int HEIGHT = 700;				// screen height
int length = 0;						// number of objects
int selected = -1;					// which object is selected
int xOrigin = 0;					// the x origin for mouse dragging
int yOrigin = 0;					// the y origin for mouse dragging
int materialDef = 1;				// the default material for objects

char filename[50];					// filename

float eye[] = {-4, 4, -4};			// (0-3) paramters of gluLookAt
float center[] = {0, 0, 0};			// next 3 (4-6) paramters of gluLookAt
float rot[] = {0, 1, 0};			// rotation angle for internal camera

/* Used for the mouse functions */
float angle = 0.0f;					
float angle2 = 0.0f;
float lx = 0.0f, lz = 0.0f;
float deltaAngle = 0.0f;
float deltaAngle2 = 0.0f;

Structure::point3D light1;			// light 1's position x, y, z
Structure::point3D light2;			// light 2's position x, y, z

FILE *fp;							// The file to save/load from

SceneObject objects[100];			// the list of objects

// Removes an object from the list
void removeObject(int posDel) {
	if (selected == posDel)	// if the object is selected, make none selected
		selected = -1;
	// Shift the objects after the deleted to the left by 1
	for (int i = posDel; i < length && i < 100; ++i) {
		SceneObject temp;
		Structure::point3D posi, roti, scal;
		posi = objects[i + 1].getPosition();
		roti = objects[i + 1].getRotation();
		scal = objects[i + 1].getScale();
		objects[i].setType(objects[i + 1].getType());
		objects[i].setPosition(posi.x, posi.y, posi.z);
		objects[i].setRotation(roti.x, roti.y, roti.z);
		objects[i].setScale(scal.x, scal.y, scal.z);
		objects[i + 1].clear();
	}
	--length;
}

//calculate whether an intersection of our ray hits the objects
void calcIntersections(bool del, bool mat, int mouseX, int mouseY) {
	//construct Ray
	double R0[3], R1[3], Rd[3];
	double modelMat[16], projMat[16];
	int viewMat[4];

	//populate mpv matricies
	glGetDoublev(GL_MODELVIEW_MATRIX, modelMat);
	glGetDoublev(GL_PROJECTION_MATRIX, projMat);
	glGetIntegerv(GL_VIEWPORT, viewMat);

	//calculate near point
	gluUnProject(mouseX, mouseY, 0.0, modelMat, projMat, viewMat, &R0[0], &R0[1], &R0[2]);
	//calculate far point
	gluUnProject(mouseX, mouseY, 1.0, modelMat, projMat, viewMat, &R1[0], &R1[1], &R1[2]);

	//calcualte our ray from R0 and R1
	Rd[0] = R1[0] - R0[0];
	Rd[1] = R1[1] - R0[1];
	Rd[2] = R1[2] - R0[2];

	//turn ray Rd into unit ray
	GLdouble m = sqrt(Rd[0]*Rd[0] + Rd[1]*Rd[1] + Rd[2]*Rd[2]);
	Rd[0] /= m;
	Rd[1] /= m;
	Rd[2] /= m;
	// check the objects  for intersection
	for (int i = length - 1; i > -1; --i) {
		Structure::point3D where = objects[i].getPosition();
		Structure::point3D bounds = objects[i].getScale();
		float boundX, boundY, boundZ;
		boundX = bounds.x / 1.7;
		boundY = bounds.y / 1.7;
		boundZ = bounds.z / 1.7;
		double t = (((double)where.z) - R0[2])/Rd[2];

		//use t value to find x and y of our intersection point
		double pt[3];
		pt[0] = R0[0] + t * Rd[0];
		pt[1] = R0[1] + t * Rd[1];
		pt[2] = where.z;
		//now that we have our point on the xy plane at the level of the object
		if (pt[0] > where.x - boundX && pt[0] < where.x + boundX &&
			pt[1] > where.y - boundY && pt[1] < where.y + boundY &&
			pt[2] > where.z - boundZ && pt[2] < where.z + boundZ) {
			if (!mat) {
				if (del)
					removeObject(i);
				else if (selected == i)	// unselect if already selected
					selected = -1;
				else 
					selected = i;
			} else {
				objects[i].setMaterial(materialDef);
			}
			break;
		}
	}
}

//save our mouse coords when they change
void mouse(int btn, int state, int x, int y) {
	int mouseX = x;
	int mouseY = HEIGHT - y;
	//if the left button is being used
	if(btn == GLUT_LEFT_BUTTON){
		//if the left button is being released.
		if(state == GLUT_UP){
			//reset current mouse angle and disallows mouseMouse from running.
			lx = 0;
			lz = 0;
			angle = 0;
			xOrigin = -1;
			yOrigin = -1;
			calcIntersections(false, false, mouseX, mouseY);
		}
		//if the left button is being pressed.
		else {
			//sets current x,y coordinate of the mouseas the cooridinates to be used for calculation.
			xOrigin = x;
			yOrigin = y;
		}
	}
	else if (btn == GLUT_RIGHT_BUTTON && state == GLUT_UP) {	// delete the object
		calcIntersections(true, false, mouseX, mouseY);
	}
}

/* Callback for when the mouse is moving */
void mouseMove(int x, int y) {
	//if the left button is being pressed (no released).
	if (xOrigin >= 0) {
		//calculates the the distance the camera will move when the mouse is moved.
		deltaAngle = (x - xOrigin) * 0.001f;
		deltaAngle2 = (y - yOrigin) * 0.001f;
		lx = sin(angle - deltaAngle) * 70;
		lz = sin(angle - deltaAngle2) * 90;
	}
}

// Render an object by what type it is
void renderObject(string type) {
	if (type == "Cube")
		glutSolidCube(1);
	else if (type == "Sphere")
		glutSolidSphere(1, 100, 100);
	else if (type == "Octahedron")
		glutSolidOctahedron();
	else if (type == "Dodecahedron")
		glutSolidDodecahedron();
	else if (type == "Tetrahedron")
		glutSolidTetrahedron();
}

// draws the ground 
void drawGround() {
	glPushMatrix();
	glEnable(GL_COLOR_MATERIAL);
	glTranslatef(-50, -1, -50);

	glColor3f(0.5, 0.5, 0.7);

	for(int i = 0; i < 100; i++){
		for(int j = 0; j < 100; j++){
			glBegin(GL_QUAD_STRIP);
				glNormal3f(0, 1, 0);
				//floor
				glVertex3f(i    , 0, j + 1);
				glVertex3f(i + 1, 0, j + 1);
				glVertex3f(i    , 0, j    );
				glVertex3f(i + 1, 0, j    );
			glEnd();
		}
	}
	glDisable(GL_COLOR_MATERIAL);
	glPopMatrix();
}

// Set the material of an object to 5 predefined materiels
void setMaterial(int mat) {
	float amb[4];
	float diff[4];
	float spec[4];
	switch(mat) {
		case 1:		// red rubber
			amb[0] = 0.05;
			amb[1] = 0;
			amb[2] = 0;
			amb[3] = 1;
			diff[0] = 0.5;
			diff[1] = 0.4;
			diff[2] = 0.4;
			diff[3] = 1;
			spec[0] = 0.7;
			spec[1] = 0.04;
			spec[2] = 0.04;
			spec[3] = 1;
			glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
			glMaterialfv(GL_FRONT, GL_DIFFUSE, diff);
			glMaterialfv(GL_FRONT, GL_SPECULAR, spec);
			glMaterialf(GL_FRONT, GL_SHININESS, 0.78125);
			break;
		case 2:		// Gold
			amb[0] = 0.24725;
			amb[1] = 0.1995;
			amb[2] = 0.0745;
			amb[3] = 1;
			diff[0] = 0.75164;
			diff[1] = 0.60648;
			diff[2] = 0.22648;
			diff[3] = 1;
			spec[0] = 0.628281;
			spec[1] = 0.555802;
			spec[2] = 0.366065;
			spec[3] = 1;
			glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
			glMaterialfv(GL_FRONT, GL_DIFFUSE, diff);
			glMaterialfv(GL_FRONT, GL_SPECULAR, spec);
			glMaterialf(GL_FRONT, GL_SHININESS, 0.4);
			break;
		case 3:		// Obsidian
			amb[0] = 0.05375;
			amb[1] = 0.05;
			amb[2] = 0.06625;
			amb[3] = 1;
			diff[0] = 0.18275;
			diff[1] = 0.17;
			diff[2] = 0.22525;
			diff[3] = 1;
			spec[0] = 0.332741;
			spec[1] = 0.328634;
			spec[2] = 0.346435;
			spec[3] = 1;
			glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
			glMaterialfv(GL_FRONT, GL_DIFFUSE, diff);
			glMaterialfv(GL_FRONT, GL_SPECULAR, spec);
			glMaterialf(GL_FRONT, GL_SHININESS, 0.3);
			break;
		case 4:		// Silver
			amb[0] = 0.19225;
			amb[1] = 0.19225;
			amb[2] = 0.19225;
			amb[3] = 1;
			diff[0] = 0.50754;
			diff[1] = 0.50754;
			diff[2] = 0.50754;
			diff[3] = 1;
			spec[0] = 0.508274;
			spec[1] = 0.508274;
			spec[2] = 0.508274;
			spec[3] = 1;
			glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
			glMaterialfv(GL_FRONT, GL_DIFFUSE, diff);
			glMaterialfv(GL_FRONT, GL_SPECULAR, spec);
			glMaterialf(GL_FRONT, GL_SHININESS, 0.4);
			break;
		case 5:		// cyan plastic
			amb[0] = 0;
			amb[1] = 0.1;
			amb[2] = 0.06;
			amb[3] = 1;
			diff[0] = 0;
			diff[1] = 0.50980392;
			diff[2] = 0.50980392;
			diff[3] = 1;
			spec[0] = 0.50196078;
			spec[1] = 0.50196078;
			spec[2] = 0.50196078;
			spec[3] = 1;
			glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
			glMaterialfv(GL_FRONT, GL_DIFFUSE, diff);
			glMaterialfv(GL_FRONT, GL_SPECULAR, spec);
			glMaterialf(GL_FRONT, GL_SHININESS, 0.25);
			break;
	}
}

// Draws the objects
// Deals with the glTranslate etc. 
void drawObjects() {
	// Loop through each
	for (int i=0; i<length; ++i) {
		glPushMatrix();
		glPushAttrib(GL_LIGHTING_BIT);
		glEnable(GL_COLOR_MATERIAL);

		Structure::point3D pos = objects[i].getPosition();
		glTranslatef(pos.x, pos.y, pos.z);

		Structure::point3D rot = objects[i].getRotation();
		glRotatef(rot.x, 1, 0, 0);
		glRotatef(rot.y, 0, 1, 0);
		glRotatef(rot.z, 0, 0, 1);

		Structure::point3D sca = objects[i].getScale();
		glScalef(sca.x, sca.y, sca.z);

    	setMaterial(objects[i].getMaterial());
    	renderObject(objects[i].getType());
		glPopAttrib();
		glPopMatrix();
    }
}

// Draws the box around the selected object
void drawBox() {
	Structure::point3D posi = objects[selected].getPosition();
	Structure::point3D scal = objects[selected].getScale();

	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glLineWidth(2);
	glColor3f(0, 0, 0);
	scal.x *= 1.5;
	scal.y *= 1.5;
	scal.z *= 1.5;
	glBegin(GL_LINES);
		//vertical lines
		glVertex3f(posi.x - scal.x, posi.y + scal.y, posi.z + scal.z);
		glVertex3f(posi.x - scal.x, posi.y - scal.y, posi.z + scal.z);
		glVertex3f(posi.x + scal.x, posi.y - scal.y, posi.z - scal.z);
		glVertex3f(posi.x + scal.x, posi.y + scal.y, posi.z - scal.z);
		glVertex3f(posi.x + scal.x, posi.y - scal.y, posi.z + scal.z);
		glVertex3f(posi.x + scal.x, posi.y + scal.y, posi.z + scal.z);
		glVertex3f(posi.x - scal.x, posi.y - scal.y, posi.z - scal.z);
		glVertex3f(posi.x - scal.x, posi.y + scal.y, posi.z - scal.z);

		//horizontal lines (top)
		glVertex3f(posi.x - scal.x, posi.y + scal.y, posi.z + scal.z);
		glVertex3f(posi.x + scal.x, posi.y + scal.y, posi.z + scal.z);
		glVertex3f(posi.x + scal.x, posi.y + scal.y, posi.z + scal.z);
		glVertex3f(posi.x + scal.x, posi.y + scal.y, posi.z - scal.z);
		glVertex3f(posi.x + scal.x, posi.y + scal.y, posi.z - scal.z);
		glVertex3f(posi.x - scal.x, posi.y + scal.y, posi.z - scal.z);
		glVertex3f(posi.x - scal.x, posi.y + scal.y, posi.z - scal.z);
		glVertex3f(posi.x - scal.x, posi.y + scal.y, posi.z + scal.z);

		//horizontal lines (bottom)
		glVertex3f(posi.x - scal.x, posi.y - scal.y, posi.z + scal.z);
		glVertex3f(posi.x + scal.x, posi.y - scal.y, posi.z + scal.z);
		glVertex3f(posi.x + scal.x, posi.y - scal.y, posi.z + scal.z);
		glVertex3f(posi.x + scal.x, posi.y - scal.y, posi.z - scal.z);
		glVertex3f(posi.x + scal.x, posi.y - scal.y, posi.z - scal.z);
		glVertex3f(posi.x - scal.x, posi.y - scal.y, posi.z - scal.z);
		glVertex3f(posi.x - scal.x, posi.y - scal.y, posi.z - scal.z);
		glVertex3f(posi.x - scal.x, posi.y - scal.y, posi.z + scal.z);
	glEnd();
	glDisable(GL_COLOR_MATERIAL);
}

// Draws any text passed to it
void drawText(string s) {
	void * font = GLUT_BITMAP_HELVETICA_18;
	for (string::iterator i = s.begin(); i != s.end(); ++i) {
	    char c = *i;
	    glutBitmapCharacter(font, c);
    }
}
 
// Get the name of a material
string getMatName(int mat) {
	switch(mat) {
		case -1:	// empty
			return " ";
			break;
		case 1:		// red rubber
			return "Red Rubber";
			break;
		case 2:		// Gold
			return "Gold";
			break;
		case 3:		// Obsidian
			return "Obsidian";
			break;
		case 4:		// Silver
			return "Silver";
			break;
		case 5:		// cyan plastic
			return "Cyan Plastic";
			break;
	}
}

// Got started with: http://stackoverflow.com/questions/5467218/opengl-2d-hud-over-3d
// Displays an head up display that tells the user info about the scene
void HUD() {
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);
		glLoadIdentity();
		gluOrtho2D(0, WIDTH, 50 - HEIGHT, 50);
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
			glLoadIdentity();
			glDisable(GL_CULL_FACE);

			glClear(GL_DEPTH_BUFFER_BIT);
			glColor3f(0, 0, 0);

			Structure::point3D temp;

			char buf1[5];
			char buf2[5];
			char buf3[5];

			string s[8];
		    s[0] = "Current Shape: " + (selected > -1 ? objects[selected].getType() : " ");
		    int mat = selected > -1 ? objects[selected].getMaterial() : -1;
		    string matName = getMatName(mat);
		    s[1] = "Current Material: " + matName;
		    if (selected > -1) {
			    s[2] = "Position";

			    temp = objects[selected].getPosition();
			    snprintf(buf1, sizeof(buf1), "%f", temp.x);	// float to char array
			    snprintf(buf2, sizeof(buf2), "%f", temp.y);
			    snprintf(buf3, sizeof(buf3), "%f", temp.z);
			    string posPos1(buf1);						// char array to float
			    string posPos2(buf2);
			    string posPos3(buf3);
			    s[3] = "x: " + posPos1 + " y: " + posPos2 + " z: " + posPos3;
			    s[4] = "Rotation";

			    temp = objects[selected].getRotation();
			    snprintf(buf1, sizeof(buf1), "%f", temp.x); // float to char array
			    snprintf(buf2, sizeof(buf2), "%f", temp.y);
			    snprintf(buf3, sizeof(buf3), "%f", temp.z);
			    string rotPos1(buf1);				// char array to float
			    string rotPos2(buf2);
			    string rotPos3(buf3);
			   	s[5] = "x: " + rotPos1 + " y: " + rotPos2 + " z: " + rotPos3;
			    s[6] = "Scale";

			    temp = objects[selected].getScale();
			    snprintf(buf1, sizeof(buf1), "%f", temp.x);	// float to char array
			    snprintf(buf2, sizeof(buf2), "%f", temp.y);
			    snprintf(buf3, sizeof(buf3), "%f", temp.z);
			    string scalPos1(buf1);					// char array to float
			    string scalPos2(buf2);
			    string scalPos3(buf3);
			    string sTemp3(buf3);
			    s[7] = "x: " + scalPos1 + " y: " + scalPos2 + " z: " + scalPos3;
			}

			// Render each string
			int v = (selected > -1 ? sizeof(s)/24 : 2); // number of strings to draw
		    for(int i = 0; i < v; i++) {
		    	glRasterPos2i(10, ((-i * 20) + 30));
		  		drawText(s[i]);
		 	}

		  	// Making sure we can render 3D again
			glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
		glMatrixMode(GL_PROJECTION);
	glDisable(GL_COLOR_MATERIAL);
	glPopMatrix();
}

// display callback which displays everything to your beautiful screen
void display(void) {
	glClearColor(95.0/255, 195.0/255, 240.0/255, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// internal camera rotation
	glRotatef(rot[0], 1, 0, 0);
	glRotatef(rot[1], 0, 1, 0);
	glRotatef(rot[2], 0, 0, 1);

	gluLookAt(eye[0], eye[1], eye[2], center[0] + lx, center[1], center[2] + lz, 0, 1, 0);
	// change the lights position
	float pos1[] = {light1.x, light1.y, light1.z, 1};
	float pos2[] = {light2.x, light2.y, light2.z, 1};
	glLightfv(GL_LIGHT0, GL_POSITION, pos1);
	glLightfv(GL_LIGHT1, GL_POSITION, pos2);
	// draw stuff
	drawGround();
	drawObjects();
	if (selected > -1) {
		drawBox();
	}
	HUD();
	glutSwapBuffers();
}

// clear the screen
void reset() {
	for (int i=0; i<length; ++i) {
		objects[i].clear();
    }
    length = 0;
    selected = -1;
}

// Reset the camera to the initial position
void resetCamera() {
	eye[0] = -4;
	eye[1] = 4;
	eye[2] = -4;

	center[0] = 0;
	center[1] = 0;
	center[2] = 0;

	rot[0] = 0;
	rot[1] = 1;
	rot[2] = 0;

	lx = 0;
	lz = 0;
}

// add an object to the scene
void addObject(string type) {
	if (length < 100) {
		objects[length].setType(type);
		objects[length].setScale(1, 1, 1);	// So you can see it
		objects[length].setMaterial(materialDef);	// So you can see it
		selected = length; 	// selects object after creation
		++length;	// increase the number of objects counter
	}
}

// Loop through the objects and write their point3D info
void loopWrite(int i, int get) {
	char buf[5];
	Structure::point3D toWrite;
	// get the points
	switch(get) {
		case 0:
			toWrite = objects[i].getPosition();
			break;
		case 1:
			toWrite = objects[i].getRotation();
			break;
		case 2:
			toWrite = objects[i].getScale();
			break;
	}
	// write the points
 	snprintf(buf, sizeof(buf), "%f", toWrite.x);
    fputs(buf, fp);
    fputs("\n", fp);
    snprintf(buf, sizeof(buf), "%f", toWrite.y);
    fputs(buf, fp);
    fputs("\n", fp);
    snprintf(buf, sizeof(buf), "%f", toWrite.z);
    fputs(buf, fp);
    fputs("\n", fp);
}

// Save the scene to a text file
void save() {
	fp = fopen(filename, "w+");
	char printLen[2];
 	snprintf(printLen, sizeof(printLen), "%d", length);
    fputs(printLen, fp);
    fputs("\n", fp);
    // Write the type
    for (int i = 0; i < length; ++i) {
    	string toWrite = objects[i].getType();
    	int n = sizeof(toWrite);
    	char buf[n];
    	for (int i = 0; i < n; ++i) {
    		buf[i] = toWrite[i];
    	}
		fputs(buf, fp);
		fputs("\n", fp);
    }
    // Write the object position
    for (int i = 0; i < length; ++i) {
    	loopWrite(i, 0);
    }
    // Write the object rotation
    for (int i = 0; i < length; ++i) {
    	loopWrite(i, 1);
    }
    // Write the object scale
    for (int i = 0; i < length; ++i) {
    	loopWrite(i, 2);
    }
    // save the material number
    for (int i = 0; i < length; ++i) {
	    snprintf(printLen, sizeof(printLen), "%d", objects[i].getMaterial());
	    fputs(printLen, fp);
	    fputs("\n", fp);
	}
    fclose(fp);
}

// Loop through the objects and write their point3D info
void loopRead(int i, int get) {
	char buf[5];
	Structure::point3D toRead;
	// read the points
	fscanf(fp, "%s", buf);
	sscanf(buf, "%f", &toRead.x);
	fscanf(fp, "%s", buf);
	sscanf(buf, "%f", &toRead.y);
	fscanf(fp, "%s", buf);
	sscanf(buf, "%f", &toRead.z);
	// get the points
	switch(get) {
		case 0:
			objects[i].setPosition(toRead.x, toRead.y, toRead.z);
			break;
		case 1:
			objects[i].setRotation(toRead.x, toRead.y, toRead.z);
			break;
		case 2:
			objects[i].setScale(toRead.x, toRead.y, toRead.z);
			break;
	}
}

// Load the scene from a textfile
void load() {
	fp = fopen(filename, "r");
	char buf[1];
	fscanf(fp, "%s", buf);
	sscanf(buf, "%d", &length);
	// Read the type
    for (int i = 0; i < length; ++i) {
    	char buf[20];
    	fscanf(fp, "%s", buf);
    	string toRead(buf);
    	objects[i].setType(toRead);
    }
	// Read the object position
    for (int i = 0; i < length; ++i) {
    	loopRead(i, 0);
    }
    // Read the object rotation
    for (int i = 0; i < length; ++i) {
    	loopRead(i, 1);
    }
    // Read the object scale
    for (int i = 0; i < length; ++i) {
    	loopRead(i, 2);
    } 
    // Read the object material
    for (int i = 0; i < length; ++i) {
    	fscanf(fp, "%s", buf);
    	int mat;
		sscanf(buf, "%d", &mat);
		objects[i].setMaterial(mat);
    }
	fclose(fp);
}

// Move the object with the keyboard, order of x then y then z
void moveObject(unsigned char key) {
	Structure::point3D posi;
	switch(key) {
		case 'i':
			posi = objects[selected].getPosition();
			if(posi.x <= 50)
				objects[selected].setPosition(posi.x + 0.3, posi.y, posi.z);
			break;
		case 'I':
			posi = objects[selected].getPosition();
			if(posi.x >= -50)
				objects[selected].setPosition(posi.x - 0.3, posi.y, posi.z);
			break;
		case 'o':
			posi = objects[selected].getPosition();
			if(posi.y <= 50)
				objects[selected].setPosition(posi.x, posi.y + 0.3, posi.z);
			break;
		case 'O':
			posi = objects[selected].getPosition();
			if(posi.y > 0)
				objects[selected].setPosition(posi.x, posi.y - 0.3, posi.z);
			break;
		case 'p':
			posi = objects[selected].getPosition();
			if(posi.z <= 50)
				objects[selected].setPosition(posi.x, posi.y, posi.z + 0.3);
			break;
		case 'P':
			posi = objects[selected].getPosition();
			if(posi.z >= -50)
				objects[selected].setPosition(posi.x, posi.y, posi.z - 0.3);
			break;
	}
}

// Rotate the object with the keyboard, order of x then y then z
void rotateObject(unsigned char key) {
	Structure::point3D rota;
	switch(key) {
		case 'h':
			rota = objects[selected].getRotation();
			objects[selected].setRotation(rota.x + 4.0, rota.y, rota.z);
			break;
		case 'H':
			rota = objects[selected].getRotation();
			objects[selected].setRotation(rota.x - 4.0, rota.y, rota.z);
			break;
		case 'j':
			rota = objects[selected].getRotation();
			objects[selected].setRotation(rota.x, rota.y + 4.0, rota.z);
			break;
		case 'J':
			rota = objects[selected].getRotation();
			objects[selected].setRotation(rota.x, rota.y - 4.0, rota.z);
			break;
		case 'k':
			rota = objects[selected].getRotation();
			objects[selected].setRotation(rota.x, rota.y, rota.z + 4.0);
			break;
		case 'K':
			rota = objects[selected].getRotation();
			objects[selected].setRotation(rota.x, rota.y, rota.z - 4.0);
			break;
	}
}

// Scale the object with the keyboard, order of x then y then z
void scaleObject(unsigned char key) {
	Structure::point3D scal;
	switch(key) {
		case 't':
			scal = objects[selected].getScale();
			objects[selected].setScale(scal.x + 0.5, scal.y, scal.z);
			break;
		case 'T':
			scal = objects[selected].getScale();
			if (scal.x > 0)
				objects[selected].setScale(scal.x - 0.5, scal.y, scal.z);
			break;
		case 'y':
			scal = objects[selected].getScale();
			objects[selected].setScale(scal.x, scal.y + 0.5, scal.z);
			break;
		case 'Y':
			scal = objects[selected].getScale();
			if (scal.y > 0)
				objects[selected].setScale(scal.x, scal.y - 0.5, scal.z);
			break;
		case 'u':
			scal = objects[selected].getScale();
			objects[selected].setScale(scal.x, scal.y, scal.z + 0.5);
			break;
		case 'U':
			scal = objects[selected].getScale();
			if (scal.z > 0)
				objects[selected].setScale(scal.x, scal.y, scal.z - 0.5);
			break;
	}
}

// rotate camera around its internal xyz axis
void internalCamera(unsigned char key) {
	float change;	// postive or negative
	switch (key) {
		case 'd':	// rotate positively on the internal x-axis
		case 'D':	// rotate negatively on the internal x-axis
			change = key == 'd' ? 1 : -1;					  // postive or negative
			rot[0] = rot[0] < 360 ? rot[0] + change * 4 : 0;  // increment or reset to 0

			break;
		case 'f':	// rotate positively on the internal y-axis
		case 'F':	// rotate negatively on the internal y-axis
			change = key == 'f' ? 1 : -1;					  // postive or negative
			rot[1] = rot[1] < 360 ? rot[1] + change * 4 : 0;  // increment or reset to 0

			break;
		case 'g':	// rotate positively on the internal z-axis
		case 'G':	// rotate negatively on the internal z-axis
			change = key == 'g' ? 1 : -1;					  // postive or negative
			rot[2] = rot[2] < 360 ? rot[2] + change * 4 : 0;  // increment or reset to 0
			break;
	}
}

// Move the object with the keyboard, order of x then y then z
void moveLights(unsigned char key) {
	int mod =2;
	switch(key) {
		case '6':
			light1.x +=5;
			break;
		case '^':
			light1.x -= 5;
			break;
		case '7':
			light1.y += 5;
			break;
		case '&':
			light1.y -=5;
			break;
		case '8':
			light1.z +=5;
			break;
		case '*':
			light1.z -= 5;
			break;
		case '9':
			light2.x += 5;
			break;
		case '(':
			light2.x -=5;
			break;
		case '0':
			light2.y +=5;
			break;
		case ')':
			light2.y -= 5;
			break;
		case '-':
			light2.z += 5;
			break;
		case '_':
			light2.z -=5;
			break;
	}
}

// Do certain actions from the keyboard's input. Async
void keyboard(unsigned char key, int xIn, int yIn) {
	switch (key) {
		case 'q':	// quit
		case 'Q':
		case 27:
			exit(0);
			break;
		case 'z':	// cube
		case 'Z':
			addObject("Cube");
			break;
		case 'x':	// sphere
		case 'X':
			addObject("Sphere");
			break;
		case 'c':	// octahedron
		case 'C':
			addObject("Octahedron");
			break;
		case 'v':	// dodecahedron
		case 'V':
			addObject("Dodecahedron");
			break;
		case 'b':	// cylinder
		case 'B':
			addObject("Tetrahedron");
			break;
		case 'r':	// reset
			reset();
			break;
		case 'R':
			resetCamera();
			break;
		case 's':
		case 'S':
			save();
			break;
		case 'l':
		case 'L':
			load();
			break;
		case 'm':
		case 'M':
			calcIntersections(false, true, xIn, yIn);
			break;
		case '1':
			materialDef = 1;
			break;
		case '2':
			materialDef = 2;
			break;
		case '3':
			materialDef = 3;
			break;
		case '4':
			materialDef = 4;
			break;
		case '5':
			materialDef = 5;
			break;
		default:
			if (selected > -1) {
				moveObject(key);
				rotateObject(key);
				scaleObject(key);
			}
			moveLights(key);
			internalCamera(key);
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

// Redisplay the screen in 60 fps
void redisplay(int i) {
	glutPostRedisplay();
	glutTimerFunc(17, redisplay, 0);
}

//Initialize the scene
void init() {
	glDepthFunc(GL_LESS);
	glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
	gluPerspective(90, 1, 1, 100);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);	// First lightbulb
	glEnable(GL_LIGHT1);	// Second lightbulb
	//upload first light data to gpu
 	float pos1[4] = {-40, 40, 0, 1};
 	light1.x = -40;
 	light1.y = 40;
 	light1.z = 0;
 	float lightColor1[4] = {1, 1, 1, 0.2};
 	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor1);
 	glLightfv(GL_LIGHT0, GL_POSITION, pos1);
	//upload second light data to gpu
	float pos2[4] = {0, 40, -40, 1};
	light2.x = 0;
 	light2.y = 40;
 	light2.z = -40;
	float lightColor2[4] = {1, 1, 0, 0.2};
 	glLightfv(GL_LIGHT1, GL_DIFFUSE, lightColor2);
	glLightfv(GL_LIGHT1, GL_POSITION, pos2);

	cout << "Input the filename" << endl;
	scanf("%s", &filename);
}

// Your long list of instructions
void printInstructions() {
	cout << "-----------------------------------"  << endl;
	cout << "INSTRUCITONS: " << endl;
	cout << "KEY        ACTION" << endl;
	cout << "Q          Quit"   << endl;
	cout << "ESC"            << endl;
	cout << "Z          Add a cube"   << endl;
	cout << "X          Add a sphere"   << endl;
	cout << "C          Add a octahedron"   << endl;
	cout << "V          Add a dodecahedron"   << endl;
	cout << "B          Add a tetrahedron"   << endl;
	cout << "R          Reset the scene"   << endl;
	cout << "S          Save the scene"   << endl;
	cout << "L          Load the scene"   << endl;
	cout << "M          Change material of hovered over object"   << endl;
	cout << "1          Red rubber"   << endl;
	cout << "2          Gold"   << endl;
	cout << "3          Obsidian"   << endl;
	cout << "4          Silver"   << endl;
	cout << "5          Cyan plastic"   << endl;
	cout << "Move light 1"   << endl;
	cout << "6          Move x positively"   << endl;
	cout << "^          Move x negatively"   << endl;
	cout << "7          Move y positively"   << endl;
	cout << "&          Move y negatively"   << endl;
	cout << "8          Move z positively"   << endl;
	cout << "*          Move z negatively"   << endl;
	cout << "Move light 2"   << endl;
	cout << "9          Move x positively"   << endl;
	cout << "(          Move x negatively"   << endl;
	cout << "0          Move y positively"   << endl;
	cout << ")          Move y negatively"   << endl;
	cout << "-          Move z positively"   << endl;
	cout << "_          Move z negatively"   << endl;
	cout << "MOVE OBJECT"   << endl;
	cout << "i          Move positively on the x-axis"  << endl;
	cout << "I          Move negatively on the x-axis"  << endl;
	cout << "o          Move positively on the y-axis"  << endl;
	cout << "O          Move negatively on the y-axis"  << endl;
	cout << "p          Move positively on the z-axis"  << endl;
	cout << "P          Move negatively on the z-axis"  << endl;
	cout << "h          Rotate positively on the x-axis"  << endl;
	cout << "H          Rotate negatively on the x-axis"  << endl;
	cout << "j          Rotate positively on the y-axis"  << endl;
	cout << "J          Rotate negatively on the y-axis"  << endl;
	cout << "k          Rotate positively on the z-axis"  << endl;
	cout << "K          Rotate negatively on the z-axis"  << endl;
	cout << "t          Scale positively on the x-axis"  << endl;
	cout << "T          Scale negatively on the x-axis"  << endl;
	cout << "y          Scale positively on the y-axis"  << endl;
	cout << "Y          Scale negatively on the y-axis"  << endl;
	cout << "u          Scale positively on the z-axis"  << endl;
	cout << "U          Scale negatively on the z-axis"  << endl;
	cout << "CAMERA CONTROLS:"  << endl;
	cout << "left       Move camera along the x axis positively"  << endl;
	cout << "LEFT       Move camera along the x axis positively"  << endl;
	cout << "right      Move camera along the z axis positively"  << endl;
	cout << "RIGHT      Move camera along the z axis positively"  << endl;
	cout << "UP         Move camera upwards"  << endl;
	cout << "DOWN       Move camera downwards"  << endl;
	cout << "d          Rotate positively on the internal x-axis"  << endl;
	cout << "D          Rotate negatively on the internal x-axis"  << endl;
	cout << "f          Rotate positively on the internal y-axis"  << endl;
	cout << "F          Rotate negatively on the internal y-axis"  << endl;
	cout << "g          Rotate positively on the internal z-axis"  << endl;
	cout << "G          Rotate negatively on the internal z-axis"  << endl;
	cout << "-----------------------------------"  << endl;
}

int main(int argc, char** argv) {
	printInstructions();
	glutInit(&argc, argv);		//starts up GLUT
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutInitWindowPosition(200, 200);
	glutCreateWindow("Modeller");	//creates the window

	init();
	glutDisplayFunc(display);	//registers "display" as the display callback function
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(special);
	glutMouseFunc(mouse);
	glutMotionFunc(mouseMove);  //registers "mouseMove" as the motion callback function.
	redisplay(0);
	glutMainLoop();				//starts the event loop

	return(0);					//return may not be necessary on all compilers
}