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
Vector3f light_dir(0, 0, 1.);
//摄像机
Vector3f camera_up(0, 1., 0);
Vector3f camera_pos(.3, .4, 1.5);
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
        //光方向与法线方向的点乘
        ndl[ivertex] = max(.0f, n.dot(light_dir));
        //纹理
        uv(0, ivertex) = model->uv(iface, ivertex)[0];
        uv(1, ivertex) = model->uv(iface, ivertex)[1];
        return m_viewport * m_projection * m_viewcamera * Vector4f(v[0], v[1], v[2], 1.);
    }

    virtual bool fragment(Vector3f barycentric, TGAColor& color) override
    {
        float intensity = ndl.dot(barycentric);
        Vector2f temp_uv = uv * barycentric;
        color = model->diffuse(temp_uv);
        color = color * intensity;
        return false;
    }
};

class SimpleShaderNormal : public Shader
{
public:
    Matrix4f MT; //projection矩阵 * viewcamera矩阵的转置
    Matrix4f M; //projection矩阵 * viewcamera矩阵
    Matrix<float, 2, 3> uv; //uv
    SimpleShaderNormal(Matrix4f m, Matrix4f mt) : MT(mt), M(m)
    {
    }

    virtual Eigen::Vector4f vertex(int iface, int ivertex) override
    {
        //顶点
        Vector3f v = model->vert(iface, ivertex);
        //纹理
        uv(0, ivertex) = model->uv(iface, ivertex)[0];
        uv(1, ivertex) = model->uv(iface, ivertex)[1];
        return m_viewport * m_projection * m_viewcamera * Vector4f(v[0], v[1], v[2], 1.);
    }

    virtual bool fragment(Vector3f barycentric, TGAColor& color) override
    {
        //uv
        Vector2f temp_uv = uv * barycentric;
        //法线 
        Vector3f n = model->normal(temp_uv);
        Vector4f n_temp = Vector4f(n[0], n[1], n[2], 0);
        n_temp = MT * n_temp;
        n = Vector3f(n[0], n[1], n[2]);
        //光 转化到屏幕空间中
        Vector4f l_temp = M * Vector4f(light_dir[0], light_dir[1], light_dir[2], 0);
        Vector3f l = Vector3f(l_temp[0], l_temp[1], l_temp[2]);
        //求光与法线夹脚
        float intensity = max(0.f, n.dot(l));
        color = model->diffuse(temp_uv) * intensity;
        return false;
    }
};

class SimpleShaderSpecular : public Shader
{
public:
    Matrix4f MT; //projection矩阵 * viewcamera矩阵的转置
    Matrix4f M; //projection矩阵 * viewcamera矩阵
    Matrix<float, 2, 3> uv; //uv
    SimpleShaderSpecular(Matrix4f m, Matrix4f mt) : MT(mt), M(m)
    {
    }

    virtual Eigen::Vector4f vertex(int iface, int ivertex) override
    {
        //顶点
        Vector3f v = model->vert(iface, ivertex);
        //纹理
        uv(0, ivertex) = model->uv(iface, ivertex)[0];
        uv(1, ivertex) = model->uv(iface, ivertex)[1];
        return m_viewport * m_projection * m_viewcamera * Vector4f(v[0], v[1], v[2], 1.);
    }

    virtual bool fragment(Vector3f barycentric, TGAColor& color) override
    {
        //uv
        Vector2f temp_uv = uv * barycentric;
        //法线 
        Vector3f n = model->normal(temp_uv);
        Vector4f n_temp = Vector4f(n[0], n[1], n[2], 0);
        n_temp = MT * n_temp;
        n = Vector3f(n[0], n[1], n[2]).normalized();
        //光 转化到屏幕空间中
        Vector4f l_temp = M * Vector4f(light_dir[0], light_dir[1], light_dir[2], 0);
        Vector3f l = Vector3f(l_temp[0], l_temp[1], l_temp[2]).normalized();
        //求光与法线夹角 漫反射
        float diffuse = max(0.f, n.dot(l));
        color = model->diffuse(temp_uv) * diffuse;
        //光的反射方向
        Vec3f r = (l + 2.f * n * (n.dot(l))).normalized();
        //高光 z的大小就是他与normal的夹角大小成正比
        float specular = r[2];
        //颜色
        TGAColor c = model->diffuse(temp_uv);
        color = c;
        for (int i = 0; i < 3; i++) color[i] = std::min<float>(5 + c[i] * (diffuse + .6 * specular), 255);
        return false;
    }
};

class TangentNormalShader : public Shader
{
public:
    Matrix4f MT; //projection矩阵 * viewcamera矩阵的转置
    Matrix4f M; //projection矩阵 * viewcamera矩阵
    Matrix<float, 2, 3> uv; //uv
    Matrix3f normal; //法线和光的点积
    TangentNormalShader(Matrix4f m, Matrix4f mt) : MT(mt), M(m)
    {
    }

    virtual Eigen::Vector4f vertex(int iface, int ivertex) override
    {
        //顶点
        Vector3f v = model->vert(iface, ivertex);
        //纹理
        uv(0, ivertex) = model->uv(iface, ivertex)[0];
        uv(1, ivertex) = model->uv(iface, ivertex)[1];
        //点积
        Vector3f temp_normal = model->normal(iface, ivertex);
        Vector4f temp_normal_1 = MT * Vector4f(temp_normal[0], temp_normal[1], temp_normal[2], 0.);
        normal(0, ivertex) = temp_normal_1[0];
        normal(1, ivertex) = temp_normal_1[1];
        normal(2, ivertex) = temp_normal_1[2];
        return m_viewport * m_projection * m_viewcamera * Vector4f(v[0], v[1], v[2], 1.);
    }

    virtual bool fragment(Vector3f barycentric, TGAColor& color) override
    {
        //uv
        Vector2f temp_uv = uv * barycentric;
        //法线 
        Vector3f n = normal * barycentric;
        //求光与法线夹脚
        float diff = std::max(0.f, n.dot(light_dir));
        color = model->diffuse(temp_uv) * diff;
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
    m_viewport = viewpotr(-width*.15f, -width*.15f, width * 1.5f, height * 1.5f);
    m_projection = projection(camera_pos[2]);
    m_viewcamera = viewcamera(camera_pos, camera_up);
    //shader
    // GroundShader* shader = new GroundShader();
    // SimpleShader* shader = new SimpleShader();

    SimpleShaderNormal* shader = new
        SimpleShaderNormal(m_projection * m_viewport, (m_projection * m_viewport).transpose());

    // SimpleShaderSpecular* shader = new
    // SimpleShaderSpecular(m_projection * m_viewport, (m_projection * m_viewport).transpose());
    // TangentNormalShader* shader = new
    //     TangentNormalShader(m_projection * m_viewport, (m_projection * m_viewport).transpose());

    cout << "total face:";
    cout << model->nfaces() << endl;

    //遍历三角形
    for (int i = 0; i < model->nfaces(); i++)
    {
        //得到一个面 三个点对应的一个面
        //--顶点阶段--
        for (int j = 0; j < 3; j++)
        {
            screen_croods[j] = shader->vertex(i, j);
        }
        //--片元阶段--
        triangle(screen_croods, *image, shader, z_buffer);
    }

    image->flip_vertically();
    image->write_tga_file("Resources/output_class6dot5_triangle_model_african_head_shader_5.tga");
    delete shader;
    delete model;
    delete image;
    return 0;
}
