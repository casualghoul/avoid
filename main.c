#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include "./constants.h"

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
int game_is_running = FALSE;
int last_frame_time = 0;

struct game_object {
	float x;
	float y;
	float width;
	float height;
	float vel_x;
	float vel_y;

} ball, enemy;

int initialize_window(void) {
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		fprintf(stderr, "Error initializing SDL\n");
		return FALSE;
	}

	window = SDL_CreateWindow(
		NULL,
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		WINDOW_WIDTH,
		WINDOW_HEIGHT,
		SDL_WINDOW_BORDERLESS
	);

	if (!window) {
		fprintf(stderr, "Error creating SDL Window");
		return FALSE;
	}

	renderer = SDL_CreateRenderer(window, -1, 0);
	if (!renderer) {
		fprintf(stderr, "Error creating SDL Renderer");
		return FALSE;
	}

	return TRUE;
}

void process_input() {
	SDL_Event event;
	SDL_PollEvent(&event);

	switch (event.type) {
		//SDL_QUIT is the event that is triggered when you click the "x" button the window
	case SDL_QUIT:
		game_is_running = FALSE;
		break;
		//moving the paddle
	case SDL_KEYDOWN:
		if (event.key.keysym.sym == SDLK_ESCAPE) {
			game_is_running = FALSE;
		}
		if (event.key.keysym.sym == SDLK_LEFT) {
			ball.vel_x = -400;
		}
		if (event.key.keysym.sym == SDLK_RIGHT) {
			ball.vel_x = 400;
		}
		if (event.key.keysym.sym == SDLK_DOWN) {
			ball.vel_y = 400;
		}
		if (event.key.keysym.sym == SDLK_UP) {
			ball.vel_y = -400;
		}
		break;
	case SDL_KEYUP:
		if (event.key.keysym.sym == SDLK_LEFT) {
			ball.vel_x = 0;
		}
		if (event.key.keysym.sym == SDLK_RIGHT) {
			ball.vel_x = 0;
		}
		if (event.key.keysym.sym == SDLK_DOWN) {
			ball.vel_y = 0;
		}
		if (event.key.keysym.sym == SDLK_UP) {
			ball.vel_y = 0;
		}
		// reset ball position when "r" is pressed
		if (event.key.keysym.sym == SDLK_r) {
			ball.x = WINDOW_WIDTH / 2;
			ball.y = WINDOW_HEIGHT / 2;
		}
		break;
	}

}

void setup() {
	//setting the properties of the ball
	ball.width = 15;
	ball.height = 15;
	ball.x = WINDOW_WIDTH / 2;
	ball.y = WINDOW_HEIGHT / 2;

	// setting up the enemy
	enemy.width = 25;
	enemy.height = 25;

	// position the enemy randomly on the screen
	
	// Seed the random number generator with the current timestamp
	srand(time(NULL));

	enemy.x = (float)rand() / (float)(RAND_MAX / WINDOW_WIDTH);
	enemy.y = (float)rand() / (float)(RAND_MAX / WINDOW_HEIGHT);
}

void update(void) {
	
	//logic to keep a fixed timestep

	//number of millseconds that have elapsed since SDL was initialized
	last_frame_time = SDL_GetTicks();

	//waste some time until we reach the frame target time
	//this locks the execution of the program to 30fps
	//this makes sure that each frame takes the same amount of time on screen
	//while (!SDL_TICKS_PASSED(SDL_GetTicks(), last_frame_time + FRAME_TARGET_TIME));

	//get the difference between the frame target time (how long each frame should be) and how long it has been since the last frame(?)
	int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - last_frame_time);
	//delay the execution if we are going too fast to process the frame
	if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME) {
		SDL_Delay(time_to_wait);
	}

	//Get a delta time factor converted to seconds to be used to update my objects
	//This allows us to move objects PER SECOND instead of per frame
	float delta_time = (SDL_GetTicks() - last_frame_time) / 1000.0f;

	// update ball position
	ball.x += ball.vel_x * delta_time;
	ball.y += ball.vel_y * delta_time;

	// Check for ball collision with the walls
	if (ball.x <= 0 || ball.x + ball.width >= WINDOW_WIDTH) {
		ball.vel_x = ball.vel_x;
	}

	if (ball.y > WINDOW_HEIGHT || ball.y + ball.height < 0) {
		ball.vel_y = ball.vel_y;
	}

	//prevent ball from leaving the window
	if (ball.x <= 0) {
		ball.x = 0;
	}
	if (ball.x >= WINDOW_WIDTH - ball.width) {
		ball.x = WINDOW_WIDTH - ball.width;
	}
	if (ball.y <= 0) {
		ball.y = 0;
	}
	if (ball.y >= WINDOW_HEIGHT - ball.height) {
		ball.y = WINDOW_HEIGHT - ball.height;
	}

}

void render() {
	//setting the color you want to render something with
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

	//allows the renderer to use the color
	SDL_RenderClear(renderer);

	//Drawing a ball
	SDL_Rect ball_rect = {
		(int)ball.x,
		(int)ball.y,
		(int)ball.width,
		(int)ball.height
	};
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	SDL_RenderFillRect(renderer, &ball_rect);

	//Drawing the enemy
	SDL_Rect enemy_rect = {
		(int)enemy.x,
		(int)enemy.y,
		(int)enemy.width,
		(int)enemy.height
	};
	SDL_SetRenderDrawColor(renderer, 206, 71, 96, 255);
	SDL_RenderFillRect(renderer, &enemy_rect);

	//The Back Buffer is where you do the rendering logic and build the game objects
	//The Front Buffer is what you display to the user
	//By rendering objects first and then displaying the final product to the user...
	//you prevent flickering and flashing while the objects are being rendered
	//This is called a "Buffer Swap"
	//Here we are performing a buffer swap!
	SDL_RenderPresent(renderer);
}

void destroy_window() {
	//we are destroying things in the reverse order we created them
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

int main(int argc, char* args[]) {

	game_is_running = initialize_window();

	setup();

	//this is the "game loop"
	//each of these things happens every frame
	//one of these functions, like process_input, 
	//could potentially change game_is_running and stop the loop, effectively ending the game
	while (game_is_running) {
		process_input();
		update();
		render();
	}

	return 0;
}