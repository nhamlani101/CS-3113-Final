#ifndef BULLET_H
#define BULLET_H

#include "ShaderProgram.h"
#include "SheetSprite.h"

class Bullet {
public:
	Bullet(ShaderProgram* program, SheetSprite sprite) : program(program), sprite(sprite), x_pos(), y_pos(), move_speed(1.5),
		direction(1.0f), alive(false) {};
	void Update(float elapsed);
	SheetSprite sprite;
	float x_pos;
	float y_pos;
	float move_speed;
	float direction;
	bool alive;
	Matrix modelMatrix;
	ShaderProgram* program;
};

#endif