#include <iostream>
#include "tgaimage.h"
#include "model.h"

const TGAColor* white = new TGAColor(255, 255, 255, 255);
const TGAColor* red = new TGAColor(255, 0, 0, 255);
const TGAColor* green = new TGAColor(0, 255, 0, 255);
const TGAColor* blue = new TGAColor(0, 0, 255, 255);

int lerp(int a, int b, float t) {
	return a + (b - a) * t;
}

int xy_learp(int x0, int y0, float t, int x) {
	return y0 + t * (x - x0);
}

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

void line(Vec2i v0, Vec2i v1, TGAImage& image, TGAColor color) {
	line(v0.x, v0.y, v1.x, v1.y, image, color);
}

void loadm_model(Model* model, int width, int height, TGAImage* image)
{
	for (int i = 0; i < model->nfaces(); i++)//遍历三角形
	{
		std::vector<int> face = model->face(i);//得到一个面

		for (int j = 0; j < 3; j++)//遍历面的顶点
		{
			Vec3f v0 = model->vert(face[j]);
			Vec3f v1 = model->vert(face[(j + 1) % 3]);//后一个点

			int x0 = (v0.x + 1.0) * width / 2.0;
			int y0 = (v0.y + 1.0) * height / 2.0;

			int x1 = (v1.x + 1.0) * width / 2.0;
			int y1 = (v1.y + 1.0) * height / 2.0;

			line(x0, y0, x1, y1, *image, *white);
		}
	}
}

void triangle(Vec2i v0, Vec2i v1, Vec2i v2, TGAImage& image, TGAColor color) {
	//升序排列顶点
	if (v0.y > v1.y) std::swap(v0, v1);
	if (v0.y > v2.y) std::swap(v0, v2);
	if (v1.y > v2.y) std::swap(v1, v2);
	line(v0, v1, image, *green);
	line(v1, v2, image, *green);
	line(v2, v0, image, *red);
	//分割成为两部分
	float t_02 = (float(v2.x - v0.x) / (v2.y - v0.y));
	int mid_x = xy_learp(v0.y, v0.x, t_02, v1.y);
	Vec2i v_mid = Vec2i(mid_x, v1.y);
	line(v_mid, v1, image, *green);
	//开始扫线 下半部分
	float t_01 = (float(v1.x - v0.x) / (v1.y - v0.y));
	for (int y = v0.y; y <= v1.y; y++)
	{
		int left_x = xy_learp(v0.y, v0.x, t_01, y);
		int right_x = xy_learp(v0.y, v0.x, t_02, y);
		line(left_x, y, right_x, y, image, color);
	}
	//上部分
	float t_21 = (float(v1.x - v2.x) / (v1.y - v2.y));
	for (int y = v1.y; y <= v2.y; y++)
	{
		int left_x = xy_learp(v2.y, v2.x, t_02, y);
		int right_x = xy_learp(v2.y, v2.x, t_21, y);
		line(left_x, y, right_x, y, image, color);
	}
}

int main(int argc, char** argv) {

	TGAImage* image = new TGAImage(200, 200, TGAImage::RGB);

	Vec2i t0[3] = { Vec2i(10, 70),   Vec2i(50, 160),  Vec2i(70, 80) };
	Vec2i t1[3] = { Vec2i(180, 50),  Vec2i(150, 1),   Vec2i(70, 180) };
	Vec2i t2[3] = { Vec2i(180, 150), Vec2i(120, 160), Vec2i(130, 180) };
	triangle(t0[0], t0[1], t0[2], *image, *red);
	triangle(t2[0], t2[1], t2[2], *image, *green);
	triangle(t1[0], t1[1], t1[2], *image, *blue);

	image->flip_vertically();
	image->write_tga_file("output_class2_triangle_3.tga");

	delete image;
	delete white;
	delete red;
	return 0;
}