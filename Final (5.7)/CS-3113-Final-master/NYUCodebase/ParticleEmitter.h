#include "Vector.h"
#include "Particle.h"
#include <vector>

using namespace std;

class ParticleEmitter {
public:
	ParticleEmitter(unsigned int particleCount);
	ParticleEmitter();
	~ParticleEmitter();
	void Update(float elapsed);
	void Render();
	Vector position;
	Vector gravity;
	Vector velocity;
	Vector velocityDev;
	//Color startColor;
	//Color endColor;
	float maxLifetime;
	vector<Particle> particles;
};