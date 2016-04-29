#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include "Entity.h"
#include "Matrix.h"
#include "ShaderProgram.h"

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

using namespace std;

#define TILE_SIZE 1.0f
#define SPRITE_COUNT_X 16
#define SPRITE_COUNT_Y 8 

//Globals
SDL_Window* displayWindow;
SDL_Event event;
bool done;
ShaderProgram* program;
Matrix projectionMatrix;
Matrix modelMatrix;
Matrix viewMatrix;
float lastFrameTicks;

vector<float> vertexData;
vector<float> texCoordData;
int mapWidth;
int mapHeight;
int tileCount;
unsigned char** levelData;
vector<Entity*> ents;
GLuint mapID;
GLuint player1ID;
//GLuint player2ID;
SheetSprite* p1sheet;
Entity* player1;

Mix_Chunk *jumpSound = Mix_LoadWAV("paddle_hit.wav");

GLuint LoadTexture(const char* image_path){
	SDL_Surface* surface = IMG_Load(image_path);

	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	SDL_FreeSurface(surface);

	return textureID;
}

bool readHeader(std::ifstream &stream) {
	string line;
	mapWidth = -1;
	mapHeight = -1;
	while (getline(stream, line)) {
		if (line == "") { break; }
		istringstream sStream(line);
		string key, value;
		getline(sStream, key, '=');
		getline(sStream, value);
		if (key == "width") {
			mapWidth = atoi(value.c_str());
		}
		else if (key == "height"){
			mapHeight = atoi(value.c_str());
		}
	}
	if (mapWidth == -1 || mapHeight == -1) {
		return false;
	}
	else { // allocate our map data
		levelData = new unsigned char*[mapHeight];
		for (int i = 0; i < mapHeight; ++i) {
			levelData[i] = new unsigned char[mapWidth];
		}
		return true;
	}
}

bool readLayerData(std::ifstream &stream) {
	string line;
	while (getline(stream, line)) {
		if (line == "") { break; }
		istringstream sStream(line);
		string key, value;
		getline(sStream, key, '=');
		getline(sStream, value);
		if (key == "data") {
			for (int y = 0; y < mapHeight; y++) {
				getline(stream, line);
				istringstream lineStream(line);
				string tile;
				for (int x = 0; x < mapWidth; x++) {
					getline(lineStream, tile, ',');
					unsigned char val = (unsigned char)atoi(tile.c_str());
					if (val > 0) {
						levelData[y][x] = val - 1;
					}
					else {
						levelData[y][x] = 0;
					}
				}
			}
		}
	}
	return true;
}

bool readEntityData(std::ifstream &stream) {
	string line;
	string type;
	while (getline(stream, line)) {
		if (line == "") { break; }
		istringstream sStream(line);
		string key, value;
		getline(sStream, key, '=');
		getline(sStream, value);
		if (key == "type") {
			type = value;
		}
		else if (key == "location") {
			istringstream lineStream(value);
			string xPosition, yPosition;
			getline(lineStream, xPosition, ',');
			getline(lineStream, yPosition, ',');
			float placeX = atoi(xPosition.c_str()) / 16 * TILE_SIZE;
			float placeY = atoi(yPosition.c_str()) / 16 * -TILE_SIZE;
			/*
			//placeEntity(type, placeX, placeY);
			player1 = new Entity();
			player1->x = placeX;
			player1->y = placeY;
			player1->height = TILE_SIZE;
			player1->width = TILE_SIZE;
			player1->isStatic = false;
			player1->isSolid = true;
			player1->sprite = new SheetSprite(mapID, player1->width, player1->height, 4.0);
			//player1->entityType = type;
			ents.push_back(player1);
			*/
		}
	}
	return true;
}

void readMap(){
	ifstream infile("map2.txt");
	string line;
	while (getline(infile, line)) {
		if (line == "[header]") {
			if (!readHeader(infile)) {
				return;
			}
		}
		else if (line == "[layer]") {
			readLayerData(infile);
		}
		else if (line == "[ObjectsLayer]") {
			//readEntityData(infile);
		}
	}

	for (int y = 0; y < mapHeight; y++) {
		for (int x = 0; x < mapWidth; x++) {
			if (levelData[y][x] != 0){
				tileCount++;
				float u = (float)(((int)levelData[y][x]) % SPRITE_COUNT_X) / (float)SPRITE_COUNT_X;
				float v = (float)(((int)levelData[y][x]) / SPRITE_COUNT_X) / (float)SPRITE_COUNT_Y;
				float spriteWidth = 1.0f / (float)SPRITE_COUNT_X;
				float spriteHeight = 1.0f / (float)SPRITE_COUNT_Y;
				vertexData.insert(vertexData.end(), {
					TILE_SIZE * x, -TILE_SIZE * y,
					TILE_SIZE * x, (-TILE_SIZE * y) - TILE_SIZE,
					(TILE_SIZE * x) + TILE_SIZE, (-TILE_SIZE * y) - TILE_SIZE,
					TILE_SIZE * x, -TILE_SIZE * y,
					(TILE_SIZE * x) + TILE_SIZE, (-TILE_SIZE * y) - TILE_SIZE,
					(TILE_SIZE * x) + TILE_SIZE, -TILE_SIZE * y
				});
				texCoordData.insert(texCoordData.end(), {
					u, v,
					u, v + (spriteHeight),
					u + spriteWidth, v + (spriteHeight),
					u, v,
					u + spriteWidth, v + (spriteHeight),
					u + spriteWidth, v
				});
			}
		}
	}
}

