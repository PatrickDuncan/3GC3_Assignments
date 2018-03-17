#include "SceneObject.h"
#include <stdio.h>
#include <stdlib.h>

#include <iostream>
using namespace std;

void SceneObject::clear() {
	type = "";

	position.x = 0;
	position.y = 0;
	position.z = 0;

	rotation.x = 0;
	rotation.y = 0;
	rotation.z = 0;

	scale.x = 0;
	scale.y = 0;
	scale.z = 0;
}

Structure::point3D SceneObject::getPosition() {
	return position;
}

void SceneObject::setPosition(float x, float y, float z) {
	position.x = x;
	position.y = y;
	position.z = z;
}

Structure::point3D SceneObject::getRotation() {
	return rotation;
}

void SceneObject::setRotation(float x, float y, float z) {
	rotation.x = x;
	rotation.y = y;
	rotation.z = z;
}

Structure::point3D SceneObject::getScale() {
	return scale;
}

void SceneObject::setScale(float x, float y, float z) {
	scale.x = x;
	scale.y = y;
	scale.z = z;
}

string SceneObject::getType() {
	return type;
}

void SceneObject::setType(string shape) {
	type = shape;
}

int SceneObject::getMaterial() {
	return material;
}

void SceneObject::setMaterial(int mat) {
	material = mat;
}
