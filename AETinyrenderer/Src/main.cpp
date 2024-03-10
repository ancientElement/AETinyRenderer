#include <iostream>
#include <Eigen/Dense>
#include "tgaimage.h"
#include "model.h"

const TGAColor* white = new TGAColor(255, 255, 255, 255);
const TGAColor* red = new TGAColor(255, 0, 0, 255);
const TGAColor* green = new TGAColor(0, 255, 0, 255);
const TGAColor* blue = new TGAColor(0, 0, 255, 255);

//宽高
int height = 500;
int width = 500;
//灯光方向
Vector3f light_dir(0, 0, 1);
//摄像机
Vector3f camera_dir(0, 0, -50.);

//返回 u v w alpha peta gama
Vector3f barycentric(Vector3i v0, Vector3i v1, Vector3i v2, Vector3i P)
{
    Vector3i AB = v2 - v0;
    Vector3i AC = v1 - v0;
    Vector3i PA = v0 - P;
    Vector3f uvw =
        Vector3f(AB[0], AC[0], PA[0]).cross(
            Vector3f(AB[1], AC[1], PA[1]));
    if (std::abs(uvw[2]) < 1) return Vector3f(-1, 1, 1);
    return Vector3f(1.f - (uvw[0] + uvw[1]) / uvw[2], uvw[1] / uvw[2], uvw[0] / uvw[2]);
}

//遍历法 深度缓冲
void triangle_foreach(std::vector<Vector3i>& pts, std::vector<Vector3f>& world_pos, std::vector<float> z_buffer,
                      TGAImage& image, TGAColor color)
{
    //包围盒
    int width = image.get_width();
    int height = image.get_height();
    Vector2i left_down(width - 1, height - 1);
    Vector2i right_up(0, 0);
    for (int i = 0; i < 3; i++)
    {
        left_down[0] = std::min((int)left_down[0], (int)pts[i][0]);
        left_down[1] = std::min((int)left_down[1], (int)pts[i][1]);

        right_up[0] = std::max((int)right_up[0], (int)pts[i][0]);
        right_up[1] = std::max((int)right_up[1], (int)pts[i][1]);
    }
    //遍历包围盒
    Vector3i point;
    float z;
    for (point[0] = left_down[0]; point[0] <= right_up[0]; point[0]++)
    {
        for (point[1] = left_down[1]; point[1] <= right_up[1]; point[1]++)
        {
            Vector3f uvw = barycentric(pts[0], pts[1], pts[2], point);


            if (uvw[0] < 0 || uvw[1] < 0 || uvw[2] < 0) continue;

            z = 0;

            //用重心坐标计算当前像素的z值
            z += world_pos[2][2] * uvw[0];
            z += world_pos[1][2] * uvw[1];
            z += world_pos[0][2] * uvw[2];

            int index = int(point[0] + point[1] * width);
            if (index < 0) continue;
            //这里z轴是负反向 越大越近
            if (z_buffer[index] < z) //当前z_buffer的深度值在后面更新设深度值 更新颜色
            {
                //std::cout << point[2] << std::endl;
                z_buffer[index] = z;
                image.set(point[0], point[1], color);
            }
        }
    }
}

//加载模型面
void load_modele_triangle(Model* model, TGAImage* image)
{
    //视口变换
    Matrix4f viewport;
    viewport <<
        width * 0.5, 0, 0, width * 0.5,
        0, height * 0.5, 0, height * 0.5,
        0, 0, 1, 0,
        0, 0, 0, 1;
    //透视投影
    Matrix4f project;
    project <<
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, -1. / camera_dir[2], 1;
    //初始化z_buffer都为及小的值
    std::vector<float> z_buffer(width * height, -std::numeric_limits<float>::max());

    //遍历三角形
    for (int i = 0; i < model->nfaces(); i++)
    {
        //得到一个面
        std::vector<int> face = model->face(i);
        //屏幕坐标
        std::vector<Vector3i> screen_crood(3);
        //世界坐标
        std::vector<Vector3f> world_pos(3);
        for (int j = 0; j < 3; j++)
        {
            Vector3f v = model->vert(face[j]);
            world_pos[j] = v;
            Vector4f temp;
            temp << v[0], v[1], v[2], 1;
            //视口变换
            temp = viewport * temp;
            screen_crood[j][0] = (int)temp[0] / temp[3];
            screen_crood[j][1] = (int)temp[1] / temp[3];
        }
        //得到法线方向
        Vector3f normal = (world_pos[0] - world_pos[2]).cross(world_pos[1] - world_pos[2]);
        normal.normalize();
        //与光的方向点乘
        float intensity = light_dir.dot(normal);
        //去除背部
        if (intensity > 0)
        {
            TGAColor color(255 * intensity, 255 * intensity, 255 * intensity, 255 * intensity);
            triangle_foreach(screen_crood, world_pos, z_buffer, *image, color);
        }
    }
}

int main(int argc, char** argv)
{
    TGAImage* image = new TGAImage(width, height, TGAImage::RGB);

    //OK
    Model* model = new Model("Resources/african_head.obj");
    load_modele_triangle(model, image);

    image->flip_vertically();
    image->write_tga_file("Resources/output_class4_triangle_model_1.tga");

    delete model;
    delete image;
    delete white;
    delete red;
    return 0;
}

//是类型转化的锅 不要在triangle_foreach中用float来遍历像素
