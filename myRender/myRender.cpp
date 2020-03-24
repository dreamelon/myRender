// myRender.cpp : This file contains the 'main' function. Program execution begins and ends there.


#include <iostream>
#include <SDL.h>
#include <SDL_image.h>
#include "canvas.h"
#include "model.h"        
#include "rasterize.h"
#include "camera.h"
#include <windows.h>

using std::cout;
using std::endl;
using std::abs;
using std::swap;

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 800;
const int depth = 2;

Vec3f center(0, 0, 0);
//Vec3f camera(0, 0, 3);
Vec3f up(0, 1, 0);
Vec3f light_dir(0, 0, -1);

/*
width    0    0   0			 1	0	0	1   
0     height  0   0    *     0  1	0	1		
0        0    1   0			 0	0	1	0
0        0	  0   1			 0	0	0	1
*/

Matrix ViewPort(int x, int y, int w, int h) {
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
	if (m[3][0] < 0) {
		std::cout << "wrong\n";
	}
	return Vec3f((m[0][0] / m[3][0]), (m[1][0] / m[3][0]), (m[2][0] / m[3][0]));
}

Vec4f v2norm(mat<4, 1, float> m) {
	Vec4f v;
	v[0] = int(m[0][0] / m[3][0]);
	v[1] = int(m[1][0] / m[3][0]);
	v[2] = int(m[2][0] / m[3][0]);
	v[3] = 1;
	return v;
}


/* misc platform functions */

static double get_native_time(void) {
	static double period = -1;
	LARGE_INTEGER counter;
	if (period < 0) {
		LARGE_INTEGER frequency;
		QueryPerformanceFrequency(&frequency);
		period = 1 / (double)frequency.QuadPart;
	}
	QueryPerformanceCounter(&counter);
	return counter.QuadPart * period;
}

float platform_get_time(void) {
	static double initial = -1;
	if (initial < 0) {
		initial = get_native_time();
	}
	return (float)(get_native_time() - initial);
}


bool firstMouse = true;
float c_lastX;
float c_lastY;


