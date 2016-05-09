#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include "Entity.h"
#include "Matrix.h"
#include "ShaderProgram.h"
#include "ParticleEmitter.h"

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
Matrix modelMatrixMain;
Matrix modelMatrixOver;
float lastFrameTicks;

vector<float> vertexData;
vector<float> texCoordData;
int mapWidth;
int mapHeight;
int tileCount;
unsigned char** levelData;
const Uint8* keys = SDL_GetKeyboardState(NULL);
vector<Entity*> ents;
string fileName;
GLuint mapID;
GLuint textSheet;
GLuint player1ID;
SheetSprite* p1sheet;
Entity* player1;
GLuint player2ID;
SheetSprite* p2sheet;
Entity* player2;
Entity* goal;

enum GameState { TITLE = 1, LEVEL1 = 2, LEVEL2 = 3, LEVEL3 = 4, END = 5 };
int gameState = 1;
bool levelInit = true;

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

void DrawText(ShaderProgram* program, int fontTexture, std::string text, float size, float spacing) {
	float texture_size = 1.0 / 16.0f;
	std::vector<float> vertexData;
	std::vector<float> texCoordData;
	for (int i = 0; i < text.size(); i++) {
		float texture_x = (float)(((int)text[i]) % 16) / 16.0f;
		float texture_y = (float)(((int)text[i]) / 16) / 16.0f;
		vertexData.insert(vertexData.end(), {
			((size + spacing) * i) + (-0.5f * size), 0.5f * size,
			((size + spacing) * i) + (-0.5f * size), -0.5f * size,
			((size + spacing) * i) + (0.5f * size), 0.5f * size,
			((size + spacing) * i) + (0.5f * size), -0.5f * size,
			((size + spacing) * i) + (0.5f * size), 0.5f * size,
			((size + spacing) * i) + (-0.5f * size), -0.5f * size,
		});
		texCoordData.insert(texCoordData.end(), {
			texture_x, texture_y,
			texture_x, texture_y + texture_size,
			texture_x + texture_size, texture_y,
			texture_x + texture_size, texture_y + texture_size,
			texture_x + texture_size, texture_y,
			texture_x, texture_y + texture_size,
		});
	}
	glUseProgram(program->programID);
	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
	glEnableVertexAttribArray(program->positionAttribute);
	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
	glEnableVertexAttribArray(program->texCoordAttribute);
	glBindTexture(GL_TEXTURE_2D, fontTexture);
	glDrawArrays(GL_TRIANGLES, 0, text.size() * 6);
	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);
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
		}
	}
	return true;
}

void readMap(string fileName){
	ifstream infile(fileName);
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
	if (keys[SDL_SCANCODE_RIGHT]){
		player1->acceleration_x = 8.0;
		
	}
	else if (keys[SDL_SCANCODE_LEFT]){
		player1->acceleration_x = -8.0;
	}
	else{
		player1->acceleration_x = 0.0;
	}
	if (keys[SDL_SCANCODE_SPACE]){
		player1->shootBullet();
	}

	if (keys[SDL_SCANCODE_W]) {
		if (player2->collidedBottom)
			player2->velocity_y = 6;
	}
	if (keys[SDL_SCANCODE_D]){
		player2->acceleration_x = 8.0;
	}
	else if (keys[SDL_SCANCODE_A]){
		player2->acceleration_x = -8.0;
	}
	else{
		player2->acceleration_x = 0.0;
	}
	if (keys[SDL_SCANCODE_S]){
		player2->shootBullet();
	}
	if (keys[SDL_SCANCODE_UP]) {
		if (player1->collidedBottom)
			player1->velocity_y = 6;
	}

	if (keys[SDL_SCANCODE_0]){ done = true; }

		
	player1->Update(elapsed);
	player2->Update(elapsed);
}

