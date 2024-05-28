#include <SDL.h>
// #include <SDL_image.h> // removed for now...
#include <stdio.h>
#include <stdbool.h>
#include "alg.h" // from Ivan's part

const int WIDTH = 1080;
const int HEIGHT = 720;
const int PLAYER_WIDTH = 60;
const int PLAYER_HEIGHT = 60;
const int ENEMY_SIZES = 60;
const int BULLET_SIZE = 10;

const int WALL = -1; // barrier cell of the matrix (a labyrinth's wall)
const int BLANK = -2; // free cell of the matrix (a map's free hall)

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
SDL_Texture* map = NULL;
SDL_Texture* menu = NULL;

// the main player textures:
SDL_Texture* player_right = NULL;
SDL_Texture* player_left = NULL;
SDL_Texture* player_down = NULL;
SDL_Texture* player_up = NULL;
SDL_Texture* currentPlayer = NULL; // the main player's texture!

// the Enemies' textures:
SDL_Texture* enemy_1 = NULL;
SDL_Texture* enemy_2 = NULL;

// the player's gun bullet:
SDL_Texture* bullet = NULL;

SDL_Surface* labyrinth = NULL; // the labyrinth's surface
const int CELL_SIZE = 1; // the size of one map's cell (in pixels)
int maze[720][1080]; // the labyrinth's maze

// the main player:
Player_Position playerPos; // the position of the player on the game's map
// the Enemies (4):
enemy Enemies[2]; // the Enemies' positions and behavers structer

typedef struct Bullet {
	int x;
	int y;
	bool is_fired; // "shooted bullet" or not
	int direction;
	bool gotInEnemy;
} Bullet; // the behave of the player's gun bullet of the playing field

int initialization() {

	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		printf("Possible problem with: %s\n", SDL_GetError());
		return 1;
	}
	else {

		window = SDL_CreateWindow("Level_One:_DEMO(v2.0)", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			WIDTH, HEIGHT, SDL_WINDOW_SHOWN);

		if (!window) {
			printf("Possible problem with: %s\n", SDL_GetError());
			return 1;
		}
	}

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	if (!renderer) {
		printf("Possible problem with: %s\n", SDL_GetError());
		return 1;
	}
	else {
		SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
	}

	return 0;
}

SDL_Texture* loadTexture(const char* path) {

	SDL_Surface* loadedSurface = SDL_LoadBMP(path);
	if (!loadedSurface) {
		printf("Possible problem with: %s\n", SDL_GetError());
		return NULL;
	}
	SDL_Texture* newTexture = SDL_CreateTextureFromSurface(renderer, loadedSurface);

	if (!newTexture) {
		printf("Possible problem with: %s\n", SDL_GetError());
		return NULL;
	}

	SDL_FreeSurface(loadedSurface);

	return newTexture;
}

SDL_Surface* loadSurface(const char* path) {

	SDL_Surface* loadedSurface = SDL_LoadBMP(path);

	if (!loadedSurface) {
		printf("Possible problem with: %s\n", SDL_GetError());
	}

	return loadedSurface;
}

// the founding pixel from surface function:
Uint32 get_pixel32(SDL_Surface* surface, int x, int y) {
	Uint32* pixels = (Uint32*)surface->pixels;
	return pixels[(y * surface->w) + x];
}

// the maze filling function with WALL and BLANL constants:
void mazeFilling(SDL_Surface* labirinth, int maze[720][1080], int cellSize) {

	if (labirinth == NULL) {
		printf("Labirinth's problem with: %s\n", SDL_GetError());
		return;
	}

	for (int y = 0; y < labirinth->h; y += cellSize) {
		for (int x = 0; x < labirinth->w; x += cellSize) {

			Uint32 pixel = get_pixel32(labirinth, x, y);
			SDL_Color color;
			SDL_GetRGB(pixel, labirinth->format, &color.r, &color.g, &color.b);

			if (color.r == 0 && color.g == 0 && color.b == 0) {
				maze[y / cellSize][x / cellSize] = WALL; // that's labyrinth's wall!
			}
			else {
				maze[y / cellSize][x / cellSize] = BLANK; // that's just map's free way...
			}
		}
	}

	return;
}