Camera camera(Vec3f(0.f, 0.f, 5.f));

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
	//Model* model = new Model("obj/cyborg.obj");
	TGAImage img = model->GetImage();

	float* zbuffer = new float[WINDOW_WIDTH * WINDOW_HEIGHT];
	
	Matrix projection = camera.projection();
	//视口转换
	Matrix viewport = ViewPort(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

	Shader shader;

	SDL_Event event;
	bool isRightMouseDown = false;
	//RenderingLoop
	bool quit = true;

	double lastTime = get_native_time();
	int nFrames = 0;
	int xFrames = 0;

	Uint64 start, now;
	motion_t motion;
	while (quit) {

		start = SDL_GetPerformanceCounter();

		memset(canvas->pixelData, 0, canvas->width * canvas ->height * sizeof(Uint32));
		
		for (int i = WINDOW_WIDTH * WINDOW_HEIGHT; i--; zbuffer[i] = -FLT_MAX);

		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				quit = false;
			}

			if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_RIGHT) {				
				c_lastX = event.motion.x;
				c_lastY = event.motion.y;
				isRightMouseDown = true;
			}
			
			if (event.type == SDL_MOUSEBUTTONUP) {
				isRightMouseDown = false;
			}
			if (isRightMouseDown && event.type == SDL_MOUSEMOTION)
			{
				//cout << "before c_lastX : " << c_lastX << endl;
				//cout << "before c_lasty : " << c_lastY << endl;
				float xoffset = (event.motion.x - c_lastX) / WINDOW_HEIGHT;
				float yoffset = (event.motion.y - c_lastY) / WINDOW_HEIGHT;

				
				motion.pan = Vec2f(xoffset, yoffset);
				camera.UpdateCameraPan(motion);

				c_lastX = event.motion.x;
				c_lastY = event.motion.y; 
			}

			if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
				c_lastX = event.motion.x;
				c_lastY = event.motion.y;
			}

			if (event.button.button == SDL_BUTTON_LEFT && event.type == SDL_MOUSEMOTION)
			{
				float xdelta = (event.motion.x - c_lastX) / WINDOW_HEIGHT;
				float ydelta = (event.motion.y - c_lastY) / WINDOW_HEIGHT;

				//motion_t motion;
				motion.dolly = 0.0;
				motion.orbit = Vec2f(xdelta, ydelta);
				camera.UpdateCameraOffset(motion);

				c_lastX = event.motion.x;
				c_lastY = event.motion.y;
			}
		}

		Matrix view = camera.LookAt();
		// canvas setpixel draw sth
		TGAColor color(255, 255, 255, 255);

		Vec3f pos[6] = {Vec3f(-0.5f, -0.5f, -0.5f),   Vec3f(0.5f, 0.5f, -0.5f),  
						Vec3f(0.5f, -0.5f, -0.5f),	  Vec3f(0.5f, 0.5f, -0.5f),
						Vec3f(-0.5f, -0.5f, -0.5f),   Vec3f(-0.5f, 0.5f, -0.5f)
		};
		

		Vec3f norm[6] = { Vec3f(0.0f, 0.0f, -1.0f),   Vec3f(0.0f, 0.0f, -1.0f),
						Vec3f(0.0f, 0.0f, -1.0f),	  Vec3f(0.0f, 0.0f, -1.0f),
						Vec3f(0.0f, 0.0f, -1.0f),   Vec3f(0.0f, 0.0f, -1.0f)
		};

		Vec2f texcoords[6] = { Vec2f(0.0f, 0.0f),   Vec2f(1.0f, 0.0f),
						Vec2f(1.0f, 1.0f),	  Vec2f(1.0f, 1.0f),
						Vec2f(0.0f, 1.0f),   Vec2f(0.0f, 0.0f)
		};
		//Vertex v[6];
		//for (int i = 0; i < 6; i++) {
		//	v[i].position = 
		//}
		

		//DrawTriangle(t0, *canvas, color);
		for (int i = 0; i < model->nfaces(); i++) {
			color = TGAColor(255, 255, 255, 255);
			std::vector<Vec3i> face = model->face(i);
			Vertex vert[3];
			//Vec3f screencoords[3];
			//Vec3f worldcoords[3];
			//Vec3f normals[3];
			//Vec2f uvs[3];
			for (int j = 0; j < 3; j++) {
				vert[j].position = model->vert(face[j][0]);
				//vert[j].uv = model->uv(i, j);
				//uvs[j] = model->uv(i, j);
				//worldcoords[j] = model->vert(face[j][0]);
				vert[j].position = m2v(viewport * projection * view * v2m(vert[j].position));

				//shader.vert();

				//screencoords[j] = m2v(viewport * projection * view * v2m(worldcoords[j]));

				vert[j].position.y = WINDOW_HEIGHT - vert[j].position.y;
				//screencoords[j].y = WINDOW_HEIGHT - screencoords[j].y;
			}	

			//Vec3f n = cross((worldcoords[2] - worldcoords[0]), (worldcoords[1] - worldcoords[0]));
			//n.normalize();
			//float intensity = n * light_dir;

			//if (intensity > 0) {
				Rasterize(vert, zbuffer, shader, *canvas, img);
				//DrawTriangle(screencoords[0], screencoords[1], screencoords[2], uvs, zbuffer, *canvas, img);
			//}	
			
		}
		//SDL_SetRenderDrawColor(render, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(render);

		void* pixels;
		int pitch = 0;
		SDL_LockTexture(renderTexture, NULL, &pixels, &pitch);
		//destination, source, 第三个参数表示要复制的字节数
		memcpy(pixels, canvas->pixelData, canvas->height * canvas->width * 4);
		SDL_UnlockTexture(renderTexture);
		
		SDL_RenderCopy(render, renderTexture, NULL, NULL);

		SDL_RenderPresent(render);


		double currentTime = get_native_time();
		nFrames++;
		if (currentTime - lastTime >= 1.0) { // If last prinf() was more than 1 sec ago
		// printf and reset timer
			//printf("%f ms/frame\n", 1000.0 / double(nFrames));
			//printf("%d fps\n", nFrames);
			nFrames = 0;
			lastTime += 1.0;
		}

		now = SDL_GetPerformanceCounter();
		//xFrames++;
		//if (now - start >= 1.0) {
		//printf("%f ms\n", (now - start) * 1000.0 / SDL_GetPerformanceFrequency());
		//}

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


