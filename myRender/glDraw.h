#pragma once

#include "Canvas.h"
#include "geometry.h"

void DrawLine(int x0, int y0, int x1, int y1, Canvas& canvas, const TGAColor& color);
void DrawTriangle(Vec3f t0, Vec3f t1, Vec3f t2, Vec2i* uv, float* zbuffer, Canvas& canvas, TGAImage& img);
/*
[u, v, 1] * [ab.x, ac.x, pa.x] = 0;
[u, v, 1] * [ab.y, ac.y, pa.y] = 0;
即uv1与ab,ac,pa的x轴，y轴都垂直。所以叉乘
要满足“1 >= u >= 0, 1 >= v >= 0, u+v <= 1”，则p在三角形abc中
*/
Vec3f BaryCentric(Vec2i* triangle, Vec2i p);
Vec3f BaryCentric(Vec3f* triangle, Vec3f p);
/*
叉乘法
t1 = PA^PB,
t2 = PB^PC,
t3 = PC^PA, 三者同号在三角形内部
*/
bool IsPointInTriangle(Vec2i* triangle, Vec2i p);

bool IsPointInTriangle(Vec3f* triangle, Vec3f p);

void DrawTriangle(Vec3f* triangle, float* intensitys, Vec2i* uvs, float* zbuffer, Canvas& canvas, TGAImage& img);