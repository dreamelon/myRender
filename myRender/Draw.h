#pragma once

#include <sdl.h>
#include "Canvas.h"
#include "geometry.h"
#include <algorithm>
#include <cstdlib>
using std::swap;
using std::abs;
using std::min;
using std::max;

void DrawLine(int x0, int y0, int x1, int y1, Canvas& canvas, const SDL_Color& color) {
	//for (float t = 0.; t < 1.; t += .01) {
	//	int x = x0 + (x1 - x0) * t;
	//	int y = y0 + (y1 - y0) * t;
	//	canvas.SetPixel(color, x, y);
	//}
	//dx > dy?
	bool isSwapXY = false;
	if (abs(x0 - x1) < abs(y0 - y1)) {
		swap(x0, y0);
		swap(x1, y1);
		isSwapXY = true;
	}
	if (x0 > x1) {
		swap(x0, x1);
		swap(y0, y1);
	}

	int dx = x1 - x0;
	int dy = y1 - y0;
	int derror = abs(dy) * 2;
	int error = 0;
	int y = y0;
	for (int x = x0; x <= x1; x++) {
		if (isSwapXY) {
			canvas.SetPixel(color, y, x);
		}
		else {
			canvas.SetPixel(color, x, y);
		}
		error += derror;
		//比较y方向是接近上面像素点还是下面像素点
		if (error > dx) {
			y += y1 > y0 ? 1 : -1;
			error -= 2 * dx;
		}
	}
}

void DrawTriangle(Vec2i t0, Vec2i t1, Vec2i t2, Canvas& canvas, const SDL_Color& color) {

	if (t0.y == t1.y && t1.y == t2.y) return;
	//t.y排序
	if (t0.y > t1.y) {
		swap(t0, t1);
	}
	if (t1.y > t2.y) {
		swap(t1, t2);
		if (t0.y > t1.y) {
			swap(t0, t1);
		}
	}

	//下三角形
	int t0t2_y = t2.y - t0.y;
	int t0t1_y = t1.y - t0.y + 1; //避免除以0
	for (int y = t0.y; y <= t1.y; y++) {
		float t0t2_t = (y - t0.y) / (float)t0t2_y;
		float t0t1_t = (y - t0.y) / (float)t0t1_y;
		int t0t2Bound_x = t0.x + (t2.x - t0.x) * t0t2_t;
		int t0t1Bound_x = t0.x + (t1.x - t0.x) * t0t1_t;
		if (t0t2Bound_x > t0t1Bound_x) swap(t0t2Bound_x, t0t1Bound_x);
		for (int x = t0t2Bound_x; x < t0t1Bound_x; x++) {
			canvas.SetPixel(color, x, y);
		}
		//DrawLine(t0t2Bound_x, y, t0t1Bound_x, y, canvas, color);
	}
	//上三角形
	int t1t2_y = t2.y - t1.y + 1;
	for (int y = t2.y; y > t1.y; y--) {
		float t0t2_t = (t2.y - y) / (float)t0t2_y;
		float t0t1_t = (t2.y - y) / (float)t1t2_y;
		int t0t2Bound_x = t2.x + (t0.x - t2.x) * t0t2_t;
		int t0t1Bound_x = t2.x + (t1.x - t2.x) * t0t1_t;
		//DrawLine(t0t2Bound_x, y, t0t1Bound_x, y, canvas, color);
		if (t0t2Bound_x > t0t1Bound_x) swap(t0t2Bound_x, t0t1Bound_x);
		for (int x = t0t2Bound_x; x < t0t1Bound_x; x++) {
			canvas.SetPixel(color, x, y);
		}
	}
}

/*
[u, v, 1] * [ab.x, ac.x, pa.x] = 0;
[u, v, 1] * [ab.y, ac.y, pa.y] = 0;
即uv1与ab,ac,pa的x轴，y轴都垂直。所以叉乘
要满足“1 >= u >= 0, 1 >= v >= 0, u+v <= 1”，则p在三角形abc中
*/
Vec3f BaryCentric(Vec2i* triangle, Vec2i p) {
	Vec3f u = cross(Vec3f(triangle[2].x - triangle[0].x, triangle[1].x - triangle[0].x, triangle[0].x - p.x),
		Vec3f(triangle[2].y - triangle[0].y, triangle[1].y - triangle[0].y, triangle[0].y - p.y));
	//std::cout << u.z;
 	//因为triangle都是整数，abs(u.z)<1意味着u.z=0,即不构成三角形
	if (abs(u.z) < 1) return Vec3f(-1, 1, 1);
	return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
}

/*
叉乘法
t1 = PA^PB,
t2 = PB^PC,
t3 = PC^PA, 三者同号在三角形内部
*/
bool IsPointInTriangle(Vec2i* triangle, Vec2i p) {
	Vec2i pa = triangle[0] - p;
	Vec2i pb = triangle[1] - p;
	Vec2i pc = triangle[2] - p;

	int t1 = cross(pa, pb);
	int t2 = cross(pb, pc);
	int t3 = cross(pc, pa);

	return t1 * t2 >= 0 && t2 * t3 >= 0;
}


void DrawTriangle(Vec2i* triangle, Canvas& canvas, const SDL_Color& color) {
	//三角形法 判断点是否在三角形内
	//首先找到包围三角形的矩形
	Vec2i bboxmin = Vec2i(canvas.width - 1, canvas.height - 1);
	Vec2i bboxmax = Vec2i(0, 0);
	Vec2i clamp = Vec2i(canvas.width - 1, canvas.height - 1);
	for (int i = 0; i < 3; i++) {
		bboxmax.x = min(clamp.x, max(bboxmax.x, triangle[i].x));
		bboxmax.y = min(clamp.y, max(bboxmax.y, triangle[i].y));
		bboxmin.x = max(0, min(bboxmin.x, triangle[i].x));
		bboxmin.y = max(0, min(bboxmin.y, triangle[i].y));
	}

	Vec2i p;
	for (p.x = bboxmin.x; p.x <= bboxmax.x; p.x++) {
		for (p.y = bboxmin.y; p.y <= bboxmax.y; p.y++) {
			//Vec3f bc_screen = BaryCentric(triangle, p);
			//if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0) continue; 
			//canvas.SetPixel(color, p.x, p.y);

			//if (IsPointInTriangle(triangle, p)) {
			//	canvas.SetPixel(color, p.x, p.y);
			//}
		}
	}

}