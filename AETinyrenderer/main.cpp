#include <iostream>
#include "tgaimage.h"
#include "main.h"

const TGAColor* white = new TGAColor(255, 255, 255, 255);
const TGAColor* red = new TGAColor(255, 0, 0, 255);

int lerp(int a, int b, float t) {
	return a + (b - a) * t;
}

int xy_learp(int x0, int y0, float t, int x) {
	return y0 + t * (x - x0);
}

void line(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color) {
	//4.优化代码 于原文一致
	//bool t_more_than_1 = false;
	////t大于1
	//if (std::abs(y0 - y1) > std::abs(x0 - x1)) {
	//	t_more_than_1 = true;
	//	//交换坐标轴
	//	std::swap(x0, y0);
	//	std::swap(x1, y1);
	//}
	////x0 > x1 交换左右点
	//if (x0 > x1) {
	//	std::swap(x0, x1);
	//	std::swap(y0, y1);
	//}
	//for (int x = x0; x <= x1; x++)
	//{
	//	float t = (float)(y1 - y0) / (x1 - x0);
	//	int  y = xy_learp(x0, y0, t, x);
	//	if (t_more_than_1) {
	//		image.set(y, x, color);
	//	}
	//	else
	//	{
	//		image.set(x, y, color);
	//	}
	//}


	//5.使用误差优化
	//bool t_more_than_1 = false;
	////t大于1
	//if (std::abs(y0 - y1) > std::abs(x0 - x1)) {
	//	t_more_than_1 = true;
	//	//交换坐标轴
	//	std::swap(x0, y0);
	//	std::swap(x1, y1);
	//}
	////x0 > x1 交换左右点
	//if (x0 > x1) {
	//	std::swap(x0, x1);
	//	std::swap(y0, y1);
	//}
	//int dx = x1 - x0;
	//int dy = y1 - y0;
	//float derror = (float)dy / dx;
	//float error = 0;
	//int y = y0;
	//for (int x = x0; x <= x1; x++)
	//{
	//	if (t_more_than_1) {
	//		image.set(y, x, color);
	//	}
	//	else
	//	{
	//		image.set(x, y, color);
	//	}
	//	error += derror;
	//	if (error > 0.5f)//如果累计步进长度到达一格
	//	{
	//		y += y1 > y0 ? 1 : -1;//向上或者向下走一格
	//		error -= 1;
	//	}
	//}

	//5.去除浮点数
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
	TGAImage* image = new TGAImage(100, 100, TGAImage::RGB);

	line(13, 20, 80, 40, *image, *white); //线段A
	line(20, 13, 40, 80, *image, *red); //线段B
	line(80, 40, 13, 20, *image, *red);//线段C

	image->flip_vertically();
	image->write_tga_file("output.tga");

	delete image;
	delete white;
	delete red;
	return 0;
}