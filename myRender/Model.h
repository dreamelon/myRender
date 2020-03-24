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
	// face 54/31/54 53/34/53 152/133/152  一个面三个顶点，位置/uv/法线的索引
	std::vector<std::vector<Vec3i> > faces_;
	TGAImage diffuseMap;

	void LoadTexture(std::string filename, const char* suffix, TGAImage& image);
public:
	Model(const char* filename);
	~Model(){}
	int nverts();
	int nfaces();
	Vec3f vert(int i);
	std::vector<Vec3i> face(int idx);
	Vec2f uv(int iface, int nvert);
	Vec3f norm(int iface, int nvert);
	TGAImage GetImage() {	return diffuseMap;	}
};