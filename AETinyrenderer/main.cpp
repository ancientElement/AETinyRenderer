#include <iostream>
#include "tgaimage.h"
#include "model.h"

const TGAColor* white = new TGAColor(255, 255, 255, 255);
const TGAColor* red = new TGAColor(255, 0, 0, 255);
const TGAColor* green = new TGAColor(0, 255, 0, 255);
const TGAColor* blue = new TGAColor(0, 0, 255, 255);

//线性插值
int lerp(int a, int b, float t) {
	return a + (b - a) * t;
}

//求解线性xy
int xy_learp(int x0, int y0, float t, int x) {
	return y0 + t * (x - x0);
}

//画线
void line(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color) {
	bool t_more_than_1 = false;
	//t大于1
	if (std::abs(y0 - y1) > std::abs(x0 - x1)) {
		t_more_than_1 = true;
		//交换坐标轴
		std::swap(x0, y0);
		std::swap(x1, y1);
	}
	//x0 > x1 交换左右点
	if (x0 > x1) {
		std::swap(x0, x1);
		std::swap(y0, y1);
	}
	int dx = x1 - x0;
	int dy = y1 - y0;
	int derror = std::abs(dy) * 2;
	int error = 0;
	int y = y0;
	for (int x = x0; x <= x1; x++)
	{
		if (t_more_than_1) {
			image.set(y, x, color);
		}
		else
		{
			image.set(x, y, color);
		}
		error += derror;
		if (error > dx)//如果累计步进长度到达一格
		{
			y += y1 > y0 ? 1 : -1;//向上或者向下走一格
			error -= dx * 2;//减去一个移动步长
		}
	}
}

//画线
void line(Vec2i v0, Vec2i v1, TGAImage& image, TGAColor color) {
	line(v0.x, v0.y, v1.x, v1.y, image, color);
}

//返回 u v w alpha peta gama
Vec3f barycentric(Vec3f v0, Vec3f v1, Vec3f v2, Vec3f P) {
	Vec3f AB = v2 - v0;
	Vec3f AC = v1 - v0;
	Vec3f PA = v0 - P;
	Vec3f uvw =
		Vec3f(AB.x, AC.x, PA.x) ^
		Vec3f(AB.y, AC.y, PA.y);
	if (std::abs(uvw.z) < 1) return Vec3f(-1, 1, 1);
	return Vec3f(1.f - (uvw.x + uvw.y) / uvw.z, uvw.y / uvw.z, uvw.x / uvw.z);
}

//遍历法 深度缓冲
void triangle_foreach(Vec3f* pts, float* z_buffer, TGAImage& image, TGAColor color) {
	//包围盒
	int width = image.get_width();
	int height = image.get_height();
	Vec2f left_down(width - 1, height - 1);
	Vec2f right_up(0, 0);
	for (int i = 0; i < 3; i++)
	{
		left_down.x = std::min(left_down.x, pts[i].x);
		left_down.y = std::min(left_down.y, pts[i].y);

		right_up.x = std::max(right_up.x, pts[i].x);
		right_up.y = std::max(right_up.y, pts[i].y);
	}
	//遍历包围盒
	Vec3f point;
	for (point.x = left_down.x; point.x <= right_up.x; point.x++)
	{
		for (point.y = left_down.y; point.y <= right_up.y; point.y++)
		{
			Vec3f uvw = barycentric(pts[0], pts[1], pts[2], point);


			if (uvw.x < 0 || uvw.y < 0 || uvw.z < 0) continue;

			point.z = 0;

			//用重心坐标计算当前像素的z值
			point.z += pts[0].z * uvw.x;
			point.z += pts[1].z * uvw.y;
			point.z += pts[2].z * uvw.z;

			int index = int(point.x + point.y * width);
			//这里z轴是负反向 越大越近
			if (z_buffer[index] < point.z) //当前z_buffer的深度值在后面更新设深度值 更新颜色
			{
				//std::cout << point.z << std::endl;
				z_buffer[index] = point.z;
				image.set(point.x, point.y, color);
			}
		}
	}
}

//世界坐标到平面坐标
Vec3f world2screen(Vec3f v, int width, int height) {
	return Vec3f(int((v.x + 1.) * width / 2. + .5), int((v.y + 1.) * height / 2. + .5), v.z);
}

//加载模型面
void load_modele_triangle(Model* model, TGAImage* image) {
	int height = image->get_height();
	int width = image->get_width();

	//灯光方向
	Vec3f light_dir(0, 0, 1);
	float* z_buffer = new float[height * width];
	//初始化z_buffer都为及小的值
	for (int i = width * height; i--; z_buffer[i] = -std::numeric_limits<float>::max());

	for (int i = 0; i < model->nfaces(); i++)//遍历三角形
	{
		std::vector<int> face = model->face(i);//得到一个面
		Vec3f pts[3];
		Vec3f world_pos[3];
		for (int j = 0; j < 3; j++) {
			Vec3f v = model->vert(face[j]);
			pts[j] = world2screen(v, width, height);
			world_pos[j] = v;
		}
		//得到法线方向
		Vec3f normal = (world_pos[0] - world_pos[2]) ^ (world_pos[1] - world_pos[2]);
		normal.normalize();
		//与光的方向点乘
		float intensity = light_dir * normal;
		if (intensity > 0) {//去除背部
			TGAColor color(255 * intensity, 255 * intensity, 255 * intensity, 255 * intensity);
			triangle_foreach(pts, z_buffer, *image, color);
		}
	}
}

int main(int argc, char** argv) {

	TGAImage* image = new TGAImage(500, 500, TGAImage::RGB);

	Model* model = new Model("african_head.obj");
	load_modele_triangle(model, image);

	image->flip_vertically();
	image->write_tga_file("output_class2_triangle_model_7.tga");

	delete model;
	delete image;
	delete white;
	delete red;
	return 0;
}