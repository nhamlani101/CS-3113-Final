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
#include "PerlinNoise.h"

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

using namespace std;

#define TILE_SIZE 0.75f
#define SPRITE_COUNT_X 16
#define SPRITE_COUNT_Y 8 

//Globals
SDL_Window* displayWindow;
SDL_Window* displayWindow2;
SDL_GLContext context;
SDL_Event event;
bool done;
ShaderProgram* program;
Matrix projectionMatrix;
Matrix modelMatrix;
Matrix viewMatrix;
Matrix modelMatrixMain;
Matrix modelMatrixOver;
Matrix particleModel;

float lastFrameTicks;

vector<float> vertexData;
vector<float> texCoordData;
vector<Entity*> bullets;
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
GLuint bulletID;
SheetSprite* bulletSprite;
bool levelOver;
float p1wins = 0.0;
float p2wins = 0.0;

int frameCounter;
int frameCounter2;
float perlinValue;

enum GameState { TITLE = 1, LEVEL1 = 2, LEVEL2 = 3, LEVEL3 = 4, END = 5 };
int gameState = 1;
bool levelInit = true;

Mix_Music *music;
Mix_Chunk* shootSound;

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

void updateBullets(float elapsed) {
	for (int i = 0; i < bullets.size(); i++) {
		bullets[i]->Update(elapsed);
	}
}


void update(float elapsed){

	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
			done = true;
		}
		if (event.type == SDL_KEYDOWN) {
			if (event.key.keysym.sym == SDLK_SPACE) {
				Mix_PlayChannel(-1, shootSound, 0);

				Entity* p1Bullet = new Entity(bulletSprite, player1->x + player1->sprite->width - 0.5, player1->y, 0.5, 0.3, 7, 0, 1, 0, 1);
				if (player1->x > player2->x) {
					p1Bullet->bulletMoveLeft = true;
					p1Bullet->x = player1->x - player1->sprite->width;
					//cout << "Player 1 x is greater than player 2" << endl;
				}
				else {
					p1Bullet->bulletMoveLeft = false;
				}
				bullets.push_back(p1Bullet);
				p1Bullet->Render(program);
			}

			if (event.key.keysym.sym == SDLK_s) {
				Mix_PlayChannel(-1, shootSound, 0);

				Entity* p2Bullet = new Entity(bulletSprite, player2->x + player2->sprite->width - 0.5, player2->y, 0.5, 0.3, 7, 0, 1, 0, 2);
				if (player2->x > player1->x) {
					p2Bullet->bulletMoveLeft = true;
					p2Bullet->x = player2->x - player2->sprite->width;
				}
				else {
					p2Bullet->bulletMoveLeft = false;
				}
				bullets.push_back(p2Bullet);
				p2Bullet->Render(program);
				//Mix_PlayChannel(-1, jumpSound, 0);
			}
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

	if (keys[SDL_SCANCODE_UP]) {
		if (player1->collidedBottom)
			player1->velocity_y = 6;
	}

	if (keys[SDL_SCANCODE_0]){ done = true; }
	
	player1->Update(elapsed);
	player2->Update(elapsed);
	updateBullets(elapsed);
}

