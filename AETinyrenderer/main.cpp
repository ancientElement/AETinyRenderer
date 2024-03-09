#include <iostream>
#include <Eigen\Dense>
#include "tgaimage.h"
#include "model.h"

using namespace Eigen;

const TGAColor* white = new TGAColor(255, 255, 255, 255);
const TGAColor* red = new TGAColor(255, 0, 0, 255);
const TGAColor* green = new TGAColor(0, 255, 0, 255);
const TGAColor* blue = new TGAColor(0, 0, 255, 255);
const TGAColor* yellow = new TGAColor(229, 229, 0, 255);

const int width = 500;
const int height = 500;

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
			image.set(y + width * 0.5f, x + height * 0.5f, color);
		}
		else
		{
			image.set(x + width * 0.5f, y + height * 0.5f, color);
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
void line(Vector2f* vs, int length, TGAImage& image, TGAColor color) {
	for (int i = 0; i < length; i++)
	{
		line(vs[i][0], vs[i][1], vs[(i + 1) % length][0], vs[(i + 1) % length][1], image, color);
	}
}

//缩放
void scale(Vector2f* verticels, int length, float scale) {
	Matrix<float, 2, 2> scale_m;
	scale_m <<
		scale, 0,
		0, scale;
	for (int i = 0; i < length; i++)
	{
		verticels[i] = scale_m * verticels[i];
	}
}

//剪切
void shearing(Vector2f* verticels, int length) {
	Matrix<float, 2, 2> shearing_m;
	shearing_m <<
		1, 1,
		0, 1;
	for (int i = 0; i < length; i++)
	{
		verticels[i] = shearing_m * verticels[i];
	}
}

//旋转
void rotate(Vector2f* verticels, int length, int angle) {
	Matrix<float, 2, 2> rotation;
	rotation <<
		std::cos(angle), -std::sin(angle),
		std::sin(angle), std::cos(angle);
	for (int i = 0; i < length; i++)
	{
		verticels[i] = rotation * verticels[i];
	}
}

//位移
void translate(Vector2f* verticels, int length, int x, int y) {
	Matrix<float, 3, 3> translate;
	translate <<
		1, 0, x,
		0, 1, y,
		0, 0, 1;
	for (int i = 0; i < length; i++)
	{
		Vector3f temp;
		temp << verticels[i][0], verticels[i][1], 1;
		temp = translate * temp;
		verticels[i][0] = temp[0];
		verticels[i][1] = temp[1];
	}
}

int main(int argc, char** argv) {

	TGAImage* image = new TGAImage(width, height, TGAImage::RGB);

	Vector2f square_1[4]{
		Vector2f(-100,-100),
		Vector2f(100,-100),
		Vector2f(100,100),
		Vector2f(-100,100),
	}, square_2[4]{
		Vector2f(-100,-100),
		Vector2f(100,-100),
		Vector2f(100,100),
		Vector2f(-100,100),
	}, square_3[4]{
		Vector2f(-100,-100),
		Vector2f(100,-100),
		Vector2f(100,100),
		Vector2f(-100,100),
	}, square_4[4]{
		Vector2f(-100,-100),
		Vector2f(100,-100),
		Vector2f(100,100),
		Vector2f(-100,100),
	};

	Vector2f x_axis[2]{
		Vector2f(0,0),
		Vector2f(0, width / 2)
	};

	Vector2f y_axis[2]{
		Vector2f(0,0),
		Vector2f(height / 2,0)
	};

	line(y_axis, 2, *image, *green);
	line(x_axis, 2, *image, *red);

	line(square_1, 4, *image, *white);

	//缩放
	scale(square_1, 4, 0.7);
	line(square_1, 4, *image, *yellow);

	//旋转
	scale(square_2, 4, 0.5);
	rotate(square_2, 4, 30);
	line(square_2, 4, *image, *white);

	//剪切
	scale(square_3, 4, 0.3);
	shearing(square_3, 4);
	line(square_3, 4, *image, *yellow);

	//位移变换
	scale(square_4, 4, 0.6);
	translate(square_4, 4, 150, 150);
	line(square_4, 4, *image, *yellow);

	image->flip_vertically();
	image->write_tga_file("output_class4.tga");

	delete image;
	delete white;
	delete red;
	return 0;
}