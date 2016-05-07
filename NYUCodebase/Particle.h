#ifndef PARTICLE_H
#define PARTICLE_H

#include "Vector.h"

class Particle {
public:
	Vector position;
	Vector velocity;
	float lifetime;
	Particle() : lifetime(0.0f) {
		velocity = Vector(0.0f, 0.0f, 0.0f);
		position = Vector(0.0f, 0.0f, 0.0f);
	};
	
};

#endif