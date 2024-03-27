#pragma once
#include <Eigen/Dense>
#include <iostream>
#include <vector>
#include "tgaimage.h"
using namespace Eigen;
using namespace std;


struct Shader
{
public:
    //iface: 第几个面
    //ivertex: 第几个顶点
    //return: 屏幕空间坐标
    virtual Eigen::Vector4f vertex(int iface, int ivertex) = 0;
    //barycentric: 重心坐标
    //color: 颜色ref
    //return: 是否抛弃这个对象
    virtual bool fragment(Vector3f barycentric, TGAColor& color) = 0;
};


Matrix4f viewcamera(Vector3f camera_pos, Vector3f up);
Matrix4f viewpotr(int x, int y, int width, int height);
Matrix4f projection(float camera_z);
Vector3f barycentric(vector<Vector3f> pts, Vector3f p);
void triangle(vector<Vector4f> pts, TGAImage& image, Shader* shader, vector<vector<float>>& z_buffer);
