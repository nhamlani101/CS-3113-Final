#include "Bullet.h"

void Bullet::Update(float elapsed) {
	program->setModelMatrix(modelMatrix);
	modelMatrix.identity();
	modelMatrix.Translate(x_pos, y_pos, 0.0f);
	if (alive) {
		y_pos += elapsed * move_speed * direction;
		sprite.Draw();
	}
	if ((direction == 1 && y_pos > 1.5f) || (direction == -1 && y_pos < -1.5f)) alive = false;
}