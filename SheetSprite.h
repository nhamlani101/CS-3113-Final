#ifdef _WINDOWS
#include <GL/glew.h>
#endif


#ifndef SPRITESHEET_H
#define SPRITESHEET_H

#include <SDL.h>
#include "ShaderProgram.h"
#include <SDL_opengl.h>
#include <SDL_image.h>

class SheetSprite {
public:
	SheetSprite(ShaderProgram* program, unsigned int textureID, float u, float v, float width, float height, float 
		size) :program(program), textureID(textureID), u(u), v(v), width(width), height(height), size(size) {}
	void Draw();
	ShaderProgram* program;
	float size;
	unsigned int textureID;
	float u;
	float v;
	float width;
	float height;
};

#endif