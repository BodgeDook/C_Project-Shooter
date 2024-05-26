#include <SDL.h>
// #include <SDL_image.h>
#include <stdio.h>
#include <stdbool.h>

const int WIDTH = 1080;
const int HEIGHT = 720;
const int PLAYER_WIDTH = 60; // final value!
const int PLAYER_HEIGHT = 60; // final value!
const int ENEMY_SIZES = 60;
const int BULLET_SIZE = 10;

const int WALL = -1; // barrier cell of the matrix
const int BLANK = -2; // free cell of the matrix

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
SDL_Texture* map = NULL;

// the main player (textures):
SDL_Texture* player_right = NULL;
SDL_Texture* player_left = NULL;
SDL_Texture* player_down = NULL;
SDL_Texture* player_up = NULL;
SDL_Texture* currentPlayer = NULL;

// added...
SDL_Texture* enemy_1 = NULL;
SDL_Texture* enemy_2 = NULL;
// added...

// the player's gun bullet:
SDL_Texture* bullet = NULL;

SDL_Surface* labyrinth = NULL; // the labyrinth's surface
const int CELL_SIZE = 1; // the size of one map's cell (in pixels)
int maze[720][1080]; // the labyrinth's maze

typedef struct Player_Position {
	int x;
	int y;
} Player_Position;

Player_Position playerPos; // the position of the player on the game's map

typedef struct Bullet {
	int x;
	int y;
	bool is_fired;
	int direction;
} Bullet; // the behave of the player's gun bullet of the playing field

SDL_Texture* loadTexture(const char* path);

