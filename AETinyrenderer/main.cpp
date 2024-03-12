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
Vector3f light_dir(1., 1., 1.);
//摄像机
Vector3f camera_up(0, 1., 0);
Vector3f camera_pos(.3, .2, 3.);
//几个矩阵
//视口矩阵
Matrix4f m_viewport;
//透视投影
Matrix4f m_projection;
//viewcamer
Matrix4f m_viewcamera;

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
    Vector3f ndl; //法线与光方向
    virtual Eigen::Vector4f vertex(int iface, int ivertex) override
    {
        Vector3f v = model->vert(iface, ivertex);
        Vector3f n = model->normal(iface, ivertex).normalized();
        ndl[ivertex] = max(.0f, n.dot(light_dir));
        return m_viewport * m_projection * m_viewcamera * Vector4f(v[0], v[1], v[2], 1.);
    }

    virtual bool fragment(Vector3f barycentric, TGAColor& color) override
    {
        float intensity = ndl.dot(barycentric);
        if (intensity > .85) intensity = 1;
        else if (intensity > .60) intensity = .80;
        else if (intensity > .45) intensity = .60;
        else if (intensity > .30) intensity = .45;
        else if (intensity > .15) intensity = .30;
        else intensity = 0;
        color = TGAColor(255, 155, 0) * intensity;
        return false;
    }
};

class SimpleShader : public Shader
{
public:
    Vector3f ndl; //法线与光方向
    Matrix<float, 2, 3> uv; //uv
    virtual Eigen::Vector4f vertex(int iface, int ivertex) override
    {
        //顶点
        Vector3f v = model->vert(iface, ivertex);
        //法线
        Vector3f n = model->normal(iface, ivertex).normalized();
        ndl[ivertex] = max(.0f, n.dot(light_dir));
        //纹理
        uv(0, ivertex) = model->uv(iface, ivertex)[0];
        uv(1, ivertex) = model->uv(iface, ivertex)[1];
        return m_viewport * m_projection * m_viewcamera * Vector4f(v[0], v[1], v[2], 1.);
    }

    virtual bool fragment(Vector3f barycentric, TGAColor& color) override
    {
        float intensity = ndl.dot(barycentric);
        Vector2f tempUV = uv * barycentric;
        color = model->diffuse(tempUV);
        color = color * intensity;
        return false;
    }
};

int main(int argc, char** argv)
{
    cout << "main" << endl;
    TGAImage* image = new TGAImage(width, height, TGAImage::RGB);
    model = new Model("Resources/african_head.obj");
    // model = new Model("Resources/african_head.obj");
    vector<vector<float>> z_buffer(width, vector<float>(height, -numeric_limits<float>::max()));
    //灯光
    light_dir.normalize();
    //屏幕坐标
    vector<Vector4f> screen_croods(3);
    //世界坐标
    vector<Vector3f> world_pos(3);
    //矩阵
    m_viewport = viewpotr(width, height);
    m_projection = projection(camera_pos[2]);
    m_viewcamera = viewcamera(camera_pos, camera_up);
    //shader
    // GroundShader* shader = new GroundShader();
    SimpleShader* shader = new SimpleShader();

    cout << "total face:";
    cout << model->nfaces() << endl;

    //遍历三角形
    for (int i = 0; i < model->nfaces(); i++)
    {
        //得到一个面 三个点对应的一个面
        // vector<int> face = model->face(i);
        // cout << "now is ";
        // cout << i;
        // cout << "face" << endl;
        //--顶点阶段--
        for (int j = 0; j < 3; j++)
        {
            screen_croods[j] = shader->vertex(i, j);
        }
        //--片元阶段--
        triangle(screen_croods, *image, shader, z_buffer);
    }

    image->flip_vertically();
    image->write_tga_file("Resources/output_class6_triangle_model_african_head_shader_2_diffuse.tga");
    delete shader;
    delete model;
    delete image;
    return 0;
}
