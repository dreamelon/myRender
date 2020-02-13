#pragma once

#include <SDL.h>

class Canvas {
	
	SDL_Rect rect;
public:	Uint32* pixelData;
		int height, width;
	
public: Canvas(int _w, int _h) : width(_w), height(_h) {
	rect.x = 0;
	rect.y = 0;
	rect.h = height;
	rect.w = width;
	pixelData = new Uint32[width * height];
}

public: bool SetPixel(const SDL_Color& color, int x, int y) {
	if (!pixelData || x < 0 || y < 0 || x >= width || y >= height) {
		return false;
	}
	else {
		pixelData[x + y * width] = color.r << 24 | color.g << 16 | color.b << 8 | color.a;
		return true;
	}
}

};