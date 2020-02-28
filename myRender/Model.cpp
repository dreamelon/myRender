#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "model.h"

Model::Model(const char* filename) : verts_(), faces_() {
	std::ifstream in;
	in.open(filename, std::ifstream::in);
	if (in.fail()) return;
	std::string line;
	while (!in.eof()) {
		std::getline(in, line);
		std::istringstream iss(line.c_str());
		char trash;
		//compare "v " 从0开始2个位置，相等返回0，否则返回非0
		if (!line.compare(0, 2, "v ")) {
			//会自动忽略空格
			//提取字符'v'
			iss >> trash;
			Vec3f v;
			//提取float
			for (int i = 0; i < 3; i++) iss >> v[i];
			verts_.push_back(v);
		}
		else if (!line.compare(0, 3, "vt ")) {
			iss >> trash >> trash;
			Vec2f uv;
			for (int i = 0; i < 2; i++) iss >> uv[i];
			uvs_.push_back(uv);
		}
		else if (!line.compare(0, 3, "vn ")) {
			iss >> trash >> trash;
			Vec3f normal;
			for (int i = 0; i < 3; i++) iss >> normal[i];
			normals_.push_back(normal);
		}
		else if (!line.compare(0, 2, "f ")) {
			std::vector<Vec3i> f;
			
			//表示顶点、法线和纹理的索引，暂时没用
			Vec3i index;
			iss >> trash;
			while (iss >> index[0] >> trash >> index[1] >> trash >> index[2]) {
				for (int i = 0; i < 3; i++) { index[i]--; } // in wavefront obj all indices start at 1, not zero
				f.push_back(index);
			}
			faces_.push_back(f);
		}
	}
	std::cerr << "# v# " << verts_.size() << " f# " << faces_.size() << std::endl;
	LoadTexture(filename, "_diffuse.tga", diffuseMap);
}

Vec2i Model::uv(int iface, int nvert) {
	int idx = faces_[iface][nvert][1];
	return Vec2i(uvs_[idx].x * diffuseMap.get_width(), uvs_[idx].y * diffuseMap.get_height());
}

Vec3f Model::norm(int iface, int nvert) {
	int idx = faces_[iface][nvert][2];
	return normals_[idx];
}

int Model::nverts() {
	return (int)verts_.size();
}

int Model::nfaces() {
	return (int)faces_.size();
}

std::vector<Vec3i> Model::face(int idx) {
	return faces_[idx];
}

Vec3f Model::vert(int i) {
	return verts_[i];
}

void Model::LoadTexture(std::string filename, const char* suffix, TGAImage& img) {
	std::string texfile(filename);
	size_t dot = texfile.find_last_of('.');
	if (dot != std::string::npos) {
		texfile = texfile.substr(0, dot) + suffix;
		std::cerr << "texture file " << texfile << " loading " << (img.read_tga_file(texfile.c_str()) ? "ok" : "failed") << std::endl;
		img.flip_vertically();
	}
}