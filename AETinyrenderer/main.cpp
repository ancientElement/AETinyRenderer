#include <iostream>
#include <Eigen/Dense>
#include "tgaimage.h"
#include "model.h"
#include "Shader.h"
#include "gl.h"
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
Vector3f light_dir(0, 0, -10.);
//摄像机
Vector3f camera_up(0, 1, 0);
Vector3f camera_right(1, 0, 0);
Vector3f camera_pos(0, 0, 10.);

void debug(Vector4f v)
{
    cout << v[0];
    cout << " ";
    cout << v[1];
    cout << " ";
    cout << v[2];
    cout << " ";
    cout << v[3];
    cout << endl;
}

int main(int argc, char** argv)
{
    cout << "main" << endl;
    TGAImage* image = new TGAImage(width, height, TGAImage::RGB);
    // Model* model = new Model("Resources/box.obj");
    Model* model = new Model("Resources/african_head.obj");
    vector<vector<float>> z_buffer(width, vector<float>(height, -numeric_limits<float>::max()));
    //灯光
    light_dir.normalize();
    //视口矩阵
    auto m_viewport = viewpotr(width, height);
    //透视投影
    auto m_projection = projection(camera_pos[2]);
    cout << "total face:";
    cout << model->nfaces() << endl;
    for (int i = 0; i < model->nfaces(); i++) //遍历三角形
    {
        vector<int> face = model->face(i); //得到一个面
        //遍历顶点
        vector<Vector4f> screen_croods(3);
        vector<Vector3f> world_pos(3);
        for (int j = 0; j < 3; j++)
        {
            Vector3f v = model->vert(face[j]);
            world_pos[j] = v;
            cout << j << endl;
            screen_croods[j] = m_viewport * m_projection * Vector4f(v[0], v[1], v[2], 1);
            // screen_croods[j] = m_viewport * Vector4f(v[0], v[1], v[2], 1);
        }
        //法线
        Vector3f normal;
        normal = (world_pos[1] - world_pos[2]).cross(world_pos[0] - world_pos[2]);
        normal.normalize();
        float itensity = normal.dot(light_dir);
        // cout << itensity << endl;
        if (itensity > 0)
        {
            // cout << "生成了三角形" + i << endl;
            triangle(screen_croods, *image, z_buffer, TGAColor(itensity * 255, itensity * 0, itensity * 0, 255));
        }
    }

    image->flip_vertically();
    image->write_tga_file("Resources/output_class4_triangle_model_african_head_project.tga");
    delete model;
    delete image;
    return 0;
}


//现在是9:30 
//现在是9:34 无发生成
//现在是9:38 无发生成
//现在是9:40 是路径错了
//现在是9:45 生成盒子但是有面是黑色 原因是因为灯光在侧面
//现在是9:48 生成人脸但是有黑色三角形
//现在是9:50 project测试盒子
//现在是9:55 对比发现projection后的盒子侧边被拉长
//现在是9:56 project头
//现在是9:59 project头有略微透视效果
//现在是9:59 相机位置错了 坐标系是右手系
//现在是10:10 修正后盒子透视真确
//现在是10:10 african_head 渲染报错
//现在是10:13 可以生成monkey但是有裂纹
//现在是10:20 可以生成african_head在无透视矩阵的情况下
//现在是10:22 生成african_head,添加透视矩阵情况下报错:error: use of undeclared identifier 'screen_croods'
//现在是11:13 学习并且完成动态规划:最小花费爬楼梯
