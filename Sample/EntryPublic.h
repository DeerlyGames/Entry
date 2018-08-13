#pragma once

#include "SDL.h"

typedef struct Entry_State
{
	float value;
	SDL_Window* window;
}Entry_State;

static Entry_State state;

static void Startup()
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
		SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
		return;
	}

	state.window = SDL_CreateWindow("Entry Test", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 512, 512, SDL_WINDOW_OPENGL);
}