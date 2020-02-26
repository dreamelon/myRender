#pragma once

#include <vector>
#include "geometry.h"
#include <SDL_image.h>
#include "tgaimage.h"
class Model {
private:
	std::vector<Vec3f> verts_;
	std::vector<Vec2f> uvs_;
	std::vector<Vec3f> normals_;
	// face 54/31/54 53/34/53 152/133/152  һ�����������㣬λ��/uv/���ߵ�����
	std::vector<std::vector<Vec3i> > faces_;
	TGAImage diffuseMap;
	void LoadTexture(std::string filename, const char* suffix, TGAImage& image);
public:
	Model(const char* filename);
	~Model();
	int nverts();
	int nfaces();
	Vec3f vert(int i);
	std::vector<Vec3i> face(int idx);
	Vec2i uv(int iface, int nvert) {
		int idx = faces_[iface][nvert][1];
		return Vec2i(uvs_[idx].x * diffuseMap.get_width(), uvs_[idx].y * diffuseMap.get_height());
	}
	TGAColor GetColor(int x, int y) {
		return diffuseMap.get(x, y);
	}

	TGAImage GetImage() {
		return diffuseMap;
	}
};