void checkCollisions(){
	for (int i = 0; i < ents.size(); i++){
		int gridX, gridY;

		worldToTileCoordinates(ents[i]->x, ents[i]->y - ents[i]->height / 2, &gridX, &gridY);
		if (levelData[gridY][gridX] != 0){
			if (levelData[gridY][gridX] == 49){
				if (ents[i]->p1){
					p1wins += 1.0;
				}
				else p2wins += 1.0;

				levelOver = true;
			}
			else{
				ents[i]->velocity_y = 0;
				ents[i]->y += 0.001;
				ents[i]->collidedBottom = true;
			}
		}

		worldToTileCoordinates(ents[i]->x, ents[i]->y + ents[i]->height / 2, &gridX, &gridY);
		if (levelData[gridY][gridX] != 0){
			if (levelData[gridY][gridX] == 49){
				if (ents[i]->p1){
					p1wins += 1.0;
				}
				else p2wins += 1.0;
				levelOver = true;
			}
			else{
				ents[i]->velocity_y = 0;
				ents[i]->y -= 0.001;
				ents[i]->collidedTop = true;
			}
		}

		worldToTileCoordinates(ents[i]->x - ents[i]->width / 2, ents[i]->y, &gridX, &gridY);
		if (levelData[gridY][gridX] != 0){
			if (levelData[gridY][gridX] == 49){
				if (ents[i]->p1){
					p1wins += 1.0;
				}
				else p2wins += 1.0;
				levelOver = true;
			}
			else{
				ents[i]->velocity_x = 0.001;
				ents[i]->x += 0.001;
				ents[i]->collidedLeft = true;
			}
		}

		worldToTileCoordinates(ents[i]->x + ents[i]->width / 2, ents[i]->y, &gridX, &gridY);
		if (levelData[gridY][gridX] != 0){
			if (levelData[gridY][gridX] == 49){
				if (ents[i]->p1){
					p1wins += 1.0;
				}
				else p2wins += 1.0;
				levelOver = true;
			}
			else{
				ents[i]->velocity_x = 0;
				ents[i]->x -= 0.001;
				ents[i]->collidedRight = true;
			}
		}
	}

	for (int i = 0; i < bullets.size(); i++) {
		for (int j = 0; j < ents.size(); j++) {
			if (!bullets[i]->bulletDead) {
				if (ents[j]->y - ents[j]->sprite->height / 2 < bullets[i]->y + bullets[i]->sprite->height / 2 &&
					ents[j]->y + ents[j]->sprite->height / 2 > bullets[i]->y - bullets[i]->sprite->height / 2 &&
					ents[j]->x + ents[j]->sprite->width / 2 > bullets[i]->x + bullets[i]->sprite->width / 2 &&
					ents[j]->x - ents[j]->sprite->width / 2 < bullets[i]->x - bullets[i]->sprite->width / 2)
				{
					if (j == 0 && bullets[i]->cameFrom == 1) {
						//Do nothing we're good
					}
					else if (j == 1 && bullets[i]->cameFrom == 2) {
						//Do nothing we're good
					}
					else {
						ents[j]->health = ents[j]->health - 1;
						bullets[i]->bulletDead = true;
						cout << "Player: " << j << " health: " << ents[j]->health << endl;
					}
				}
			}
		}
	}
	for (int i = 0; i < ents.size(); i++){
		if (ents[i]->health <= 0){
			if (ents[i]->p1){
				p2wins += 1.0;
			}
			else p1wins += 1.0;

			levelOver = true;
		}
	}
}


void render(){
	glClear(GL_COLOR_BUFFER_BIT);
	renderWorld();
	for (int i = 0; i < ents.size(); i++){
		ents[i]->Render(program);
	}
	for (int i = 0; i < bullets.size(); i++) {
		bullets[i]->Render(program);
	}
	
}

