#pragma once

#include "Canvas.h"
#include "geometry.h"

void DrawLine(int x0, int y0, int x1, int y1, Canvas& canvas, const TGAColor& color);
void DrawTriangle(Vec3f t0, Vec3f t1, Vec3f t2, Vec2i* uv, float* zbuffer, Canvas& canvas, TGAImage& img);
/*
[u, v, 1] * [ab.x, ac.x, pa.x] = 0;
[u, v, 1] * [ab.y, ac.y, pa.y] = 0;
��uv1��ab,ac,pa��x�ᣬy�ᶼ��ֱ�����Բ��
Ҫ���㡰1 >= u >= 0, 1 >= v >= 0, u+v <= 1������p��������abc��
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
��˷�
t1 = PA^PB,
t2 = PB^PC,
t3 = PC^PA, ����ͬ�����������ڲ�
*/
bool IsPointInTriangle(Vec2i* triangle, Vec2i p);

bool IsPointInTriangle(Vec3f* triangle, Vec3f p);

void DrawTriangle(Vec3f* triangle, float* intensitys, Vec2i* uvs, float* zbuffer, Canvas& canvas, TGAImage& img);