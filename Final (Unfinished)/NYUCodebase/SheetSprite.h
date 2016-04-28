//#pragma once

#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "ShaderProgram.h"

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif
class SheetSprite{
public:
	//SheetSprite();
	SheetSprite(ShaderProgram* program, unsigned int textureID, float u, float v, float width, float height, float size) :
		program(program), textureID(textureID), u(u), v(v), width(width), height(height), size(size) {}
	void Draw();
	
	ShaderProgram* program;
	float size;
	unsigned int textureID;
	float u;
	float v;
	float width;
	float height;
};

