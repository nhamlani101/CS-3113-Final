#ifndef VECTOR_H
#define VECTOR_H

class Vector{
public:
	Vector();
	Vector(float x, float y, float z);

	Vector operator * (float scalar);
	Vector operator + (const Vector& v2);

	float x;
	float y;
	float z;

};

#endif