#ifndef VECTOR_H
#define VECTOR_H

class Vector{
public:
	Vector();
	Vector(float x, float y, float z) : x(x), y(y), z(z) {}

	float x;
	float y;
	float z;

};

#endif