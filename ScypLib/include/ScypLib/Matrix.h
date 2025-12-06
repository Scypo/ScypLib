#pragma once
#include "Vec3.h"
#include <cmath>

namespace sl
{
    inline float ToRadians(float angle)
    {
        constexpr float pi = 3.14159265358979323846f;
        float radians = angle * pi / 180.0f;
        return radians;
    }
    template<typename T, size_t N, size_t M>
    class Mat
    {
    public:
        Mat(T val = T(1.0f))
        {
            for (size_t i = 0; i < N; i++)
            {
                for (size_t j = 0; j < M; j++)
                {
                    mat[i][j] = (i == j) ? val : T(0);
                }
            }
        }

        void Translate(const Vec3<T>& v) requires (N == 4 && M == 4)
        {
            Mat<T, 4, 4> t(1);
            t.mat[3][0] = v.x;
            t.mat[3][1] = v.y;
            t.mat[3][2] = v.z;

            *this = t * *this;
        }

        void Scale(const Vec3<T>& v) requires (N == 4 && M == 4)
        {
            mat[0][0] *= v.x;
            mat[1][1] *= v.y;
            mat[2][2] *= v.z;
        }

        void Rotate(const Vec3<T>& v) requires (N == 4 && M == 4)
        {
            T cx = std::cos(v.x), sx = std::sin(v.x);
            T cy = std::cos(v.y), sy = std::sin(v.y);
            T cz = std::cos(v.z), sz = std::sin(v.z);

            Mat<T, 4, 4> Rx(1);
            Rx.mat[1][1] = cx;  Rx.mat[1][2] = -sx;
            Rx.mat[2][1] = sx;  Rx.mat[2][2] = cx;

            Mat<T, 4, 4> Ry(1);
            Ry.mat[0][0] = cy;  Ry.mat[0][2] = sy;
            Ry.mat[2][0] = -sy; Ry.mat[2][2] = cy;

            Mat<T, 4, 4> Rz(1);
            Rz.mat[0][0] = cz;  Rz.mat[0][1] = -sz;
            Rz.mat[1][0] = sz;  Rz.mat[1][1] = cz;

            *this = Rz * Ry * Rx * (*this);
        }

        T* Data()
        {
            return &mat[0][0];
        }

        const T* Data() const
        {
            return &mat[0][0];
        }
        Mat<T, N, M> operator*(const Mat<T, N, M>& other) const
        {
            Mat<T, N, M> result(T(0));

            for (size_t i = 0; i < N; i++)
            {
                for (size_t j = 0; j < M; j++)
                {
                    for (size_t k = 0; k < N; k++)
                    {
                        result.mat[i][j] += mat[i][k] * other.mat[k][j];
                    }
                }
            }

            return result;
        }

    private:
        T mat[N][M];
    };

    template<typename T>
    inline Mat<T, 4, 4> Ortho(T left, T right, T bottom, T top, T near, T far)
    {
        Mat<T, 4, 4> result(1);

        T* m = result.Data();
        m[0] = T(2) / (right - left);
        m[5] = T(2) / (top - bottom);
        m[10] = -T(2) / (far - near);

        m[12] = -(right + left) / (right - left);
        m[13] = -(top + bottom) / (top - bottom);
        m[14] = -(far + near) / (far - near);

        return result;
    }

    using Mat4f = Mat<float, 4, 4>;
    using Mat4d = Mat<double, 4, 4>;
}
