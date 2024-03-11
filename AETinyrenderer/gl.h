#pragma once
#include <Eigen/Dense>
#include <iostream>
#include <vector>
#include "tgaimage.h"
using namespace Eigen;
using namespace std;

Matrix4f viewcamera(Vector3f camera_pos, Vector3f up);
Matrix4f viewpotr(int width, int height);
Matrix4f projection(float camera_z);
Vector3f barycentric(vector<Vector2i> pts, Vector2i p);
void triangle(vector<Vector4f> pts, TGAImage& image, vector<vector<float>>& z_buffer, const TGAColor& color);

struct Shader
{
public:
    virtual ~Shader();
    virtual Eigen::Vector4f vertex(int iface, int ivertex) = 0;
    virtual bool fragment(Vector3f barycentric, TGAColor& color) = 0;
};