void updateAndRender(){

	float ticks = (float)SDL_GetTicks() / 1000.0f;
	float elapsed = ticks - lastFrameTicks;
	perlinValue += elapsed;
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

void resetLevel(){
	gameState += 1;
	levelInit = true;
	levelOver = false;
	for (int i = 0; i < vertexData.size(); i++){
		vertexData[i] = NULL;
	}
	for (int i = 0; i < texCoordData.size(); i++){
		texCoordData[i] = NULL;
	}
	for (int i = 0; i < ents.size() + 1; i++){
		ents.pop_back();
	}
	if (bullets.size() != 0){
		for (int i = 0; i < bullets.size() + 1; i++){
			bullets.pop_back();
		}
	}
}
void init(){
	SDL_Init(SDL_INIT_VIDEO);

	displayWindow = SDL_CreateWindow("Player 2 Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 600, 450, SDL_WINDOW_OPENGL);
	displayWindow2 = SDL_CreateWindow("Player 1 Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 600, 450, SDL_WINDOW_OPENGL);

	context = SDL_GL_CreateContext(displayWindow);

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

	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
	music = Mix_LoadMUS("sorry.mp3");
	Mix_PlayMusic(music, -1);

	shootSound = Mix_LoadWAV("sfx.wav");

	textSheet = LoadTexture("font1.png");


}

void displaySecond(){
	glClear(GL_COLOR_BUFFER_BIT);

	if (gameState == 1){
		SDL_GL_MakeCurrent(displayWindow2, context);

		modelMatrixMain.identity();
		modelMatrixMain.Translate(-5.8, 0.0, 0);
		program->setModelMatrix(modelMatrixMain);
		DrawText(program, textSheet, "CS 3113", 2.0, 0.0005f);
		program->setModelMatrix(modelMatrixOver);
		modelMatrixOver.identity();
		modelMatrixOver.Translate(-6.5, -3.0, 0.0f);
		DrawText(program, textSheet, "Marcus & Noman", 1.0, 0.0005f);

		SDL_GL_SwapWindow(displayWindow2);
	}
	else{
		if (p1wins == 2.0){
			SDL_GL_MakeCurrent(displayWindow2, context);

			modelMatrixMain.identity();
			modelMatrixMain.Translate(-6.75, 0.0, 0);
			program->setModelMatrix(modelMatrixMain);
			DrawText(program, textSheet, "Charizard Wins!", 1.0, 0.0005f);
			program->setModelMatrix(modelMatrixOver);
			modelMatrixOver.identity();
			modelMatrixOver.Translate(-5.0, -3.0, 0.0f);
			DrawText(program, textSheet, "Play Again?", 1.0, 0.0005f);

		}

		else{
			SDL_GL_MakeCurrent(displayWindow2, context);

			modelMatrixMain.identity();
			modelMatrixMain.Translate(-6.5, 0.0, 0);
			program->setModelMatrix(modelMatrixMain);
			DrawText(program, textSheet, "Pikachu Wins!", 1.0, 0.0005f);
			program->setModelMatrix(modelMatrixOver);
			modelMatrixOver.identity();
			modelMatrixOver.Translate(-5.0, -3.0, 0.0f);
			DrawText(program, textSheet, "Play Again?", 1.0, 0.0005f);
		}
		SDL_GL_SwapWindow(displayWindow2);

	}

	return;
}


void titleScreen(){
	glClear(GL_COLOR_BUFFER_BIT);

	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
			done = true;
		}
	}

	SDL_GL_MakeCurrent(displayWindow, context);

	modelMatrixMain.identity();
	modelMatrixMain.Translate(-5.8, 0.0, 0);
	program->setModelMatrix(modelMatrixMain);
	DrawText(program, textSheet, "CS 3113", 2.0, 0.0005f);
	program->setModelMatrix(modelMatrixOver);
	modelMatrixOver.identity();
	modelMatrixOver.Translate(-6.5, -3.0, 0.0f);
	DrawText(program, textSheet, "Marcus & Noman", 1.0, 0.0005f);

	if (keys[SDL_SCANCODE_RETURN]) {
		gameState = 2;
	}
	if (keys[SDL_SCANCODE_0]) {
		done = true;
	}

	SDL_GL_SwapWindow(displayWindow);

	displaySecond();
}

void endScreen(){
	glClear(GL_COLOR_BUFFER_BIT);

	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
			done = true;
		}
	}

	viewMatrix.identity();
	program->setViewMatrix(viewMatrix);
	if (p1wins == 2.0){
		SDL_GL_MakeCurrent(displayWindow, context);

		modelMatrixMain.identity();
		modelMatrixMain.Translate(-6.75, 0.0, 0);
		program->setModelMatrix(modelMatrixMain);
		DrawText(program, textSheet, "Charizard Wins!", 1.0, 0.0005f);
		program->setModelMatrix(modelMatrixOver);
		modelMatrixOver.identity();
		modelMatrixOver.Translate(-5.0, -3.0, 0.0f);
		DrawText(program, textSheet, "Play Again?", 1.0, 0.0005f);

	}

	else{
		SDL_GL_MakeCurrent(displayWindow, context);

		modelMatrixMain.identity();
		modelMatrixMain.Translate(-6.5, 0.0, 0);
		program->setModelMatrix(modelMatrixMain);
		DrawText(program, textSheet, "Pikachu Wins!", 1.0, 0.0005f);
		program->setModelMatrix(modelMatrixOver);
		modelMatrixOver.identity();
		modelMatrixOver.Translate(-5.0, -3.0, 0.0f);
		DrawText(program, textSheet, "Play Again?", 1.0, 0.0005f);
	}

	if (keys[SDL_SCANCODE_RETURN]) {
		p1wins = 0.0;
		p2wins = 0.0;
		gameState = 2;
	}
	else if (keys[SDL_SCANCODE_0]){
		done = true;
	}

	SDL_GL_SwapWindow(displayWindow);

	displaySecond();
}

