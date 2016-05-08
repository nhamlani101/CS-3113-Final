#include "ParticleEmitter.h"

void ParticleEmitter::Render(ShaderProgram* program) {

	vector<float> particleVertices;

	for (int i = 0; i < particles.size(); i++) {
		particleVertices.push_back(particles[i].position.x);
		particleVertices.push_back(particles[i].position.y);
	}

	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, particleVertices.data());
	glEnableVertexAttribArray(program->positionAttribute);
	glDrawArrays(GL_POINTS, 0, particleVertices.size() / 2);

}
void ParticleEmitter::Update(float elapsed) {
	for (unsigned int i = 0; i < particles.size(); i++){
		particles[i].lifetime += elapsed;
		particles[i].position.x += particles[i].velocity.x*elapsed;
		particles[i].position.y += particles[i].velocity.y*elapsed;
	}
}