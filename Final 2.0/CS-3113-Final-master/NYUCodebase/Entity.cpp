#include "Entity.h"
//Entity::Entity(){}

float Entity::lerp(float v0, float v1, float t) {
	return (1.0 - t)*v0 + t*v1;
}

void Entity::Update(float elapsed){
	collidedTop = false;
	collidedBottom = false;
	collidedLeft = false;
	collidedRight = false;

	velocity_y += gravity * elapsed;
	velocity_x = lerp(velocity_x, 0.0f, elapsed * friction_x);
	velocity_x += acceleration_x * elapsed;
	
	x += velocity_x * elapsed;
	y += velocity_y * elapsed;
	acceleration_x = 0;

	
	//if (bullet.alive) {
		bullet.Update(elapsed);
	
}

void Entity::Render(ShaderProgram* program){
	modMatrix.identity();
	modMatrix.Translate(x, y, 0);
	//modMatrix.Scale(1.0, sin(1.0), 1.0);

	if (velocity_x < 0) {
		modMatrix.Scale(-1.0, 1.0, 1.0);
	}
	
	program->setModelMatrix(modMatrix);
	sprite->Draw();
}

void Entity::shootBullet() {
	if (!bullet.alive){
		bullet.x_pos = x + width;
		bullet.y_pos = y + height;
		bullet.direction = 1.0f;
		bullet.alive = true;
		bullet.sprite.Draw();
	}
}
