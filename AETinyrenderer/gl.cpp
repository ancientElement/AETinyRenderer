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

Matrix4f viewcamera(Vector3f camera_pos, Vector3f forward, Vector3f up)
{
    Vector3f _up = up.normalized();
    Vector3f _forward = forward.normalized();
    Vector3f _left = _up.cross(_forward).normalized();
    Matrix4f m;
    m <<
        _left[0], _left[1], _left[2], -camera_pos[0],
        _up[0], _up[1], _up[2], -camera_pos[1],
        _forward[0], _forward[1], _forward[2], -camera_pos[2],
        0, 0, 0, 1;
    return m;
}

Matrix4f viewpotr(int width, int height)
{
    Matrix4f m;
    float w = width * .5;
    float h = height * .5;
    m <<
        w, 0, 0, w,
        0, h, 0, h,
        0, 0, 1., 0,
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

Vector3f barycentric(vector<Vector2f> pts, Vector2f p)
{
    Vector2f AB = pts[1] - pts[0];
    Vector2f AC = pts[2] - pts[0];
    Vector2f PA = pts[0] - p;
    Vector3f uvw = Vector3f(AB[0], AC[0], PA[0]).cross(Vector3f(AB[1], AC[1], PA[1]));
    if (std::abs(uvw[2]) > 1e-2) return Vector3f(1 - (uvw[0] + uvw[1]) / uvw[2], uvw[1] / uvw[2], uvw[0] / uvw[2]);
    return Vector3f(-1, 1, 1);
}

void triangle(vector<Vector4f> pts, TGAImage& image, vector<vector<float>>& z_buffer, const TGAColor& color)
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
        for (y = left_down[1]; y <= right_up[1]; y++)
        {
            vector<Vector2f> temp_pts;
            temp_pts.push_back(Vector2f(pts[0][0], pts[0][1]));
            temp_pts.push_back(Vector2f(pts[1][0], pts[1][1]));
            temp_pts.push_back(Vector2f(pts[2][0], pts[2][1]));
            //重心坐标
            /*debug(temp_pts[0]);
            debug(temp_pts[1]);*/
            Vector3f uvw = barycentric(temp_pts, Vector2f(x, y));
            if (uvw[0] < 0 || uvw[1] < 0 || uvw[2] < 0) continue;
            //计算z值
            float cur_z = 0;
            cur_z += pts[0][2] * uvw[0];
            cur_z += pts[1][2] * uvw[1];
            cur_z += pts[2][2] * uvw[2];
            /*cout << cur_z << endl;*/
            //深度测试
            if (cur_z > z_buffer[x][y])
            {
                z_buffer[x][y] = cur_z;
                image.set(x, y, color);
            }
        }
    }
}
