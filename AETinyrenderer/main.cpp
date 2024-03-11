#include <iostream>
#include <Eigen/Dense>
#include "tgaimage.h"
#include "model.h"
#include "gl.h"
using namespace Eigen;
using namespace std;

const TGAColor white(255, 255, 255, 255);
const TGAColor red(255, 0, 0, 255);
const TGAColor greenTGAColor(0, 255, 0, 255);
const TGAColor blue(0, 0, 255, 255);
//模型
Model* model = NULL;
//图片宽高
int height = 500;
int width = 500;
//灯光方向
Vector3f light_dir(0, 0, -10.);
//摄像机
Vector3f camera_up(0, 1., 0);
Vector3f camera_pos(0, 0, 1.);

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

//着色器
class GroundShader : public Shader
{
public:
    virtual Eigen::Vector4f vertex(int iface, int ivertex) override
    {
    }

    virtual bool fragment(Vector3f barycentric, TGAColor& color) override
    {
    }
};

int main(int argc, char** argv)
{
    cout << "main" << endl;
    TGAImage* image = new TGAImage(width, height, TGAImage::RGB);
    // Model* model = new Model("Resources/box.obj");
    model = new Model("Resources/monkey.obj");
    vector<vector<float>> z_buffer(width, vector<float>(height, -numeric_limits<float>::max()));
    //灯光
    light_dir.normalize();
    //视口矩阵
    Matrix4f m_viewport = viewpotr(width, height);
    //透视投影
    Matrix4f m_projection = projection(camera_pos[2]);
    //viewcamer
    Matrix4f m_viewcamera = viewcamera(camera_pos, camera_up);
    //屏幕坐标
    vector<Vector3f> world_pos(3);
    //世界坐标
    vector<Vector4f> screen_croods(3);
    cout << "total face:";
    cout << model->nfaces() << endl;
    //遍历三角形
    for (int i = 0; i < model->nfaces(); i++)
    {
        //得到一个面
        vector<int> face = model->face(i);
        cout << "now is ";
        cout << i;
        cout << "face" << endl;
        //遍历顶点
        for (int j = 0; j < 3; j++)
        {
            world_pos[j] = model->vert(face[j]);
            screen_croods[j] =
                m_viewport *
                m_projection *
                m_viewcamera *
                Vector4f(world_pos[j][0], world_pos[j][1], world_pos[j][2], 1);
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
            triangle(screen_croods, *image, z_buffer, TGAColor(itensity * 255, itensity * 255, itensity * 255, 255));
        }
    }

    image->flip_vertically();
    image->write_tga_file("Resources/output_class4_triangle_model_monkey_head_viewcamera.tga");
    delete model;
    delete image;
    return 0;
}


//现在是09:30 
//现在是09:34 无发生成
//现在是09:38 无发生成
//现在是09:40 是路径错了
//现在是09:45 生成盒子但是有面是黑色 原因是因为灯光在侧面
//现在是09:48 生成人脸但是有黑色三角形
//现在是09:50 project测试盒子
//现在是09:55 对比发现projection后的盒子侧边被拉长
//现在是09:56 project头
//现在是09:59 project头有略微透视效果
//现在是09:59 相机位置错了 坐标系是右手系
//现在是10:10 修正后盒子透视真确
//现在是10:10 african_head 渲染报错
//现在是10:13 可以生成monkey但是有裂纹
//现在是10:20 可以生成african_head在无透视矩阵的情况下
//现在是10:22 生成african_head,添加透视矩阵情况下报错:error: use of undeclared identifier 'screen_croods'
//现在是11:13 学习并且完成动态规划:最小花费爬楼梯
//现在是11:30 吃完饭
//现在是11:40 写完了斐波那契数列的文章
//现在是11:51 还有一个错误Exception 0x80000003 encountered at address 0x7ffe471af2d2
//现在是12:03 盒子总共有12个面
//现在是12:23 上了厕所
//现在是12:24 人脸总共2491个面
//现在是12:35 图像是正确生成但是main函数return 0的时候报错,只有african_head有这个错误
//现在是12:46 既然对生成图形没有影响我就先不管了,先用猴头代替
//现在是12:48 接下来解决裂缝问题
//现在是13:06 裂缝在包围盒子生成时要用四舍五入,成功解决
//现在是13:06 开始viewcamer矩阵
//现在是13:33 透视矩阵正确 但是需改进方向
//现在是16:18 学习了回溯算法写了一道动态规划题目
//现在是16:20 计划让相机永远看向中心,我们只需要输入up轴就可以了
//现在是16:38 viewcamera矩阵没有什么大问题
