#pragma once

#include "canvas.h"
#include "geometry.h"


struct Vertex
{
	Vec3f position;
	Vec3f normal;
	Vec2f uv;
};

struct V2F {
	Vec4f position;
	Vec3f worldPos;
	Vec3f normal;
	Vec2f uv;
};

class Shader {
public:
	~Shader() {}
	V2F vert(Vertex v);
	TGAColor frag(V2F o);
};



void DrawLine(int x0, int y0, int x1, int y1, Canvas& canvas, const TGAColor& color);
void DrawTriangle(Vec3f t0, Vec3f t1, Vec3f t2, Vec2i* uv, float* zbuffer, Canvas& canvas, TGAImage& img);
/*
[u, v, 1] * [ab.x, ac.x, pa.x] = 0;
[u, v, 1] * [ab.y, ac.y, pa.y] = 0;
即uv1与ab,ac,pa的x轴，y轴都垂直。所以叉乘
要满足“1 >= u >= 0, 1 >= v >= 0, u+v <= 1”，则p在三角形abc中
*/
Vec3f BaryCentric(Vec2i* triangle, Vec2i p);

/*
 * for barycentric coordinates, see
 * http://blackpawn.com/texts/pointinpoly/
 *
 * solve
 *     P = A + s * AB + t * AC  -->  AP = s * AB + t * AC
 * then
 *     s = (AC.y * AP.x - AC.x * AP.y) / (AB.x * AC.y - AB.y * AC.x)
 *     t = (AB.x * AP.y - AB.y * AP.x) / (AB.x * AC.y - AB.y * AC.x)
 *
 * notice
 *     P = A + s * AB + t * AC
 *       = A + s * (B - A) + t * (C - A)
 *       = (1 - s - t) * A + s * B + t * C
 * then
 *     weight_A = 1 - s - t
 *     weight_B = s
 *     weight_C = t
 */
Vec3f BaryCentric(Vec3f* triangle, Vec3f p);

/*
叉乘法
t1 = PA^PB,
t2 = PB^PC,
t3 = PC^PA, 三者同号在三角形内部
*/
bool IsPointInTriangle(Vec2i* triangle, Vec2i p);

bool IsPointInTriangle(Vec3f* triangle, Vec3f p);

void Rasterize(Vertex* vertexes, float* zbuffer, Shader &shader, Canvas& canvas, TGAImage& img);


