#pragma once

#include "geometry.h"

typedef struct { Vec2f orbit; Vec2f pan; float dolly; } motion_t;

const float PI = 3.1415926;
const float EPSILON = 1e-5f;


inline float float_clamp(float f, float min, float max) {
	return f < min ? min : (f > max ? max : f);
}

class Camera {
public:
	Vec3f position;
	Vec3f center;
	Vec3f up;

	float aspect;
	float fov;

	float near;
	float far;
	Camera(Vec3f pos = Vec3f(0.f, 0.f, 1.0f), Vec3f Center = Vec3f(0.f,0.f,0.f), Vec3f Up = Vec3f(0.f, 1.f, 0.f),
		float Aspect = 1.f, float fovy = PI / 3, float Near = 0.1f, float Far = 1000.f) :
		position(pos), center(Center), up(Up), aspect(Aspect), fov(fovy), near(Near), far(Far) { }


 //R * T ��ƽ�Ƶ�ԭ���ٽ��л������任
 /*
 eyeΪ���λ��position��һ����+z��
 * u  -dot(u,eye)
 * v  -dot(v,eye)
 * w  -dot(w,eye)
 * 0       1        

 */
	Matrix LookAt() {
		Matrix m = Matrix::identity();
		//Ϊ���������ָ���� z ����
		Vec3f w = (position - center).normalize();
		Vec3f u = cross(up, w).normalize();
		Vec3f v = cross(w, u).normalize();

		Vec3f translation = Vec3f(0,0,0) - position;
		m[0][3] = translation * u;
		m[1][3] = translation * v;
		m[2][3] = translation * w;

		m[0][0] = u.x;
		m[0][1] = u.y;
		m[0][2] = u.z;
		m[1][0] = v.x;
		m[1][1] = v.y;
		m[1][2] = v.z;
		m[2][0] = w.x;
		m[2][1] = w.y;
		m[2][2] = w.z;

		return m;
	}

	//����ϵ�Ƶ�����ѭn,f��Ϊ���룬Ϊ��ֵ
	/*
	*/
	Matrix projection() {
		Matrix m = Matrix::identity();
		float cottheta = 1 / tan(fov / 2);
		m[0][0] = cottheta / aspect;
		m[1][1] = cottheta;
		m[2][2] = -(near + far) / (far - near);
		m[2][3] = 2 * near * far / (near - far);
		m[3][2] = -1;
		m[3][3] = 0;
		return m;
	}
	Vec3f calculate_pan(Vec3f from_camera, motion_t motion) {
		Vec3f forward = from_camera.normalize();
		Vec3f right = cross(up, forward);
		Vec3f up = cross(forward, right);

		float distance = from_camera.norm();
		//ԭ����2����6�Ƚϸ���
		float factor = distance * (float)tan(fov / 2) * 6;
		Vec3f delta_x = right * motion.pan.x * factor;
		Vec3f delta_y = up * motion.pan.y * factor;
		return delta_x + delta_y;
	}

	Vec3f calculate_offset(Vec3f from_target, motion_t motion) {
		float radius = from_target.norm();
		float theta = (float)atan2(from_target.x, from_target.z);  /* azimuth */
		float phi = (float)acos(from_target.y / radius);           /* polar */
		float factor = PI * 2;
		Vec3f offset;

		radius *= (float)pow(0.95, motion.dolly);
		theta -= motion.orbit.x * factor; // xoffset ��λ��
		phi -= motion.orbit.y * factor; // yoffset ����
		//phi (0, 180) 
		phi = float_clamp(phi, EPSILON, PI - EPSILON);
		//������
		offset.x = radius * (float)sin(phi) * (float)sin(theta);
		offset.y = radius * (float)cos(phi);
		offset.z = radius * (float)sin(phi) * (float)cos(theta);

		return offset;
	}

	void UpdateCameraPan(motion_t motion) {
		Vec3f from_camera = center - position;
		Vec3f from_target = position - center;
		Vec3f pan = calculate_pan(from_camera, motion);
		center = center + pan;
		position = center + from_target;
	}

	void UpdateCameraOffset(motion_t motion) {
		Vec3f from_target = position - center;
		Vec3f offset = calculate_offset(from_target, motion);
		position = center + offset;
	}
};