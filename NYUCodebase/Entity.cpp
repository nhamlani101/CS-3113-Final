#include "Entity.h"

float Entity::lerp(float v0, float v1, float t) {
	return (1.0 - t)*v0 + t*v1;
}

void Entity::Update(float elapsed){
	//If players
	if (cameFrom == 0) {
		collidedTop = false;
		collidedBottom = false;
		collidedLeft = false;
		collidedRight = false;
		bulletMoveLeft = false;

		velocity_y += gravity * elapsed;
		velocity_x = lerp(velocity_x, 0.0f, elapsed * friction_x);
		velocity_x += acceleration_x * elapsed;

		x += velocity_x * elapsed;
		y += velocity_y * elapsed;
		acceleration_x = 0;
	}

	//if bullet
	else {
		bulletLifetime++;
		if (bulletLifetime == 300) {
			bulletDead = true;
		}
		if (bulletMoveLeft) {
			velocity_x -= acceleration_x * elapsed;
			x -= velocity_x * elapsed;
			
		}
		else {
			velocity_x += acceleration_x * elapsed;
			x += velocity_x * elapsed;
		}
	}
}

void Entity::Render(ShaderProgram* program){
	if (!bulletDead) {
		modMatrix.identity();
		modMatrix.Translate(x, y, 0);

		if (velocity_x < 0 && cameFrom == 0) {
			modMatrix.Scale(-1.0, 1.0, 1.0);
		}

		program->setModelMatrix(modMatrix);
		sprite->Draw();
	}
}

	