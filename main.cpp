#define _USE_MATH_DEFINES
#include<math.h>
#include<stdio.h>
#include<string.h>
#include <iostream>
#include <cstdlib> 
#include <ctime>   
#include <string>
#include <cmath>

using namespace std;

extern "C" {
#include"./SDL2-2.0.10/include/SDL.h"
#include"./SDL2-2.0.10/include/SDL_main.h"
}

#define SCREEN_WIDTH	500
#define SCREEN_HEIGHT	500

#define FLAPPY_MOVING_UP_TIME 0.4
#define PIPE_STARTING_POS_X 540
#define PIPES_NUMBER 4
#define PIPES_SPEED 0.5
#define PIPE_STARTING_SECOND_POS_X 830;
#define PIPE_RESET_POS_X -40

#define PIPE_HEIGHT 300
#define PIPE_WIDTH 90
#define FLAPPY_HEIGHT 65
#define FLAPPY_WIDTH 85

#define POINTS_VALUE 1000
#define FLAPPY_ASCENDING_SPEED 0.2
#define FLAPPY_DESCENDING_SPEED 0.25


struct Position {
	double posX, posY;
};

struct Border {
	double top, right, down, left;
};

struct FlappyBird {
	Position pos;
	SDL_Surface* icon;
	Border border;
};

struct Pipe {
	Position pos;
	SDL_Surface* icon;
	Border border;
	bool marked;
};



void DrawString(SDL_Surface *screen, int x, int y, const char *text,
                SDL_Surface *charset) {
	int px, py, c;
	SDL_Rect s, d;
	s.w = 8;
	s.h = 8;
	d.w = 8;
	d.h = 8;
	while(*text) {
		c = *text & 255;
		px = (c % 16) * 8;
		py = (c / 16) * 8;
		s.x = px;
		s.y = py;
		d.x = x;
		d.y = y;
		SDL_BlitSurface(charset, &s, screen, &d);
		x += 8;
		text++;
		};
	};


void DrawSurface(SDL_Surface *screen, SDL_Surface *sprite, int x, int y) {
	SDL_Rect dest;
	dest.x = x - sprite->w / 2;
	dest.y = y - sprite->h / 2;
	dest.w = sprite->w;
	dest.h = sprite->h;
	SDL_BlitSurface(sprite, NULL, screen, &dest);
	};


void DrawPixel(SDL_Surface *surface, int x, int y, Uint32 color) {
	int bpp = surface->format->BytesPerPixel;
	Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
	*(Uint32 *)p = color;
	};


void DrawLine(SDL_Surface *screen, int x, int y, int l, int dx, int dy, Uint32 color) {
	for(int i = 0; i < l; i++) {
		DrawPixel(screen, x, y, color);
		x += dx;
		y += dy;
		};
	};


void DrawRectangle(SDL_Surface *screen, int x, int y, int l, int k,
                   Uint32 outlineColor, Uint32 fillColor) {
	int i;
	DrawLine(screen, x, y, k, 0, 1, outlineColor);
	DrawLine(screen, x + l - 1, y, k, 0, 1, outlineColor);
	DrawLine(screen, x, y, l, 1, 0, outlineColor);
	DrawLine(screen, x, y + k - 1, l, 1, 0, outlineColor);
	for(i = y + 1; i < y + k - 1; i++)
		DrawLine(screen, x + 1, i, l - 2, 1, 0, fillColor);
	};


double moveAscending() {
	return FLAPPY_ASCENDING_SPEED;
}


double moveDescending() {
	return -FLAPPY_DESCENDING_SPEED;
}


void initializePipes(Pipe* pipes) {
	for (int i = 0; i < PIPES_NUMBER; ++i) {
		if (i % 2 == 0)
			pipes[i].icon = SDL_LoadBMP("./Pipe.bmp");
		else
			pipes[i].icon = SDL_LoadBMP("./PipeTwo.bmp");

	}
}


