#include <Eigen/Dense>
#include <iostream>
#include <vector>
#include "gl.h"
#include "tgaimage.h"
using namespace Eigen;
using namespace std;

void debug(Vector2f v)
{
    cout << v[0];
    cout << " ";
    cout << v[1];
    cout << endl;
}

Matrix4f viewcamera(Vector3f camera_pos, Vector3f up)
{
    Vector3f _up = up.normalized();
    Vector3f _forward = -(Vector3f::Zero() - camera_pos).normalized();
    Vector3f _right = _up.cross(_forward).normalized();
    Matrix4f m;
    m <<
        _right[0], _right[1], _right[2], -camera_pos[0],
        _up[0], _up[1], _up[2], -camera_pos[1],
        _forward[0], _forward[1], _forward[2], -camera_pos[2],
        0, 0, 0, 1;
    return m;
}

Matrix4f viewpotr(int x, int y, int width, int height)
{
    Matrix4f m;
    float w = width;
    float h = height;
    m <<
        w * .5f, 0, 0, w * .5f + x,
        0, h * .5f, 0, h * .5f + y,
        0, 0, 1, 0,
        0, 0, 0, 1.;
    return m;
}

Matrix4f projection(float camera_z)
{
    Matrix4f m;
    m <<
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, -1. / camera_z, 1;
    return m;
}

Vector3f barycentric(vector<Vector3f> pts, Vector3f p)
{
    Vector3f AB = pts[1] - pts[0];
    Vector3f AC = pts[2] - pts[0];
    Vector3f PA = pts[0] - p;
    Vector3f uvw = Vector3f(AB[0], AC[0], PA[0]).cross(Vector3f(AB[1], AC[1], PA[1]));
    // if (std::abs(uvw[2]) > 1e-2) return Vector3f(1 - (uvw[0] + uvw[1]) / uvw[2], uvw[1] / uvw[2], uvw[0] / uvw[2]);
    if (std::abs(uvw[2]) > 1e-2) return Vector3f(1 - (uvw[0] + uvw[1]) / uvw[2], uvw[0] / uvw[2], uvw[1] / uvw[2]);
    return Vector3f(-1, 1, 1);
}

void triangle(vector<Vector4f> pts, TGAImage& image, Shader* shader, vector<vector<float>>& z_buffer)
{
    //包围盒
    Vector2i left_down(numeric_limits<int>::max(), numeric_limits<int>::max());
    Vector2i right_up(-numeric_limits<int>::max(), -numeric_limits<int>::max());
    for (int i = 0; i < 3; ++i)
    {
        pts[i] = pts[i] / pts[i][3];
        left_down[0] = min(left_down[0], (int)(pts[i][0] + .5));
        left_down[1] = min(left_down[1], (int)(pts[i][1] + .5));

        right_up[0] = max(right_up[0], (int)(pts[i][0] + .5));
        right_up[1] = max(right_up[1], (int)(pts[i][1] + .5));
    }
    //遍历包围盒
    //逐像素
    int x = 0;
    int y = 0;
    for (x = left_down[0]; x <= right_up[0]; x++)
    {
        if (x < 0) continue;
        for (y = left_down[1]; y <= right_up[1]; y++)
        {
            if (y < 0) continue;
            vector<Vector3f> temp_pts;
            temp_pts.push_back(Vector3f(pts[0][0], pts[0][1], 1.f));
            temp_pts.push_back(Vector3f(pts[1][0], pts[1][1], 1.f));
            temp_pts.push_back(Vector3f(pts[2][0], pts[2][1], 1.f));
            //重心坐标
            /*debug(temp_pts[0]);
            debug(temp_pts[1]);*/
            Vector3f uvw = barycentric(temp_pts, Vector3f(x, y, 1.f));
            if (uvw[0] < 0 || uvw[1] < 0 || uvw[2] < 0) continue;
            //计算z值
            float cur_z = pts[0][2] * uvw[0] + pts[1][2] * uvw[1] + pts[2][2] * uvw[2];
            //深度测试
            if (cur_z > z_buffer[x][y])
            {
                TGAColor color;
                bool discard = shader->fragment(uvw, color);
                if (!discard)
                {
                    z_buffer[x][y] = cur_z;
                    image.set(x, y, color);
                }
            }
        }
    }
}
