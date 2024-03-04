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
	//1.只有三个点
	////起点
	//image.set(x0, y0, color);
	////中点
	//image.set((x0 + x1) / 2, (y0 + y1) / 2, color);
	////终点
	//image.set(x1, y1, color);
	//2.使用线性插值
	//用线性插值
	//for (float i = 0; i < 1; i += 0.1f)
	//{
	//	float x = lerp(x0, x1, i);
	//	float y = lerp(y0, y1, i);
	//	image.set(x, y, color);
	//}
	//3.使用函数 由于上面无论线段长度是多长,绘制的点数是固定的所以 多余了性能消耗
	//考虑到坐标大小

	float t = (float)(y1 - y0) / (x1 - x0);
	std::cout << t;
	std::cout << "\n";
	//改用y坐标系
	if (t > 1) {
		int miny = std::min(y0, y1);
		int maxy = std::max(y0, y1);
		for (int y = miny; y <= maxy; y++)
		{
			int x = xy_learp(y0, x0, 1 / t, y);
			image.set(x, y, color);
		}
	}
	else
	{
		int minx = std::min(x0, x1);
		int maxx = std::max(x0, x1);
		for (int x = minx; x <= maxx; x++)
		{
			int y = xy_learp(x0, y0, t, x);
			image.set(x, y, color);
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