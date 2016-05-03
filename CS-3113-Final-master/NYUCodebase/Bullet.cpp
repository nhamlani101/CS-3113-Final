#include "Bullet.h"

void Bullet::Update(float elapsed) {
	program->setModelMatrix(modelMatrix);
	modelMatrix.identity();
	modelMatrix.Translate(x_pos, y_pos, 0.0f);
	//std::cout << x_pos << std::endl;
	if (alive) {
		x_pos += elapsed * move_speed * direction;
		sprite.Draw();
	}
	//if ((direction == 1 && x_pos > 1.5f)) alive = false;
}