#include "SheetSprite.h"
#include "Bullet.h"
#include <vector>
#define FIXED_TIMESTEP 0.03333f
#define MAX_TIMESTEPS 6
#define friction_x 1
#define friction_y 1
#define gravity -7

class Entity{
public:
	Entity();
	Entity(SheetSprite* ss, float x, float y, float width, float height,
		float velocity_x, float velocity_y, float acceleration_x, float acceleration_y, Bullet b) :
		sprite(ss), x(x), y(y), width(width), height(height),
		velocity_x(velocity_x), velocity_y(velocity_y), acceleration_x(acceleration_x), acceleration_y(acceleration_y), 
		bullet(b), wins(0), health(3) {}

	void Render(ShaderProgram* program);
	void Update(float elapsed);
	float lerp(float v0, float v1, float t);
	void shootBullet();

	SheetSprite* sprite;
	float x;
	float y;
	float height;
	float width;
	float velocity_x;
	float velocity_y;
	float acceleration_x;
	float acceleration_y;

	bool collidedTop = false;
	bool collidedBottom = false;
	bool collidedLeft = false;
	bool collidedRight = false;

	Matrix modMatrix;
	Bullet bullet;
	
	int wins;
	int health;
};