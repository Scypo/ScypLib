#pragma once
#include "Vec3.h"
#include <cmath>

namespace sl
{
    class Mat4
    {
    public:
        Mat4()
        {
            Identity();
        }

        void Identity()
        {
            for (int i = 0; i < 4; i++)
            {
                for (int j = 0; j < 4; j++)
                {
                    mat[i][j] = (i == j) ? 1.0f : 0.0f;
                }
            }
        }

        void Translate(const Vec3f& vec)
        {
            mat[3][0] += vec.x;
            mat[3][1] += vec.y;
            mat[3][2] += vec.z;
        }

        void Scale(const Vec3f& vec)
        {
            mat[0][0] *= vec.x;
            mat[1][1] *= vec.y;
            mat[2][2] *= vec.z;
        }

        void Rotate(const Vec3f& vec)
        {
            if (vec.x != 0.0f)
            {
                float c = std::cos(vec.x);
                float s = std::sin(vec.x);
                float rot[4][4] = {
                    {1, 0, 0, 0},
                    {0, c,-s, 0},
                    {0, s, c, 0},
                    {0, 0, 0, 1}
                };
                Multiply(rot);
            }

            if (vec.y != 0.0f)
            {
                float c = std::cos(vec.y);
                float s = std::sin(vec.y);
                float rot[4][4] = {
                    { c, 0, s, 0},
                    { 0, 1, 0, 0},
                    {-s, 0, c, 0},
                    { 0, 0, 0, 1}
                };
                Multiply(rot);
            }

            if (vec.z != 0.0f)
            {
                float c = std::cos(vec.z);
                float s = std::sin(vec.z);
                float rot[4][4] = {
                    {c,-s, 0, 0},
                    {s, c, 0, 0},
                    {0, 0, 1, 0},
                    {0, 0, 0, 1}
                };
                Multiply(rot);
            }
        }

        float* Data() { return &mat[0][0]; }
        const float* Data() const { return &mat[0][0]; }

    private:
        void Multiply(const float other[4][4])
        {
            float result[4][4] = { 0 };

            for (int i = 0; i < 4; i++)
            {
                for (int j = 0; j < 4; j++)
                {
                    for (int k = 0; k < 4; k++)
                    {
                        result[i][j] += mat[k][j] * other[i][k];
                    }
                }
            }

            for (int i = 0; i < 4; i++)
            {
                for (int j = 0; j < 4; j++)
                {
                    mat[i][j] = result[i][j];
                }
            }
        }

    private:
        float mat[4][4];
    };

    inline Mat4 Ortho(float left, float right, float bottom, float top, float near, float far)
    {
        Mat4 result;
        result.Identity();

        float* m = result.Data();

        m[0] = 2.0f / (right - left);
        m[5] = 2.0f / (top - bottom);
        m[10] = -2.0f / (far - near);

        m[12] = -(right + left) / (right - left);
        m[13] = -(top + bottom) / (top - bottom);
        m[14] = -(far + near) / (far - near);

        return result;
    }
}
