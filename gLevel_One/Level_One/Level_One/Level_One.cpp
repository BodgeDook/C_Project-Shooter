#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>
#include <stdbool.h>

const int WIDTH = 1080;
const int HEIGHT = 720;
const int PLAYER_WIDTH = 60; // final value!
const int PLAYER_HEIGHT = 60; // final value!

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

SDL_Surface* labyrinth = NULL; // the labyrinth's surface
const int CELL_SIZE = 1; // the size of one map's cell (in pixels)
int maze[720][1080]; // the labyrinth's maze

typedef struct Player_Position {
	int x;
	int y;
} Player_Position;

Player_Position playerPos; // the position of the player on the game's map

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
void analyzeMapAndFillMaze(SDL_Surface* labirinth, int maze[720][1080], int cellSize) {

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

bool canMoveTo(int PosX, int PosY) {

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
	SDL_DestroyTexture(enemy_1);
	SDL_DestroyTexture(enemy_2);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_FreeSurface(labyrinth);
	labyrinth = NULL;
	currentPlayer = player_right = player_left = player_down = player_up = map = enemy_1 = enemy_2 = NULL;
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

		// the main textures' loadings:
		map = loadTexture("Images/the_map.bmp");
		currentPlayer = loadTexture("Images/main_player_right.bmp");
		player_right = loadTexture("Images/main_player_right.bmp");
		player_left = loadTexture("Images/main_player_left.bmp");
		player_down = loadTexture("Images/main_player_down.bmp");
		player_up = loadTexture("Images/main_player_up.bmp");
		enemy_1 = loadTexture("Images/the_enemy.bmp");
		enemy_2 = loadTexture("Images/the_enemy.bmp");

		labyrinth = loadSurface("Images/wbmap.bmp");
		analyzeMapAndFillMaze(labyrinth, maze, CELL_SIZE);

		playerPos.x = 40; // the start position of the player on "x" coordinate
		playerPos.y = (HEIGHT / 2) - 100; // the start position of the player on "y" coordinate

		if (!map) {
			printf("Texture's downloading down...\n");
			return 1;
		}
		else {

			bool isRunning = true;
			SDL_Event event;

			while (isRunning) {

				while (SDL_PollEvent(&event) != 0) {

					if (event.type == SDL_QUIT) {
						isRunning = false; // the first exit out of the game (the main*)
					}

					else if (event.type == SDL_KEYDOWN) {

						int newX = playerPos.x;
						int newY = playerPos.y;

						switch (event.key.keysym.sym) {

						case SDLK_q:
							isRunning = false; // the second exit out of tha game (added*)
							break;
						case SDLK_w:
							newY -= 10;
							currentPlayer = player_up;
							break;
						case SDLK_s:
							newY += 10;
							currentPlayer = player_down;
							break;
						case SDLK_a:
							newX -= 10;
							currentPlayer = player_left;
							break;
						case SDLK_d:
							newX += 10;
							currentPlayer = player_right;
							break;
						default:
							// just nothing...
							break;
						}

						if (canMoveTo(newX, newY)) {
							playerPos.x = newX;
							playerPos.y = newY;
						}
					}
				}

				SDL_RenderClear(renderer);

				// the map rendering
				SDL_Rect mapRect = { NULL, NULL, WIDTH, HEIGHT };
				SDL_RenderCopy(renderer, map, NULL, &mapRect);

				// the enemy_1 rendering
				SDL_Rect enemyRect_1 = { 250, 20, 60, 60 };
				SDL_RenderCopy(renderer, enemy_1, NULL, &enemyRect_1);

				// the enemy_2 rendering
				SDL_Rect enemyRect_2 = { 995, 640, 60, 60 };
				SDL_RenderCopy(renderer, enemy_2, NULL, &enemyRect_2);

				// the player rendering
				SDL_Rect playerRect = { playerPos.x, playerPos.y, PLAYER_WIDTH, PLAYER_HEIGHT };
				SDL_RenderCopy(renderer, currentPlayer, NULL, &playerRect);


				SDL_RenderPresent(renderer); // the updating of the textures on the map
			}
		}
	}

	quit_function(); // the end of the game...

	return 0;
}
