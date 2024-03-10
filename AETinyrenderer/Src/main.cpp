#include <iostream>
#include <Eigen/Dense>
#include "tgaimage.h"
#include "model.h"
#include "GL.h"
#include "Shader.h" 
using namespace Eigen;
using namespace std;

const TGAColor white(255, 255, 255, 255);
const TGAColor red(255, 0, 0, 255);
const TGAColor greenTGAColor(0, 255, 0, 255);
const TGAColor blue(0, 0, 255, 255);

//图片宽高
int height = 500;
int width = 500;
//灯光方向
Vector3f light_dir(0, 0, 1);
//摄像机
Vector3f camera_dir(0, 0, -50.);

int main(int argc, char** argv)
{
    TGAImage* image = new TGAImage(width, height, TGAImage::RGB);
    Model* model = new Model("Resources/african_head.obj");


    image->flip_vertically();
    image->write_tga_file("Resources/output_class4_triangle_model_1.tga");
    delete image;
    delete model;
    return 0;
}
