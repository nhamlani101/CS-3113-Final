#include "Vector.h"
#include "ShaderProgram.h"
#include <vector>

using namespace std;

#ifndef PARTICLE
#define PARTICLE
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

class ParticleEmitter {
public:
	ParticleEmitter(unsigned int particleCount) : particles(vector<Particle>(particleCount)) {}
	ParticleEmitter();
	~ParticleEmitter();

	void Update(float elapsed);
	void Render(ShaderProgram* program);

	float maxLifetime;
	vector<Particle> particles;
};
