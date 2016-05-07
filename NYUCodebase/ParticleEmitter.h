#include "Vector.h"
#include "Particle.h"
#include "ShaderProgram.h"
#include <vector>

using namespace std;

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