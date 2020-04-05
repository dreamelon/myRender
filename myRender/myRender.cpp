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
Vec4f ViewPort(Vec4f v) {
	Matrix m = Matrix::identity();
	m[0][3] = WINDOW_WIDTH / 2.f;
	m[1][3] = WINDOW_HEIGHT / 2.f;
	//m[2][3] = depth / 2.f;

	m[0][0] = WINDOW_WIDTH / 2.f;
	m[1][1] = WINDOW_HEIGHT / 2.f;
	//m[2][2] = depth / 2.f;
	float rhw = v.w;
	v.w = 1.f;
	Vec4f temp = m * v;
	temp.w = rhw;
	return temp;
}

//透视除法
Vec4f PerspectiveDivision(Vec4f m) {
	Vec4f v;
	float rhw = 1 / m[3];
	v[0] = m[0] * rhw;
	v[1] = m[1] * rhw;
	v[2] = m[2] * rhw;
	v[3] = rhw;
	return v;
}

/*
 * for facing determination, see subsection 3.5.1 of
 * https://www.khronos.org/registry/OpenGL/specs/es/2.0/es_full_spec_2.0.pdf
 *
 * this is the same as (but more efficient than)
 *     vec3_t ab = vec3_sub(b, a);
 *     vec3_t ac = vec3_sub(c, a);
 *     return vec3_cross(ab, ac).z <= 0;
 */
static int is_back_facing(Vec3f ndc_coords[3]) {
	Vec3f  a = ndc_coords[0];
	Vec3f  b = ndc_coords[1];
	Vec3f  c = ndc_coords[2];
	float signed_area = 0;
	signed_area += a.x * b.y - a.y * b.x;
	signed_area += b.x * c.y - b.y * c.x;
	signed_area += c.x * a.y - c.y * a.x;
	return signed_area <= 0;
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

	TextureShader shader;
	shader.projection = camera.projection();
	shader.model = Matrix::identity();

	SDL_Event event;
	bool isRightMouseDown = false;
	//RenderingLoop
	bool quit = true;

	double lastTime = get_native_time();
	int nFrames = 0;
	int xFrames = 0;

	bool isBackfaceCull = true;

	Uint64 start, now;
	motion_t motion;
	while (quit) {

		start = SDL_GetPerformanceCounter();

		memset(canvas->pixelData, 0, canvas->width * canvas ->height * sizeof(Uint32));
		
		//每次清空zbuffer
		for (int i = WINDOW_WIDTH * WINDOW_HEIGHT; i--; zbuffer[i] = FLT_MAX);

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

		shader.view = camera.LookAt();

		// canvas setpixel draw sth
		Color color(200, 100, 255, 255);
		DrawLine(-200, -100, 200, 200, *canvas, color);
		//Vec3f pos[6] = {Vec3f(-0.5f, -0.5f, -0.5f),   Vec3f(0.5f, 0.5f, -0.5f),  
		//				Vec3f(0.5f, -0.5f, -0.5f),	  Vec3f(0.5f, 0.5f, -0.5f),
		//				Vec3f(-0.5f, -0.5f, -0.5f),   Vec3f(-0.5f, 0.5f, -0.5f)
		//};
		//

		//Vec3f norm[6] = { Vec3f(0.0f, 0.0f, -1.0f),   Vec3f(0.0f, 0.0f, -1.0f),
		//				Vec3f(0.0f, 0.0f, -1.0f),	  Vec3f(0.0f, 0.0f, -1.0f),
		//				Vec3f(0.0f, 0.0f, -1.0f),   Vec3f(0.0f, 0.0f, -1.0f)
		//};

		//Vec2f texcoords[6] = { Vec2f(0.0f, 0.0f),   Vec2f(1.0f, 0.0f),
		//				Vec2f(1.0f, 1.0f),	  Vec2f(1.0f, 1.0f),
		//				Vec2f(0.0f, 1.0f),   Vec2f(0.0f, 0.0f)
		//};
		
		
		//DrawTriangle(t0, *canvas, color);
		for (int i = 0; i < model->nfaces(); i++) {
			std::vector<Vec3i> face = model->face(i);
			A2V	 a2v[3];
			V2F  v2f[3];
			for (int j = 0; j < 3; j++) {
				a2v[j].position = model->vert(face[j][0]);
				a2v[j].uv = model->uv(i, j);
				a2v[j].normal = model->norm(i, j);
				//vert[j].position = projection * view * Vec4f(vert[j].position, 1.f);
				//顶点着色器
				v2f[j] = shader.vert(a2v[j]);
				v2f[j].position = ViewPort(PerspectiveDivision(v2f[j].position));
				//v2f[j].position.y = WINDOW_HEIGHT - v2f[j].position.y;
			}	
			//背面剔除
			Vec3f ndcCoords[3]{v2f[0].position, v2f[1].position, v2f[2].position};
			int backface = is_back_facing(ndcCoords);
			if (backface && isBackfaceCull) continue;

			//翻转y轴，满足屏幕坐标系
			for (int j = 0; j < 3; j++) {
				v2f[j].position.y = WINDOW_HEIGHT - v2f[j].position.y;
			}
			//光栅化
			Rasterize(v2f, zbuffer, shader, *canvas, img);		
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



