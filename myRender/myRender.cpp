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
const int depth = 2;
Vec3f camera(0, 0, 3);


Matrix viewport(int x, int y, int w, int h) {
	Matrix m = Matrix::identity();
	m[0][3] = x + w / 2.f;
	m[1][3] = y + h / 2.f;
	//m[2][3] = depth / 2.f;

	m[0][0] = w / 2.f;
	m[1][1] = h / 2.f;
	//m[2][2] = depth / 2.f;
	return m;
}

mat<4, 1, float> v2m(Vec3f v) {
	mat<4,1,float> m;
	m[0][0] = v.x;
	m[1][0] = v.y;
	m[2][0] = v.z;
	m[3][0] = 1.f;
	return m;
}

Vec3f m2v(mat<4, 1, float> m) {
	return Vec3f(int(m[0][0] / m[3][0]), int(m[1][0] / m[3][0]), int(m[2][0] / m[3][0]));
}
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
	TGAImage img = model->GetImage();

	float* zbuffer = new float[WINDOW_WIDTH * WINDOW_HEIGHT];
	for (int i = WINDOW_WIDTH * WINDOW_HEIGHT; i--; zbuffer[i] = -std::numeric_limits<float>::max());

	Vec3f light_dir(0, 0, -1);

	Matrix projection = Matrix::identity();
	//视口转换
	Matrix view = viewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	projection[3][2] = -1.f / camera.z;
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
		TGAColor color(255, 255, 255, 255);
		
		//DrawLine(10, 20, 200, 400, *canvas, color);

		Vec2i t0[3] = { Vec2i(10, 70),   Vec2i(50, 160),  Vec2i(70, 80) };
		Vec2i t1[3] = { Vec2i(180, 50),  Vec2i(150, 1),   Vec2i(70, 180) };
		Vec2i t2[3] = { Vec2i(180, 150), Vec2i(120, 160), Vec2i(130, 180) };
		//DrawTriangle(t0, *canvas, color);
		for (int i = 0; i < model->nfaces(); i++) {
			color = TGAColor(255, 255, 255, 255);
			std::vector<Vec3i> face = model->face(i);
			Vec3f screencoords[3];
			Vec3f worldcoords[3];
			Vec2i uvs[3];
			for (int j = 0; j < 3; j++) {
				worldcoords[j] = model->vert(face[j][0]);
				screencoords[j] = m2v(view * (projection * v2m(worldcoords[j])));
				
				//屏幕坐标转为int？？
				//screencoords[j] = Vec3f(int((worldcoords[j].x + 1)*WINDOW_WIDTH/2 +.5), int(WINDOW_HEIGHT - (worldcoords[j].y + 1)*WINDOW_HEIGHT/2 - .5), worldcoords[j].z);
			}

			Vec3f n = cross((worldcoords[2] - worldcoords[0]), (worldcoords[1] - worldcoords[0]));
			n.normalize();
			float intensity = n * light_dir;
			//std::cout << "intensity :" << intensity << std::endl;
			//color.a *= intensity;
			color.r *= intensity;
			color.g *= intensity;
			color.b *= intensity;
			if (intensity > 0) {
				for (int k = 0; k < 3; k++) {
					uvs[k] = model->uv(i, k);
				}
		
				DrawTriangle(screencoords, uvs, zbuffer, *canvas, img);
				//DrawTriangle(screencoords[0], screencoords[1], screencoords[2], uvs, zbuffer, *canvas, img);
			}	
			
		}
		SDL_SetRenderDrawColor(render, 0xFF, 0xFF, 0xFF, 0xFF);
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
