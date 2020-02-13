// myRender.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

//#define SDL_MAIN_HANDLED
#include <iostream>
#include <SDL.h>
#include <SDL_image.h>
#include "Canvas.h"
#include "Model.h"
#include "Draw.h"
using std::cout;
using std::endl;
using std::abs;
using std::swap;

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 800;

//void DrawLine(int x0, int y0, int x1, int y1, Canvas& canvas, SDL_Color color);

int main(int argc, char* argv[])
{
	// Initialize SDL. SDL_Init will return -1 if it fails.
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		cout << "Error initializing SDL: " << SDL_GetError() << endl;
		return 1;
	}


	SDL_Window* window = SDL_CreateWindow("MyRender", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
										  WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
	if (!window) {
		cout << "Error creating window: " << SDL_GetError() << endl;
		return 1;
	}

	SDL_Renderer* render = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (!render) {
		printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
		return 1;
	} 
	else
	{
		// Initialize renderer color
		SDL_SetRenderDrawColor(render, 0x29, 0x6A, 0x54, 0xFF);

		// Initialize PNG loading
		int imgFlags = IMG_INIT_PNG;
		if (!(IMG_Init(imgFlags) & imgFlags))
		{
			printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
			
		}
	}

	//SDL_Surface* winSurface = SDL_GetWindowSurface(window);
	//// Make sure getting the surface succeeded
	//if (!winSurface) {
	//	cout << "Error getting surface: " << SDL_GetError() << endl;
	//	return 1;

	//}

	SDL_Texture* renderTexture = SDL_CreateTexture(render, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING,
												   WINDOW_WIDTH, WINDOW_HEIGHT);
	Canvas* canvas = new Canvas(WINDOW_WIDTH, WINDOW_HEIGHT);
	
	Model* model = new Model("obj/african_head.obj");
	//RenderingLoop
	bool quit = true;
	while (quit) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				quit = false;
			}
		}
		// canvas setpixel draw sth
		SDL_Color color;
		color.r = 225;
		color.g = 0;
		color.b = 224;
		color.a = 255;
		DrawLine(10, 20, 200, 400, *canvas, color);

		Vec2i t0[3] = { Vec2i(10, 70),   Vec2i(50, 160),  Vec2i(70, 80) };
		Vec2i t1[3] = { Vec2i(180, 50),  Vec2i(150, 1),   Vec2i(70, 180) };
		Vec2i t2[3] = { Vec2i(180, 150), Vec2i(120, 160), Vec2i(130, 180) };
		DrawTriangle(t0, *canvas, color);
		for (int i = 0; i < model->nfaces(); i++) {
			std::vector<int> face = model->face(i);
			Vec2i screencoords[3];
			for (int j = 0; j < 3; j++) {
				Vec3f worldcoords = model->vert(face[j]);
				screencoords[j] = Vec2i((worldcoords.x + 1)*WINDOW_WIDTH/2, WINDOW_HEIGHT - (worldcoords.y + 1)*WINDOW_HEIGHT/2 );
			}
			DrawTriangle(screencoords, *canvas, color);
			
		}

		SDL_RenderClear(render);
		void* pixels;
		int pitch = 0;
		SDL_LockTexture(renderTexture, NULL, &pixels, &pitch);
		//destination, source, 第三个参数表示要复制的字节数
		memcpy(pixels, canvas->pixelData, canvas->height * canvas->width * 4);
		SDL_UnlockTexture(renderTexture);
		
		SDL_RenderCopy(render, renderTexture, NULL, NULL);

		SDL_RenderPresent(render);

	}

	//SDL_FillRect(winSurface, NULL, SDL_MapRGB(winSurface->format, 255, 90, 120));

	//SDL_UpdateWindowSurface(window);
	//SDL_Delay(3000);

	SDL_DestroyTexture(renderTexture);
	SDL_DestroyRenderer(render);	
	SDL_DestroyWindow(window);
	
	SDL_Quit();

	return 0;
}




// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
