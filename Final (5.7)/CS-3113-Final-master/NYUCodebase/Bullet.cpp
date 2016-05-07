#include "Bullet.h"

void Bullet::Update(float elapsed) {
	program->setModelMatrix(modelMatrix);
	modelMatrix.identity();
	modelMatrix.Translate(x_pos, y_pos, 0.0f);
	if (alive){
		x_pos += elapsed * move_speed * direction;
		sprite.Draw();
	}
}