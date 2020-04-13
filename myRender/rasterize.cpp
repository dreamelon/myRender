
#include "rasterize.h"
#include "canvas.h"
#include "geometry.h"
#include <algorithm>
#include <cstdlib>
using std::swap;
using std::abs;
using std::min;
using std::max;
using std::equal;


const int LINE_INSIDE = 0; // 0000
const int LINE_LEFT = 1;   // 0001
const int LINE_RIGHT = 2;  // 0010
const int LINE_BOTTOM = 4; // 0100
const int LINE_TOP = 8;    // 1000

//cohen-sutherland算法，位编码标记裁剪区域
int Encode(Vec2f pos, Vec2f min, Vec2f max) {
	int code = LINE_INSIDE;

	if (pos.x < min.x)
		code |= LINE_LEFT;
	else if (pos.x > max.x) 
		code |= LINE_RIGHT;
	if (pos.y < min.y) 
		code |= LINE_BOTTOM;
	else if (pos.y > max.y)
		code |= LINE_TOP;

	return code;
}

bool CohenSutherlandLineClip(Vec2f start, Vec2f end, Vec2f min, Vec2f max) {
	int start_code = Encode(start, min, max);
	int end_code = Encode(end, min, max);

	bool accept = true;
	while (true) {
		if (!(start_code | end_code))
			break;
		else if (start_code & end_code) {
			accept = false;
			break;
		}
		else {
			float x, y;
			int outCode = start_code ? start_code : end_code;
			if (outCode & LINE_LEFT) {
				y = (end.y - start.y) / (end.x - start.x) * (min.x - start.x) + start.y;
				x = min.x;
			}
			else if (outCode & LINE_BOTTOM) {
				y = min.y;
				x = (end.x - start.x) / (end.y - start.y) * (min.y - start.y) + start.x;
			}
			else if (outCode & LINE_RIGHT) {
				x = max.x;
				y = (end.y - start.y) / (end.x - start.x) * (max.x - start.x) + start.y;
			}
			else if (outCode & LINE_TOP) {
				y = max.y;
				x = (end.x - start.x) / (end.y - start.y) * (max.y - start.y) + start.x;
			}

			if (outCode == start_code) {
				start = Vec2f(x, y);
				start_code = Encode(start, min, max);
			}
			else {
				end = Vec2f(x, y);
				end_code = Encode(end, min, max);
			}
		}
	}

	return accept;
}