void renderWorld(){
	modelMatrix.identity();
	program->setModelMatrix(modelMatrix);
	glUseProgram(program->programID);
	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
	glEnableVertexAttribArray(program->positionAttribute);
	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
	glEnableVertexAttribArray(program->texCoordAttribute);
	glBindTexture(GL_TEXTURE_2D, mapID);
	glDrawArrays(GL_TRIANGLES, 0, tileCount * 6);
	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);
}

void worldToTileCoordinates(float worldX, float worldY, int *gridX, int *gridY) {
	*gridX = (int)(worldX / TILE_SIZE);
	*gridY = (int)(-worldY / TILE_SIZE);
}

void update(float elapsed){	

	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
			done = true;
		}
	}
	const Uint8* keys = SDL_GetKeyboardState(NULL);
	if (keys[SDL_SCANCODE_RIGHT]){
		player1->acceleration_x = 8.0;
	}
	else if (keys[SDL_SCANCODE_LEFT]){
		player1->acceleration_x = -8.0;
	}
	/*else{
		player1->acceleration_x = 0.0;
	}*/
	if (keys[SDL_SCANCODE_SPACE]){
		player1->shootBullet();
	}
	if (keys[SDL_SCANCODE_UP]) {
		if (player1->collidedBottom) {
			player1->velocity_y = 6;
			Mix_PlayChannel(-1, jumpSound, 0);
		}

	}
	if (keys[SDL_SCANCODE_0]){ done = true; }

	
		//player1->acceleration_y = 0.0;
	
	player1->Update(elapsed);
}

void checkCollisions(){
	//for (int i = 0; i < ents.size(); i++){
		int gridX, gridY;

		//bottom collision
		worldToTileCoordinates(player1->x, player1->y - player1->height / 2, &gridX, &gridY);
		if (levelData[gridY][gridX] != 0){
			player1->velocity_y = 0;
			//player1->y += 0.5;
			player1->collidedBottom = true;
		}
		//top collision
		worldToTileCoordinates(player1->x, player1->y + player1->height / 2, &gridX, &gridY);
		if (levelData[gridY][gridX] != 0){
			player1->velocity_y = 0;
			//player1->y -= 0.5;
			player1->collidedTop = true;
		}
		//left collision
		worldToTileCoordinates(player1->x - player1->width / 2, player1->y, &gridX, &gridY);
		if (levelData[gridY][gridX] != 0){
			player1->velocity_x = 0;
			//player1->x += 0.5;
			player1->collidedLeft = true;
		}
		//right collision
		worldToTileCoordinates(player1->x + player1->width / 2, player1->y, &gridX, &gridY);
		if (levelData[gridY][gridX] != 0){
			player1->velocity_x = 0;
			//player1->x -= 0.5;
			player1->collidedRight = true;
		}
	//}
}

void render(){
	glClear(GL_COLOR_BUFFER_BIT);
	renderWorld();
	for (int i = 0; i < ents.size(); i++){
		ents[i]->Render(program);
	}
}

void updateAndRender(){
	float ticks = (float)SDL_GetTicks() / 1000.0f;
	float elapsed = ticks - lastFrameTicks;
	lastFrameTicks = ticks;

	float fixedElapsed = elapsed;
	if (fixedElapsed > FIXED_TIMESTEP * MAX_TIMESTEPS) {
		fixedElapsed = FIXED_TIMESTEP * MAX_TIMESTEPS;
	}
	while (fixedElapsed >= FIXED_TIMESTEP) {
		fixedElapsed -= FIXED_TIMESTEP;
		update(FIXED_TIMESTEP);
	}
	update(fixedElapsed);
	checkCollisions();	
	render();
}

void init(){
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("CS3113 Final", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
	glewInit();
#endif

	done = false;

	glViewport(0, 0, 640, 360);

	program = new ShaderProgram(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
	projectionMatrix.setOrthoProjection(-8.0f, 8.0f, -6.0f, 6.0f, -1.0f, 1.0f);
	program->setModelMatrix(modelMatrix);
	program->setProjectionMatrix(projectionMatrix);
	program->setViewMatrix(viewMatrix);
	glUseProgram(program->programID);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	mapID = LoadTexture("arne_sprites.png");
	player1ID = LoadTexture("player1.png");
	GLuint bulletTexture = LoadTexture("bullet.png");
	SheetSprite bulletSprite(program, bulletTexture, 0, 0, 0.1, 0.06, 0.1);
	Bullet bullet(program, bulletSprite);
	p1sheet = new SheetSprite(program, player1ID, 0, 0, 1, 1, 1.0);
	player1 = new Entity(p1sheet, 4.0, -10.0, 0.5, 0.4, 0, 0, 0, 0, bullet);
	
	ents.push_back(player1);
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);

	Mix_Music *music;
	music = Mix_LoadMUS("music.mp3");
	//player2ID = LoadTexture("player2.png");
	//p2sheet = new SheetSprite(program, player1ID, 0, 0, 1, 1, 1.0);
	//player2 = new Entity(p2sheet, 8.0, -10.0, 2.0, 0.4, 0, 0, 0, 0);
	//ents.push_back(player2);
	readMap();

	Mix_PlayMusic(music, -1);
}

void scrollingP1(){
	viewMatrix.identity();
	viewMatrix.Translate(-player1->x, -player1->y, 0);
	program->setViewMatrix(viewMatrix);
}

int main(int argc, char *argv[]){
	init();
	while (!done) { //you can press 0 to exit the game (if it were to work)
		
		updateAndRender();
		scrollingP1();
		//viewMatrix.identity();
		//viewMatrix.Translate(-4.0, 8.0, 0);
		//program->setViewMatrix(viewMatrix);
		
		SDL_GL_SwapWindow(displayWindow);
	}
	SDL_Quit();
	return 0;
}