bool playerCollision(int PosX, int PosY) {

	if (PosX < 0 || PosY < 0 || PosX + PLAYER_WIDTH >= WIDTH || PosY + PLAYER_HEIGHT >= HEIGHT) {
		return false;
	}

	// the checking of the every player's angle in collision with wall:
	for (int w = 0; w < PLAYER_WIDTH; w += CELL_SIZE) {
		for (int h = 0; h < PLAYER_HEIGHT; h += CELL_SIZE) {

			int gridX = (PosX + w) / CELL_SIZE;
			int gridY = (PosY + h) / CELL_SIZE;

			if (maze[gridY][gridX] == WALL) {
				return false;
			}
		}
	}

	return true;
}

int getBulletDirection(SDL_Texture* currentPlayer) {

	if (currentPlayer == player_up) {
		return 0;
	}
	else if (currentPlayer == player_down) {
		return 1;
	}
	else if (currentPlayer == player_left) {
		return 2;
	}
	else if (currentPlayer == player_right) {
		return 3;
	}

	return -1; // if the player's direction wasn't defined...
}

// if there's n0t any wall right in front of the player, we can shoot the bullet:
bool canShoot(int direction) {

	int checkX = playerPos.x;
	int checkY = playerPos.y;

	switch (direction) {
	case 0: // up
		checkY -= CELL_SIZE;
		break;
	case 1: // down
		checkY += PLAYER_HEIGHT;
		break;
	case 2: // left
		checkX -= CELL_SIZE;
		break;
	case 3: // right
		checkX += PLAYER_WIDTH;
		break;
	default:
		// just nothing for now...
		break;
	}

	int gridX = checkX / CELL_SIZE;
	int gridY = checkY / CELL_SIZE;

	if (gridX < 0 || gridY < 0 || gridX >= WIDTH / CELL_SIZE || gridY >= HEIGHT / CELL_SIZE) {
		return false;
	}

	return maze[gridY][gridX] != WALL;
}

// the "unshooting bullet" function:
void stayingBullet(Bullet* bullet, SDL_Texture* currentPlayer) {

	// the track displacement:
	int bulletOffsetX = 20;
	int bulletOffsetY = -20;

	bullet->is_fired = true;

	int up = 0, down = 1, left = 2, right = 3;

	if (currentPlayer == player_up) {

		bullet->direction = up;
		bullet->x = playerPos.x + (PLAYER_WIDTH / 2) - bulletOffsetX;
		bullet->y = playerPos.y + bulletOffsetY + 20;

	}
	else if (currentPlayer == player_down) {

		bullet->direction = down;
		bullet->x = playerPos.x + (PLAYER_WIDTH / 2) + 7;
		bullet->y = playerPos.y + PLAYER_HEIGHT - 20;

	}
	else if (currentPlayer == player_left) {

		bullet->direction = left;
		bullet->x = playerPos.x + 10;
		bullet->y = playerPos.y + (PLAYER_HEIGHT / 2) + 6;

	}
	else if (currentPlayer == player_right) {
		
		bullet->direction = right;
		bullet->x = playerPos.x + PLAYER_WIDTH - 20;
		bullet->y = playerPos.y + (PLAYER_HEIGHT / 2) - 17;

	}

	return;
}

// the reseting the bullet if it got into the board or wall and bringing back to the player's position:
void resetBullet(Bullet* bullet) {

	bullet->is_fired = false;

	bullet->x = playerPos.x;
	bullet->y = playerPos.y;

	return;
}

bool bulletCollision(Bullet* bullet) {

	for (int w = 0; w < BULLET_SIZE; w += CELL_SIZE) {
		for (int h = 0; h < BULLET_SIZE; h += CELL_SIZE) {

			int gridX = (bullet->x + w) / CELL_SIZE;
			int gridY = (bullet->y + h) / CELL_SIZE;

			if (gridX < 0 || gridY < 0 || gridX >= WIDTH / CELL_SIZE || gridY >= HEIGHT / CELL_SIZE) {
				bullet->is_fired = false;
				return false;
			}
			if (maze[gridY][gridX] == WALL) {
				bullet->is_fired = false;
				return false;
			}
		}
	}

	return true; // there's no collision (a free way)
}