void scrollingP1(){
	frameCounter++;
	viewMatrix.identity();
	viewMatrix.Translate(-player1->x, -player1->y, 0);
	if (frameCounter == 500) {
		viewMatrix.Translate(noise1(perlinValue) * 3, noise1(perlinValue + 10.0f) * 3, 0.0);
		frameCounter = 0;
	}
	program->setViewMatrix(viewMatrix);
	modelMatrixMain.identity();
	modelMatrixMain.Translate(player1->x, player1->y + 3.0f, 0);
	program->setModelMatrix(modelMatrixMain);
	DrawText(program, textSheet, to_string(player2->health), 1.0, 0.0005f);
}

void scrollingP2(){
	frameCounter2++;
	viewMatrix.identity();
	viewMatrix.Translate(-player2->x, -player2->y, 0);
	if (frameCounter2 == 500) {
		viewMatrix.Translate(noise1(perlinValue) * 3, noise1(perlinValue + 10.0f) * 3, 0.0);
		frameCounter2 = 0;
	}
	program->setViewMatrix(viewMatrix);
	modelMatrixMain.identity();
	modelMatrixMain.Translate(player2->x, player2->y + 3.0f, 0);
	program->setModelMatrix(modelMatrixMain);
	DrawText(program, textSheet, to_string(player1->health), 1.0, 0.0005f);
}

void checkLevelOver(){
	if (levelOver){
		resetLevel();
		if (p1wins == 2.0 || p2wins == 2.0){
			gameState = END;
		}
	}
}

void level1(string fileName){
	glClear(GL_COLOR_BUFFER_BIT);
	
	GLint levelUniform = glGetUniformLocation(program->programID, "level");

	glUniform1i(levelUniform, 1);

	if (levelInit){
		mapID = LoadTexture("arne_sprites.png");

		player1ID = LoadTexture("charizard2.png");
		p1sheet = new SheetSprite(program, player1ID, 0, 0, 1, 1, 1.0);
		player1 = new Entity(p1sheet, 2, -6.0, 0.5, 0.8, 0, 0, 0, 0, 0);
		player1->p1 = true;

		ents.push_back(player1);

		player2ID = LoadTexture("pikachu.png");
		p2sheet = new SheetSprite(program, player2ID, 0, 0, 1, 1, 1.0);
		player2 = new Entity(p2sheet, 22.0, -6.0, 0.4, 0.8, 0, 0, 0, 0, 0);
		ents.push_back(player2);

		bulletID = LoadTexture("bullet.png");
		bulletSprite = new SheetSprite(program, bulletID, 0, 0, 0.15, 0.13, 0.18);

		readMap(fileName);
		levelInit = false;
	}
	updateAndRender();

	SDL_GL_MakeCurrent(displayWindow, context);
	render();
	scrollingP1();
	SDL_GL_SwapWindow(displayWindow);

	SDL_GL_MakeCurrent(displayWindow2, context);
	render();
	scrollingP2();
	SDL_GL_SwapWindow(displayWindow2);

	checkLevelOver();
}