int init_function() {

	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		printf("Possible problem with: %s\n", SDL_GetError());
		return 1;
	}
	else {

		window = SDL_CreateWindow("Level_One:_Demo", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
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
		printf("Labirinth surface is null...\n");
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

// the "unshooting bullet" function:
void stayingBullet(Bullet* bullet, Player_Position playerPos, SDL_Texture* currentPlayer) {

	// the track displacement:
	int bulletOffsetX = 20;
	int bulletOffsetY = -20;

	bullet->is_fired = true;

	int up = 0, down = 1, left = 2, right = 3;

	if (currentPlayer == player_up) {
		bullet->direction = up;
		bullet->x = playerPos.x + (PLAYER_WIDTH / 2) - bulletOffsetX;
		bullet->y = playerPos.y + bulletOffsetY;
	}
	else if (currentPlayer == player_down) {
		bullet->direction = down;
		bullet->x = playerPos.x + (PLAYER_WIDTH / 2) + 7;
		bullet->y = playerPos.y + PLAYER_HEIGHT;
	}
	else if (currentPlayer == player_left) {
		bullet->direction = left;
		bullet->x = playerPos.x;
		bullet->y = playerPos.y + (PLAYER_HEIGHT / 2) + 6;
	}
	else if (currentPlayer == player_right) {
		bullet->direction = right;
		bullet->x = playerPos.x + PLAYER_WIDTH;
		bullet->y = playerPos.y + (PLAYER_HEIGHT / 2) - 17;
	}

	return;
}

// the reseting the bullet if it got into the board or wall and bringing back to the player's position:
void resetBullet(Bullet* bullet, Player_Position playerPos) {

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
		if (!bulletCollision(&temp_bullet)) {
			resetBullet(bullet, playerPos);
		}
		else {
			bullet->x = new_x;
			bullet->y = new_y;
		}
	}

	return;
}

void quit_function() {

	for (int i = 0; i < HEIGHT; i++) {
		for (int j = 0; j < WIDTH; j++) {
			maze[i][j] = NULL; // for the memory cleaning after work of the matrix (THE MOST IMPORTANT!!!)
		}
	}

	SDL_DestroyTexture(map);
	SDL_DestroyTexture(player_right);
	SDL_DestroyTexture(player_left);
	SDL_DestroyTexture(player_down);
	SDL_DestroyTexture(player_up);
	SDL_DestroyTexture(currentPlayer);
	SDL_DestroyTexture(bullet);
	SDL_DestroyTexture(enemy_1);
	SDL_DestroyTexture(enemy_2);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_FreeSurface(labyrinth);
	labyrinth = NULL;
	currentPlayer = player_right = player_left = player_down = player_up =
		bullet = map = enemy_1 = enemy_2 = NULL;
	renderer = NULL;
	window = NULL;

	SDL_Quit();
	
	return;
}

int main(int argc, char* argv[]) {

	if (init_function() != 0) {
		printf("Initialisation down...\n");
		return 1;
	}
	else {

		bool keyStates[SDL_NUM_SCANCODES] = { false }; // the mass of the key activity values 

		// the main textures' loadings:
		map = loadTexture("Images/the_map.bmp");
		currentPlayer = loadTexture("Images/main_player_right.bmp");
		player_right = loadTexture("Images/main_player_right.bmp");
		player_left = loadTexture("Images/main_player_left.bmp");
		player_down = loadTexture("Images/main_player_down.bmp");
		player_up = loadTexture("Images/main_player_up.bmp");
		bullet = loadTexture("Images/the_bullet.bmp"); // the bullet!
		enemy_1 = loadTexture("Images/the_enemy.bmp");
		enemy_2 = loadTexture("Images/the_enemy.bmp");

		labyrinth = loadSurface("Images/wbmap.bmp");
		mazeFilling(labyrinth, maze, CELL_SIZE);

		playerPos.x = 40; // the start position of the player on "x" coordinate
		playerPos.y = (HEIGHT / 2) - 100; // the start position of the player on "y" coordinate

		Bullet theBullet = { 0, 0, false, -1 }; // crearing the bullet object

		if (!map) {
			printf("Texture's downloading down...\n");
			return 1;
		}
		else {

			bool isRunning = true;

			while (isRunning) {

				SDL_Event event;

				while (SDL_PollEvent(&event)) {

					if (event.type == SDL_QUIT) {
						isRunning = false; // the first exit out of the game (the main*)
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
					isRunning = false; // the second exit out of the game (added for sure*)
				}

				if (keyStates[SDL_SCANCODE_E] && !theBullet.is_fired) {
					stayingBullet(&theBullet, playerPos, currentPlayer);
				}

				bulletMoving(&theBullet);
				
				if (playerCollision(newX, newY)) {
					playerPos.x = newX;
					playerPos.y = newY;
				}

				SDL_RenderClear(renderer);

				// the map rendering
				SDL_Rect mapRect = { NULL, NULL, WIDTH, HEIGHT };
				SDL_RenderCopy(renderer, map, NULL, &mapRect);

				// the enemy_1 rendering
				SDL_Rect enemyRect_1 = { 250, 20, ENEMY_SIZES, ENEMY_SIZES };
				SDL_RenderCopy(renderer, enemy_1, NULL, &enemyRect_1);

				// the enemy_2 rendering
				SDL_Rect enemyRect_2 = { 995, 640, ENEMY_SIZES, ENEMY_SIZES };
				SDL_RenderCopy(renderer, enemy_2, NULL, &enemyRect_2);

				// the bullet rendering
				if (theBullet.is_fired) {
					SDL_Rect bulletRect = { theBullet.x, theBullet.y, BULLET_SIZE, BULLET_SIZE };
					SDL_RenderCopy(renderer, bullet, NULL, &bulletRect);
				}

				// the player rendering
				SDL_Rect playerRect = { playerPos.x, playerPos.y, PLAYER_WIDTH, PLAYER_HEIGHT };
				SDL_RenderCopy(renderer, currentPlayer, NULL, &playerRect);

				SDL_RenderPresent(renderer); // the updating of the textures on the map

				SDL_Delay(28);
			}
		}
	}

	quit_function(); // the end of the game.

	return 0;
}