// the bullet moving function:
void bulletMoving(Bullet* bullet) {

	if (bullet->is_fired) {

		const int bullet_speed = 15; // or 10 (changeable for now...)

		int new_x = bullet->x;
		int new_y = bullet->y;

		switch (bullet->direction) {
		case 0: // up
			new_y -= bullet_speed;
			break;
		case 1: // down
			new_y += bullet_speed;
			break;
		case 2: // left
			new_x -= bullet_speed;
			break;
		case 3: // right
			new_x += bullet_speed;
			break;
		default:
			// just nothing for now...
			break;
		}

		// creating a "transitory" bullet object to test the collision
		Bullet temp_bullet = *bullet;
		temp_bullet.x = new_x;
		temp_bullet.y = new_y;

		// if collision occurs, we reset the actual bullet
		if (!bulletCollision(&temp_bullet) || bullet->gotInEnemy == true) {
			resetBullet(bullet);
			bullet->gotInEnemy = false;
		}
		else {
			bullet->x = new_x;
			bullet->y = new_y;
		}
	}

	return;
}

void initialiseEnemies() {

	// first enemy initialisation:
	Enemies[0].x = 250;
	Enemies[0].y = 20;
	Enemies[0].isTriggered = 0;
	Enemies[0].isKilled = 0; // 0 means that the enemy is alive

	// second enemy initialisation:
	Enemies[1].x = 675;
	Enemies[1].y = 640;
	Enemies[1].isTriggered = 0;
	Enemies[1].isKilled = 0;

	return;
}

void checkBulletHit(Bullet* bullet) {
	for (int i = 0; i < 2; i++) {
		if (bullet->is_fired && Enemies[i].isKilled < 3 &&
			bullet->x >= Enemies[i].x && bullet->x <= (Enemies[i].x + ENEMY_SIZES) &&
			bullet->y >= Enemies[i].y && bullet->y <= (Enemies[i].y + ENEMY_SIZES)) {

			Enemies[i].isKilled += 1;

			bullet->gotInEnemy = true;

			return;
		}
	}
}

void quit_game() {

	for (int i = 0; i < HEIGHT; i++) {
		for (int j = 0; j < WIDTH; j++) {
			maze[i][j] = NULL;
		}
	}

	SDL_DestroyTexture(map);
	SDL_DestroyTexture(menu);
	SDL_DestroyTexture(player_right);
	SDL_DestroyTexture(player_left);
	SDL_DestroyTexture(player_down);
	SDL_DestroyTexture(player_up);
	SDL_DestroyTexture(currentPlayer);
	SDL_DestroyTexture(bullet);
	SDL_DestroyTexture(enemy_1);
	SDL_DestroyTexture(enemy_2);
	SDL_DestroyRenderer(renderer);
	SDL_FreeSurface(labyrinth);
	SDL_DestroyWindow(window);

	labyrinth = NULL;
	menu = map = NULL;
	currentPlayer = player_right = player_left = player_down = player_up = NULL;
	bullet = enemy_1 = enemy_2 = NULL;
	renderer = NULL;
	window = NULL;

	SDL_Quit();
	
	return;
}

