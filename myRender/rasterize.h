#pragma once

#include "canvas.h"
#include "geometry.h"
#include "Color.h"

class Shader {
public:
	~Shader() {}
	virtual	V2F vert(A2V v) = 0;
	virtual Color frag(V2F o) = 0;
};

class TextureShader : public Shader {
public:
	Matrix model;
	Matrix view;
	Matrix projection;
	virtual	V2F vert(A2V v);
	virtual Color frag(V2F o);
};

void DrawLine(int x0, int y0, int x1, int y1, Canvas& canvas, const Color& color);

void DrawTriangle(Vec3f t0, Vec3f t1, Vec3f t2, Vec2i* uv, float* zbuffer, Canvas& canvas, TGAImage& img);

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
Vec3f BaryCentric(Vec3f* triangle, Vec2i p);

/*
叉乘法
t1 = PA^PB,
t2 = PB^PC,
t3 = PC^PA, 三者同号在三角形内部
*/
bool IsPointInTriangle(Vec3f* triangle, Vec3i p);

void Rasterize(V2F* vertexes, float* zbuffer, Shader& shader, Canvas& canvas, TGAImage& img);

int Encode(Vec2f pos, Vec2f min, Vec2f max);

bool CohenSutherlandLineClip(Vec2f start, Vec2f end, Vec2f min, Vec2f max);