void randomizePipe(Pipe* pipeOne, Pipe* pipeTwo, bool startingSecond) {
	if (!startingSecond) {
		pipeOne->pos.posX = PIPE_STARTING_POS_X;
		pipeTwo->pos.posX = PIPE_STARTING_POS_X;
	}
	else {
		pipeOne->pos.posX = PIPE_STARTING_SECOND_POS_X;
		pipeTwo->pos.posX = PIPE_STARTING_SECOND_POS_X;
	}

	double randomPosY = 400 + std::rand() % (500 - 350 + 1);

	pipeTwo->pos.posY = randomPosY;
	pipeOne->pos.posY = randomPosY - 450;

	pipeOne->marked = false;
	pipeTwo->marked = false;
}


void checkResetPipes(Pipe* pipeOne, Pipe* pipeTwo) {
	if (pipeOne->pos.posX < PIPE_RESET_POS_X && pipeTwo->pos.posX < PIPE_RESET_POS_X)
		randomizePipe(pipeOne, pipeTwo, false);
}


void movePipes(Pipe* pipes) {
	for (int i = 0; i < PIPES_NUMBER; ++i) {
		pipes[i].pos.posX -= PIPES_SPEED;
	}
}


void checkDeath(FlappyBird flappy, int *quit) {
	if (flappy.pos.posY > SCREEN_HEIGHT || flappy.pos.posY < 0)
		*quit = 1;
}


void calculateAxis(FlappyBird* flappy, Pipe* pipes) {
	for (int i = 0; i < PIPES_NUMBER; ++i) {

		if (i % 2 == 0) {
			pipes[i].border.down = pipes[i].pos.posY - (PIPE_HEIGHT / 2);
			pipes[i].border.top = pipes[i].pos.posY + (PIPE_HEIGHT / 2);
		}
		else {
			pipes[i].border.top = pipes[i].pos.posY - (PIPE_HEIGHT / 2);
			pipes[i].border.down = pipes[i].pos.posY + (PIPE_HEIGHT / 2);
		}
		pipes[i].border.left = pipes[i].pos.posX - (PIPE_WIDTH / 2);
		pipes[i].border.right = pipes[i].pos.posX + (PIPE_WIDTH / 2);
	}

	flappy->border.top = flappy->pos.posY - (FLAPPY_HEIGHT / 2);
	flappy->border.down = flappy->pos.posY + (FLAPPY_HEIGHT / 2);
	flappy->border.left = flappy->pos.posX - (FLAPPY_WIDTH / 2);
	flappy->border.right = flappy->pos.posX + (FLAPPY_WIDTH / 2);

}


void checkColission(FlappyBird* flappy, Pipe* pipes, int* quit) {
	for (int i = 0; i < PIPES_NUMBER; ++i) {

		if (i % 2 == 0) {
			if (flappy->border.right > pipes[i].border.left && flappy->border.top > pipes[i].border.down && flappy->border.down < pipes[i].border.top) {
				*quit = 1;
			}
			else if (flappy->border.top < pipes[i].border.top && flappy->border.right < pipes[i].border.right && flappy->border.left > pipes[i].border.left)
				*quit = 1;
		}
		else{
			if (flappy->border.right > pipes[i].border.left && flappy->border.top > pipes[i].border.top && flappy->border.down < pipes[i].border.down) {
				*quit = 1;
			}
			else if (flappy->border.down > pipes[i].border.top && flappy->border.right < pipes[i].border.right && flappy->border.left > pipes[i].border.left)
				*quit = 1;
		}
	}
}



int countPoints(FlappyBird flappy, Pipe *pipes) {
	for (int i = 0; i < PIPES_NUMBER; ++i) {
		if (i % 2 == 1 || pipes[i].marked)
			continue;

		if (flappy.border.top > pipes[i].border.top && flappy.border.down < pipes[i + 1].border.top) {
			if (flappy.border.left > pipes[i].border.left && flappy.border.right < pipes[i].border.right) {
				return POINTS_VALUE;
				pipes[i].marked = true;
				pipes[i + 1].marked = true;

			}
		}
	}

	return 0;
}