int main(int argc, char* argv[]) {

	if (initialization() != 0) {
		printf("Initialisation down...\n");
		return 1;
	}
	else {

		bool keyStates[SDL_NUM_SCANCODES] = { false }; // the mass of the key activity values 

		// the main textures' loadings:
		menu = loadTexture("Images/the_menu.bmp");
		map = loadTexture("Images/the_map.bmp");
		player_right = loadTexture("Images/main_player_right.bmp");
		player_left = loadTexture("Images/main_player_left.bmp");
		player_down = loadTexture("Images/main_player_down.bmp");
		player_up = loadTexture("Images/main_player_up.bmp");
		bullet = loadTexture("Images/the_bullet.bmp");
		enemy_1 = loadTexture("Images/the_enemy_left.bmp"); // not current for now...
		enemy_2 = loadTexture("Images/the_enemy_right.bmp"); // not current for now...

		labyrinth = loadSurface("Images/wbmap.bmp");
		mazeFilling(labyrinth, maze, CELL_SIZE);

		playerPos.x = 40; // the start position of the player on "x" coordinate
		playerPos.y = (HEIGHT / 2) - 100; // the start position of the player on "y" coordinate

		Bullet theBullet = { 0, 0, false, -1, false}; // crearing the bullet object

		initialiseEnemies();

		bool menuShown = true;
		bool isRunning = true;

		if (!menu) {
			printf("Problem with menu's texture...\n");
		}
		else {

			while (isRunning && menuShown) {
				SDL_Event event;
				while (SDL_PollEvent(&event)) {
					if (event.type == SDL_QUIT) {
						isRunning = false;
					}
					if (event.type == SDL_KEYDOWN) {
						if (event.key.keysym.scancode == SDL_SCANCODE_X) {
							menuShown = false;
						}
						if (event.key.keysym.scancode == SDL_SCANCODE_Q) {
							menuShown = false;
							isRunning = false;
						}
					}
				}
				// menu drawing:
				SDL_RenderClear(renderer);
				SDL_Rect menuRect = { 0, 0, WIDTH, HEIGHT };
				SDL_RenderCopy(renderer, menu, NULL, &menuRect);
				SDL_RenderPresent(renderer);
			}
		}

		if (!map) {
			printf("Texture's downloading down...\n");
			return 1;
		}
		else {

			currentPlayer = player_right; // the start pose of the player...

			while (isRunning) {

				SDL_Event event;

				while (SDL_PollEvent(&event)) {

					if (event.type == SDL_QUIT) {
						isRunning = false;
					}
					if (event.type == SDL_KEYDOWN) {
						keyStates[event.key.keysym.scancode] = true;
					}
					if (event.type == SDL_KEYUP) {
						keyStates[event.key.keysym.scancode] = false;
					}
				}

				int newX = playerPos.x;
				int newY = playerPos.y;

				if (keyStates[SDL_SCANCODE_W]) { 
					newY -= 5; 
					currentPlayer = player_up;
				}
				else if (keyStates[SDL_SCANCODE_S]) { 
					newY += 5; 
					currentPlayer = player_down;
				}
				else if (keyStates[SDL_SCANCODE_A]) { 
					newX -= 5; 
					currentPlayer = player_left;
				}
				else if (keyStates[SDL_SCANCODE_D]) { 
					newX += 5; 
					currentPlayer = player_right;
				}
				else if (keyStates[SDL_SCANCODE_Q]) {
					isRunning = false;
				}

				if (keyStates[SDL_SCANCODE_E] && !theBullet.is_fired) {

					theBullet.direction = getBulletDirection(currentPlayer);
					
					if (theBullet.direction != -1 && canShoot(theBullet.direction)) {
						stayingBullet(&theBullet, currentPlayer);
					}
				}

				checkBulletHit(&theBullet);
				bulletMoving(&theBullet);
				
				if (playerCollision(newX, newY)) {
					playerPos.x = newX;
					playerPos.y = newY;
				}

				SDL_RenderClear(renderer);

				// the map rendering
				SDL_Rect mapRect = { NULL, NULL, WIDTH, HEIGHT };
				SDL_RenderCopy(renderer, map, NULL, &mapRect);

				// the bullet rendering
				if (theBullet.is_fired) {
					SDL_Rect bulletRect = { theBullet.x, theBullet.y, BULLET_SIZE, BULLET_SIZE };
					SDL_RenderCopy(renderer, bullet, NULL, &bulletRect);
				}

				// enemies rendering with isKilled < 3
				for (int i = 0; i < 2; i++) {
					if (Enemies[i].isKilled < 3) {
						SDL_Rect enemyRect = { Enemies[i].x, Enemies[i].y, ENEMY_SIZES, ENEMY_SIZES };
						SDL_Texture* enemyTexture = (i % 2 == 0) ? enemy_1 : enemy_2;
						SDL_RenderCopy(renderer, enemyTexture, NULL, &enemyRect);
					}
				}

				// the player rendering
				SDL_Rect playerRect = { playerPos.x, playerPos.y, PLAYER_WIDTH, PLAYER_HEIGHT };
				SDL_RenderCopy(renderer, currentPlayer, NULL, &playerRect);

				SDL_RenderPresent(renderer); // the updating of the textures on the map

				SDL_Delay(28); // for about 35fps (+-)
			}
		}
	}

	quit_game();

	return 0;
}
