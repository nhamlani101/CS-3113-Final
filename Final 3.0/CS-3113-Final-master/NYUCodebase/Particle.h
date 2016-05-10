#ifndef PARTICLE_H
#define PARTICLE_H

#include "Vector.h"

class Particle {
public:
	Vector position;
	Vector velocity;
	float lifetime;
};

#endif