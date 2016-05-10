#ifndef PARTICLEEMITTER_H
#define PARTICLEEMITTER_H
#include "ShaderProgram.h"
#include "Matrix.h"
#include <map>
#include <vector>
#include "Vector.h"
#include "Color.h"
#include <SDL_image.h>
#include <SDL.h>
#include <SDL_opengl.h>

struct Particle {
	Particle();
	Vector position, velocity;
	float lifetime;
	float sizeDeviation;

};


class ParticleEmitter {

public:
	ParticleEmitter();
	ParticleEmitter(unsigned int ParticleCount);
	void SetTex(const char * path);

	void Update(float elasped);
	void Render(ShaderProgram* program);
	void EmitXDirection(int number, bool toTheRight);
	void EmitYDirection(int number, bool toTheBottom);
	void EmitYDirectionWithOffsetX(int number, bool toTheRight, bool toTheBotton, float offset);

	float lerp(float start, float end, float time);

	GLuint LoadTexture(const char * image_path);
	GLuint texture;
	Color startColor, endColor;
	Vector position, gravity, velocity, velocityDeviation;

	std::vector<Particle> particles;
	std::vector<float> vertices, particleColors, texCoords, colors;

	float minLifeTime, maxLifeTime, particleSize, decayRate, m, startSize, endSize, sizeDeviation;
};
#endif