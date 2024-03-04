#include <iostream>
#include "tgaimage.h"
#include "main.h"
#include "model.h"

const TGAColor* white = new TGAColor(255, 255, 255, 255);
const TGAColor* red = new TGAColor(255, 0, 0, 255);

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

int main(int argc, char** argv) {
	TGAImage* image = new TGAImage(500, 500, TGAImage::RGB);

	Model* model = new Model("african_head.obj");

	int height = image->get_height();
	int width = image->get_width();

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

	image->flip_vertically();
	image->write_tga_file("output.tga");

	delete model;
	delete image;
	delete white;
	delete red;
	return 0;
}