void checkCollisions(){
	for (int i = 0; i < ents.size(); i++){
		int gridX, gridY;

		//bottom collision
		worldToTileCoordinates(ents[i]->x, ents[i]->y - ents[i]->height / 2, &gridX, &gridY);
		if (levelData[gridY][gridX] != 0){
			ents[i]->velocity_y = 0;
			ents[i]->y += 0.001;
			ents[i]->collidedBottom = true;
		}
		//top collision
		worldToTileCoordinates(ents[i]->x, ents[i]->y + ents[i]->height / 2, &gridX, &gridY);
		if (levelData[gridY][gridX] != 0){
			ents[i]->velocity_y = 0;
			ents[i]->y -= 0.001;
			ents[i]->collidedTop = true;
		}
		//left collision
		worldToTileCoordinates(ents[i]->x - ents[i]->width / 2, ents[i]->y, &gridX, &gridY);
		if (levelData[gridY][gridX] != 0){
			ents[i]->velocity_x = 0.001;
			ents[i]->x += 0.001;
			ents[i]->collidedLeft = true;
		}
		//right collision
		worldToTileCoordinates(ents[i]->x + ents[i]->width / 2, ents[i]->y, &gridX, &gridY);
		if (levelData[gridY][gridX] != 0){
			ents[i]->velocity_x = 0;
			ents[i]->x -= 0.001;
			ents[i]->collidedRight = true;
		}
	}
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
	displayWindow = SDL_CreateWindow("CS3113 Final", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
	glewInit();
#endif

	done = false;

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
}

void scrollingP1(){
	viewMatrix.identity();
	viewMatrix.Translate(-player1->x, -player1->y, 0);
	program->setViewMatrix(viewMatrix);
}

void scrollingP2(){
	viewMatrix.identity();
	viewMatrix.Translate(-player2->x, -player2->y, 0);
	program->setViewMatrix(viewMatrix);
}

void level1(string fileName){
	glClear(GL_COLOR_BUFFER_BIT);

	if (levelInit){
		mapID = LoadTexture("arne_sprites.png");

		GLuint bulletTexture = LoadTexture("bullet.png");
		SheetSprite bulletSprite(program, bulletTexture, 0, 0, 0.1, 0.06, 0.1);
		Bullet bullet(program, bulletSprite);

		player1ID = LoadTexture("player1.png");
		p1sheet = new SheetSprite(program, player1ID, 0, 0, 1, 1, 1.0);
		player1 = new Entity(p1sheet, 4.0, -10.0, 0.5, 0.8, 0, 0, 0, 0, bullet);

		ents.push_back(player1);

		player2ID = LoadTexture("player2.png");
		p2sheet = new SheetSprite(program, player2ID, 0, 0, 1, 1, 1.0);
		player2 = new Entity(p2sheet, 8.0, -10.0, 0.5, 0.8, 0, 0, 0, 0, bullet);
		ents.push_back(player2);

		readMap(fileName);
		levelInit = false;
	}
	updateAndRender();
	scrollingP1();
	SDL_GL_SwapWindow(displayWindow);
}

void titleScreen(){
	glClear(GL_COLOR_BUFFER_BIT);

	textSheet = LoadTexture("font1.png");
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
			done = true;
		}
	}

	ParticleEmitter emitter = ParticleEmitter(15);
	emitter.Render(program);

	emitter.Update()

	program->setModelMatrix(modelMatrixMain);
	modelMatrixMain.identity();
	modelMatrixMain.Translate(-5.8, 0.0, 0);
	DrawText(program, textSheet, "CS 3113", 2.0, 0.0005f);
	program->setModelMatrix(modelMatrixOver);
	modelMatrixOver.identity();
	modelMatrixOver.Translate(-6.5, -3.0, 0.0f);
	DrawText(program, textSheet, "Marcus & Noman", 1.0, 0.0005f);

	if (keys[SDL_SCANCODE_RETURN]) {
		gameState = 2;
	}
	SDL_GL_SwapWindow(displayWindow);

	return;
}

int main(int argc, char *argv[]){
	init();
	while (!done) {
		switch (gameState){
			case TITLE:
				titleScreen();
				break;
			case LEVEL1:
				level1("world1.txt");
				break;
			case LEVEL2:
				//level2();
				break;
			case LEVEL3:
				//level3();
				break;
			case END:
				//END();
				break;
		}
		/*
		updateAndRender();
		scrollingP1();
		SDL_GL_SwapWindow(displayWindow);*/
	}
	SDL_Quit();
	return 0;
}