void level2(string fileName){
	glClear(GL_COLOR_BUFFER_BIT);

	GLint levelUniform = glGetUniformLocation(program->programID, "level");

	glUniform1i(levelUniform, 2);

	if (levelInit){
		mapID = LoadTexture("arne_sprites.png");

		player1ID = LoadTexture("charizard2.png");
		p1sheet = new SheetSprite(program, player1ID, 0, 0, 1, 1, 1.0);
		player1 = new Entity(p1sheet, 2.5, -22.5, 0.5, 0.8, 0, 0, 0, 0, 0);
		player1->p1 = true;

		ents.push_back(player1);

		player2ID = LoadTexture("pikachu.png");
		p2sheet = new SheetSprite(program, player2ID, 0, 0, 1, 1, 1.0);
		player2 = new Entity(p2sheet, 10.0, -22.5, 0.4, 0.8, 0, 0, 0, 0, 0);
		ents.push_back(player2);

		bulletID = LoadTexture("bullet.png");
		bulletSprite = new SheetSprite(program, bulletID, 0, 0, 0.15, 0.13, 0.18);

		readMap(fileName);
		levelInit = false;
	}
	updateAndRender();

	SDL_GL_MakeCurrent(displayWindow, context);
	render();
	scrollingP1();
	SDL_GL_SwapWindow(displayWindow);

	SDL_GL_MakeCurrent(displayWindow2, context);
	render();
	scrollingP2();
	SDL_GL_SwapWindow(displayWindow2);

	checkLevelOver();
}

void level3(string fileName){
	glClear(GL_COLOR_BUFFER_BIT);

	GLint levelUniform = glGetUniformLocation(program->programID, "level");

	glUniform1i(levelUniform, 3);

	if (levelInit){
		mapID = LoadTexture("arne_sprites.png");

		player1ID = LoadTexture("charizard2.png");
		p1sheet = new SheetSprite(program, player1ID, 0, 0, 1, 1, 1.0);
		player1 = new Entity(p1sheet, 10, -7.0, 0.5, 0.8, 0, 0, 0, 0, 0);
		player1->p1 = true;

		ents.push_back(player1);

		player2ID = LoadTexture("pikachu.png");
		p2sheet = new SheetSprite(program, player2ID, 0, 0, 1, 1, 1.0);
		player2 = new Entity(p2sheet, 15.0, -7.0, 0.4, 0.8, 0, 0, 0, 0, 0);
		ents.push_back(player2);

		bulletID = LoadTexture("bullet.png");
		bulletSprite = new SheetSprite(program, bulletID, 0, 0, 0.15, 0.13, 0.18);

		readMap(fileName);
		levelInit = false;
	}
	updateAndRender();

	SDL_GL_MakeCurrent(displayWindow, context);
	render();
	scrollingP1();
	SDL_GL_SwapWindow(displayWindow);

	SDL_GL_MakeCurrent(displayWindow2, context);
	render();
	scrollingP2();
	SDL_GL_SwapWindow(displayWindow2);

	checkLevelOver();

}

int main(int argc, char *argv[]){
	init();
	while (!done) {
		switch (gameState){
		case TITLE:
			titleScreen();
			break;
		case LEVEL1:
			level1("level1.txt");
			break;
		case LEVEL2:
			level2("level2.txt");
			break;
		case LEVEL3:
			level3("level3.txt");
			break;
		case END:
			endScreen();
			break;
		}

	}
	Mix_FreeChunk(shootSound);
	Mix_FreeMusic(music);
	SDL_Quit();
	return 0;
}