// main
#ifdef __cplusplus
extern "C"
#endif
int main(int argc, char **argv) {
	int t1, t2, quit, frames, rc;
	double delta, worldTime, fpsTimer, fps, etiSpeed;
	SDL_Event event;
	SDL_Surface *screen, *charset;
	SDL_Texture *scrtex;
	SDL_Window *window;
	SDL_Renderer *renderer;

	srand(time(0));

	FlappyBird flappy;
	flappy.icon = SDL_LoadBMP("./eti.bmp");

	Pipe pipeArray[PIPES_NUMBER];

	initializePipes(pipeArray);
	randomizePipe(&pipeArray[0], &pipeArray[1], false);
	randomizePipe(&pipeArray[2], &pipeArray[3], true);


	if(SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("SDL_Init error: %s\n", SDL_GetError());
		return 1;
		}

	rc = SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0,
	                                 &window, &renderer);
	if(rc != 0) {
		SDL_Quit();
		printf("SDL_CreateWindowAndRenderer error: %s\n", SDL_GetError());
		return 1;
		};
	
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

	SDL_SetWindowTitle(window, "Flappy Bird");


	screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32,
	                              0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

	scrtex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
	                           SDL_TEXTUREACCESS_STREAMING,
	                           SCREEN_WIDTH, SCREEN_HEIGHT);


	charset = SDL_LoadBMP("./cs8x8.bmp");
	SDL_Surface* background;
	background = SDL_LoadBMP("./bg.bmp");

	SDL_Surface* start;
	start = SDL_LoadBMP("./start.bmp");

	SDL_SetColorKey(charset, true, 0x000000);

	char text[128];
	int black = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);


	t1 = SDL_GetTicks();

	frames = 0;
	fpsTimer = 0;
	fps = 0;
	quit = 0;
	worldTime = 0;
	etiSpeed = 1;

	
	flappy.pos.posX = 100;
	flappy.pos.posY = 300;

	double newTime = 0;
	bool moving = false;
	double flappy_time = 999;

	int points = 0;

	bool gameStarted = false;
	bool resetTime = false;

	while(!quit) {

		if (!gameStarted) {
			SDL_FillRect(screen, NULL, black);
			DrawSurface(screen, background, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 1);
			DrawSurface(screen, start, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 1);
			resetTime = true;
		}

		if (gameStarted) {
			t2 = SDL_GetTicks();
			delta = (t2 - t1) * 0.001;
			t1 = t2;

			worldTime += delta;

			if (resetTime) {
				worldTime = 0;
				resetTime = false;
			}

			SDL_FillRect(screen, NULL, black);
			DrawSurface(screen, background, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 1);


			if (flappy_time < FLAPPY_MOVING_UP_TIME) {
				flappy.pos.posY -= moveAscending();
				flappy_time += delta;
			}
			else {
				flappy.pos.posY -= moveDescending();
			}

			movePipes(pipeArray);
			checkResetPipes(&pipeArray[0], &pipeArray[1]);
			checkResetPipes(&pipeArray[2], &pipeArray[3]);
			calculateAxis(&flappy, pipeArray);
			checkColission(&flappy, pipeArray, &quit);

			points += countPoints(flappy, pipeArray);

			DrawSurface(screen, flappy.icon, flappy.pos.posX, flappy.pos.posY);
			DrawSurface(screen, pipeArray[0].icon, pipeArray[0].pos.posX, pipeArray[0].pos.posY);
			DrawSurface(screen, pipeArray[1].icon, pipeArray[1].pos.posX, pipeArray[1].pos.posY);
			DrawSurface(screen, pipeArray[2].icon, pipeArray[2].pos.posX, pipeArray[2].pos.posY);
			DrawSurface(screen, pipeArray[3].icon, pipeArray[3].pos.posX, pipeArray[3].pos.posY);

			checkDeath(flappy, &quit);

			sprintf(text, "Time = %.1lf s", worldTime);
			DrawString(screen, 125, 10, text, charset);
			sprintf(text, "Score = %d", points);
			DrawString(screen, 275, 10, text, charset);
		}

		SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
		SDL_RenderCopy(renderer, scrtex, NULL, NULL);
		SDL_RenderPresent(renderer);
		

		while(SDL_PollEvent(&event)) {
			switch(event.type) {
				case SDL_MOUSEBUTTONDOWN:
					if (event.button.button == SDL_BUTTON_LEFT) {
						flappy_time = 0;
						gameStarted = true;
					}
					break;
				case SDL_QUIT:
					quit = 1;
					break;
				};
			};
		frames++;
		};

	SDL_FreeSurface(charset);
	SDL_FreeSurface(screen);
	SDL_DestroyTexture(scrtex);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	SDL_Quit();
	return 0;
};