void DrawLine(int x0, int y0, int x1, int y1, Canvas& canvas, const Color& color) {
	//for (float t = 0.; t < 1.; t += .01) {
	//	int x = x0 + (x1 - x0) * t;
	//	int y = y0 + (y1 - y0) * t;
	//	canvas.SetPixel(color, x, y);
	//}
	//dx > dy?
	Vec2f start(x0, y0);
	Vec2f end(x1, y1);
	if (!CohenSutherlandLineClip(start, end, Vec2f(0.f, 0.f), Vec2f(canvas.width, canvas.height))){
		return;
	}
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

//扫描线法，分成上下两个三角形
void DrawTriangle(Vec3f t0, Vec3f t1, Vec3f t2, Vec2i* uv, float* zbuffer, Canvas& canvas, TGAImage& img) {

	if (abs(t0.y - t1.y) < 0.001f && abs(t1.y - t2.y) < 0.001f) return;
	//t.y排序
	if (t0.y > t1.y) {
		swap(t0, t1);
		swap(uv[0], uv[1]);
	}
	if (t1.y > t2.y) {
		swap(t1, t2);
		swap(uv[1], uv[2]);
		if (t0.y > t1.y) {
			swap(t0, t1);
			swap(uv[0], uv[1]);
		}
	}

	//下三角形
	float t0t2_y = t2.y - t0.y;
	float t0t1_y = t1.y - t0.y; //避免除以0
	for (float y = t0.y; y <= t1.y; y++) {
		float t0t2_t = (y - t0.y) / (float)t0t2_y;
		float t0t1_t = (y - t0.y) / (float)t0t1_y;

		float t0t2Bound_x = t0.x + (t2.x - t0.x) * t0t2_t;
		float t0t1Bound_x = t0.x + (t1.x - t0.x) * t0t1_t;

		float t0t2Bound_z = t2.z + (t0.z - t2.z) * t0t2_t;
		float t0t1Bound_z = t2.z + (t1.z - t2.z) * t0t1_t;

		Vec2i t0t2_uv = uv[0] + (uv[2] - uv[0]) * t0t2_t;
		Vec2i t0t1_uv = uv[0] + (uv[1] - uv[0]) * t0t1_t;

		//float z = (1 - t0t1_t - t0t2_t) * t0.z + t0t1_t * t1.z + t0t2_t * t2.z;
		if (t0t2Bound_x > t0t1Bound_x) {
			swap(t0t2Bound_x, t0t1Bound_x);
			swap(t0t2Bound_z, t0t1Bound_z);
			swap(t0t2_uv, t0t1_uv);
		}
		for (float x = t0t2Bound_x; x <= t0t1Bound_x; x++) {
			int idx = x + y * canvas.width;
			float delta = (x - t0t2Bound_x) / (t0t1Bound_x - t0t2Bound_x);
			float z = t0t2Bound_z + delta * (t0t1Bound_z - t0t2Bound_z);
			Vec2i uv = t0t2_uv + (t0t1_uv - t0t2_uv) * delta;

			if (zbuffer[idx] < z) {
				canvas.SetPixel(img.get(uv.x, uv.y), x, y);
				zbuffer[idx] = z;
			}

		}
		//DrawLine(t0t2Bound_x, y, t0t1Bound_x, y, canvas, color);
	}
	//上三角形
	float t1t2_y = t2.y - t1.y;
	for (float y = t2.y; y >= t1.y; y--) {
		float t0t2_t = (t2.y - y) / (float)t0t2_y;
		float t0t1_t = (t2.y - y) / (float)t1t2_y;
		float t0t2Bound_x = t2.x + (t0.x - t2.x) * t0t2_t;
		float t0t1Bound_x = t2.x + (t1.x - t2.x) * t0t1_t;
		float t0t2Bound_z = t2.z + (t0.z - t2.z) * t0t2_t;
		float t0t1Bound_z = t2.z + (t1.z - t2.z) * t0t1_t;

		Vec2i t0t2_uv = uv[2] + (uv[0] - uv[2]) * t0t2_t;
		Vec2i t0t1_uv = uv[2] + (uv[1] - uv[2]) * t0t1_t;

		//DrawLine(t0t2Bound_x, y, t0t1Bound_x, y, canvas, color);
		if (t0t2Bound_x > t0t1Bound_x) {
			swap(t0t2Bound_x, t0t1Bound_x);
			swap(t0t2Bound_z, t0t1Bound_z);
			swap(t0t2_uv, t0t1_uv);

		}
		for (float x = t0t2Bound_x; x <= t0t1Bound_x; x++) {
			int idx = x + y * canvas.width;
			float delta = (x - t0t2Bound_x) / (t0t1Bound_x - t0t2Bound_x);
			float z = t0t2Bound_z + delta * (t0t1Bound_z - t0t2Bound_z);
			Vec2i uv = t0t2_uv + (t0t1_uv - t0t2_uv) * delta;
			if (zbuffer[idx] < z) {
				canvas.SetPixel(img.get(uv.x, uv.y), x, y);
				zbuffer[idx] = z;
			}
		}
	}
}

/*
[u, v, 1] * [ab.x, ac.x, pa.x] = 0;
[u, v, 1] * [ab.y, ac.y, pa.y] = 0;
即uv1与ab,ac,pa的x轴，y轴都垂直。所以叉乘
要满足“1 >= u >= 0, 1 >= v >= 0, u+v <= 1”，则p在三角形abc中
*/
Vec3f BaryCentric(Vec3f* triangle, Vec2i p) {
	Vec3f u = cross(Vec3f(triangle[2].x - triangle[0].x, triangle[1].x - triangle[0].x, triangle[0].x - p.x),
		Vec3f(triangle[2].y - triangle[0].y, triangle[1].y - triangle[0].y, triangle[0].y - p.y));
	//因为triangle都是整数，abs(u.z)<1意味着u.z=0,即不构成三角形
	if (abs(u.z) <= 0.01f) return Vec3f(-1, 1, 1);
	return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
}

/*
叉乘法
t1 = PA^PB,
t2 = PB^PC,
t3 = PC^PA, 三者同号在三角形内部
*/
bool IsPointInTriangle(Vec3f* triangle, Vec3i p) {
	Vec2f pa = Vec2f(triangle[0].x, triangle[0].y) - Vec2f(p.x, p.y);
	Vec2f pb = Vec2f(triangle[1].x, triangle[1].y) - Vec2f(p.x, p.y);
	Vec2f pc = Vec2f(triangle[2].x, triangle[2].y) - Vec2f(p.x, p.y);

	float t1 = cross(pa, pb);
	float t2 = cross(pb, pc);
	float t3 = cross(pc, pa);

	return t1 * t2 >= 0.f && t2 * t3 >= 0.f;
}

void Rasterize(V2F* vertexes, float* zbuffer, Shader& shader, Canvas& canvas, TGAImage& img) {
	//三角形法 判断点是否在三角形内
	//首先找到包围三角形的矩形
	//Vec2f bboxmin(canvas.width - 1, canvas.height - 1);
	//Vec2f bboxmax(0, 0);
	Vec3f triangle[3];
	Vec2f uvs[3];
	float rhw[3];
	for (int i = 0; i < 3; i++) {
		triangle[i] = vertexes[i].position;
		uvs[i] = vertexes[i].uv;
		rhw[i] = vertexes[i].position.w;
	}

	//用包围盒做屏幕空间裁剪
	Vec2f bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
	Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
	Vec2f clamp = Vec2f(canvas.width - 1, canvas.height - 1);

	for (int i = 0; i < 3; i++) {
		bboxmax.x = min(clamp.x, max(bboxmax.x, triangle[i].x));
		bboxmax.y = min(clamp.y, max(bboxmax.y, triangle[i].y));
		bboxmin.x = max(0.f, min(bboxmin.x, triangle[i].x));
		bboxmin.y = max(0.f, min(bboxmin.y, triangle[i].y));
	}


	int x, y;
	float z;
	for (x = bboxmin.x; x <= bboxmax.x; x++) {
		for (y = bboxmin.y; y <= bboxmax.y; y++) {
			//if (IsPointInTriangle(triangle, p)) {
			//	canvas.SetPixel(color, p.x, p.y);
			//}

			Vec3f bc_screen = BaryCentric(triangle, Vec2i(x, y));

			if (bc_screen.x < -EPSILON || bc_screen.y < -EPSILON || bc_screen.z < -EPSILON) continue;
			z = 0.f;
			//z' = 2nf / (z * (f-n)) + (n+f) / (f-n)  
			//z是相机空间坐标，z'与1/z成正比, xy也要做透视除法，与1/z成正比
			
			for (int i = 0; i < 3; i++) {
				z += bc_screen[i] * triangle[i].z;
			}

			if (zbuffer[int(x + y * canvas.width)] > z) {
				zbuffer[int(x + y * canvas.width)] = z;

				Vec2f uv;
				//Vec3f normal;
				float  interpolate_rhw = 0.f;
				for (int i = 0; i < 3; i++) {		
					//透视校正插值
					interpolate_rhw = interpolate_rhw + rhw[i] * bc_screen[i];
					uv = uv + uvs[i] * rhw[i] * bc_screen[i];
					//uv = uv + uvs[i] * bc_screen[i];
				}
				uv = uv / interpolate_rhw;

				Color color = img.get(uv.x, uv.y);
				canvas.SetPixel(color, x, y);
			}
		}
	}
}

V2F TextureShader::vert(A2V v) {
	V2F v2f;
	v2f.position = projection * view * model * Vec4f(v.position, 1.f);
	v2f.uv = v.uv;
	v2f.normal = Vec3f(model.invert_transpose() * Vec4f(v.normal, 1.f));
	return v2f;
}

Color TextureShader::frag(V2F o) {
	return Color();
}