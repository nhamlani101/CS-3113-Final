#include "ParticleEmitter.h"


using namespace std;
Particle::Particle()
{
	position = Vector(0.0, 0.0, 0.0);
	velocity = Vector(1.0, 1.0, 0.0);
	lifetime = 5.0;
	sizeDeviation = 0.1;
}

ParticleEmitter::ParticleEmitter()
{
	particleSize = 0.1;
	decayRate = 0.5;
	startSize = 1.0;
	endSize = 0.0;
	sizeDeviation = 0.1;
	minLifeTime = 0.0;
	maxLifeTime = 5.0;
	gravity = Vector(0.0, -9.81, 0.0);
	velocity = Vector(0.0, 0.0, 0.0);
	velocityDeviation = Vector(0.5, 0.5, 0.0);
	startColor = Color(1.0, 0.0, 0.0, 1.0);
	endColor = Color(0.0, 1.0, 0.0, 1.0);
}
ParticleEmitter::ParticleEmitter(unsigned int particleCount) {

	particles.resize(particleCount);
	particleSize = 0.1;
	decayRate = 0.5;
	startSize = 0.5;
	endSize = 0.0;
	sizeDeviation = 0.1;
	for (int i = 0; i < particles.size(); i++) {
		particles[i].lifetime = 4.0f;
	}
	minLifeTime = 0.0;
	maxLifeTime = 5.0;
	gravity = Vector(0.0, -9.81, 0.0);
	velocity = Vector(0.0, 0.0, 0.0);
	velocityDeviation = Vector(0.5, 0.5, 0.0);
	startColor = Color(1.0, 1.0, 0.0, 1.0);
	endColor = Color(0.0, 1.0, 0.0, 1.0);

}

void ParticleEmitter::SetTex(const char* path) {
	texture = LoadTexture(path);
}

void ParticleEmitter::Render(ShaderProgram* program)
{
	for (int i = 0; i < particles.size(); i++) {
		m = (particles[i].lifetime / maxLifeTime);
		float size = lerp(startSize, endSize, m) + particles[i].sizeDeviation;

		vertices.insert(vertices.end(), {
			particles[i].position.x - size, particles[i].position.y + size,
			particles[i].position.x - size, particles[i].position.y - size,
			particles[i].position.x + size, particles[i].position.y + size,

			particles[i].position.x + size, particles[i].position.y + size,
			particles[i].position.x - size, particles[i].position.y - size,
			particles[i].position.x + size, particles[i].position.y - size
		});

		texCoords.insert(texCoords.end(), {
			0.0f, 0.0f,
			0.0f, 1.0f,
			1.0f, 0.0f,

			1.0f, 0.0f,
			0.0f, 1.0f,
			1.0f, 1.0f
		});

		for (int j = 0; j < 6; j++) {
			colors.push_back(lerp(startColor.r, endColor.r, m));
			colors.push_back(lerp(startColor.g, endColor.g, m));
			colors.push_back(lerp(startColor.b, endColor.b, m));
			colors.push_back(lerp(startColor.a, endColor.a, m));
		}
		//vertices.push_back(particles[i].position.first);
		//vertices.push_back(particles[i].position.second);
		/*vertices.push_back(particles[i].position.x);
		vertices.push_back(particles[i].position.y);*/
	}

	//GLuint colorAttribute = glGetAttribLocation(program->programID, "color");
	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices.data());
	glEnableVertexAttribArray(program->positionAttribute);
	//glVertexAttribPointer(colorAttribute, 4, GL_FLOAT, false, 0, colors.data());
	//glEnableVertexAttribArray(colorAttribute);
	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords.data());
	glEnableVertexAttribArray(program->texCoordAttribute);

	glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 2);



}

void ParticleEmitter::EmitXDirection(int number, bool toTheRight)
{
	for (int i = 0; i < number; i++) {
		particleSize = 12.0f;
		particles[i].velocity = velocity;
		particles[i].position.y = position.y;
		if (!toTheRight) {
			particles[i].position.x = position.x + 0.05f;
			particles[i].velocity.x = -velocityDeviation.x * ((float)rand() / (float)RAND_MAX);
			particles[i].velocity.y = velocityDeviation.y * ((float)rand() / (float)RAND_MAX);
		}
		else {
			particles[i].position.x = position.x - 0.05f;
			particles[i].velocity.x = velocityDeviation.x * ((float)rand() / (float)RAND_MAX);
			particles[i].velocity.y = velocityDeviation.y * ((float)rand() / (float)RAND_MAX);
		}

	}
}

void ParticleEmitter::EmitYDirection(int number, bool toTheBottom)
{
	for (int i = 0; i < number; i++) {
		particleSize = 12.0f;
		particles[i].velocity = velocity;
		particles[i].position = position;
		if (!toTheBottom) {
			particles[i].velocity.y = -velocityDeviation.y * ((float)rand() / (float)RAND_MAX);
			particles[i].velocity.x = velocityDeviation.x * ((float)rand() / (float)RAND_MAX) - 0.5f;
		}
		else {
			particles[i].velocity.y = velocityDeviation.y * ((float)rand() / (float)RAND_MAX);
			particles[i].velocity.x = velocityDeviation.x * ((float)rand() / (float)RAND_MAX) - 0.5f;
		}
	}

}

void ParticleEmitter::EmitYDirectionWithOffsetX(int number, bool toTheRight, bool toTheBotton, float offset)
{
	for (int i = 0; i < number; i++) {
		particleSize = 12.0f;
		particles[i].velocity = velocity;
		particles[i].position.y = position.y;
		if (!toTheRight) {
			particles[i].position.x = position.x + offset;
		}
		else {
			particles[i].position.x = position.x - offset;
		}
		if (!toTheBotton) {
			particles[i].velocity.y = -velocityDeviation.y * ((float)rand() / (float)RAND_MAX);
			particles[i].velocity.x = velocityDeviation.x * ((float)rand() / (float)RAND_MAX) - 0.5f;
		}
		else {
			particles[i].velocity.y = velocityDeviation.y * ((float)rand() / (float)RAND_MAX);
			particles[i].velocity.x = velocityDeviation.x * ((float)rand() / (float)RAND_MAX) - 0.5f;
		}
	}
}

void ParticleEmitter::Update(float elapsed) {
	for (int i = 0; i < particles.size(); i++) {
		particles[i].velocity.x += (gravity.x * elapsed);
		particles[i].position.x += (particles[i].velocity.x * elapsed);
		particles[i].velocity.y += (gravity.y * elapsed);
		particles[i].position.y += (particles[i].velocity.y * elapsed);
	}

	particleSize -= decayRate;

}

float ParticleEmitter::lerp(float start, float end, float time)
{
	return (1.0 - time) * start + time * end;
}

GLuint ParticleEmitter::LoadTexture(const char* image_path) {

	SDL_Surface *surface = IMG_Load(image_path);

	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_RGBA,
		GL_UNSIGNED_BYTE, surface->pixels); //USE GL_RGB/A FOR WINDOWS, GL_BGR/A FOR MAC (.PNG files use RGBA, .JPG uses RGB)

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);//MUST USE THIS
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);//MUST USE THIS

	SDL_FreeSurface(surface);
	return textureID;


}
