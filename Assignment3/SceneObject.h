#ifndef SCENEOBJECT_H
#define SCENEOBJECT_H

#include "Structure.h"

#include <iostream>
using namespace std;

class SceneObject {

int material;
string type;
Structure::point3D position;	// The position of the object
Structure::point3D rotation;	// each value is the rotation angle about the 3 axises
Structure::point3D scale;		// each value is the scale angle about the 3 axises

public:
	void clear();
	Structure::point3D getPosition();
	void setPosition(float x, float y, float z);
	Structure::point3D getRotation();
	void setRotation(float x, float y, float z);
	Structure::point3D getScale();
	void setScale(float x, float y, float z);
	string getType();
	void setType(string change);
	int getMaterial();
	void setMaterial(int mat);
};

#endif