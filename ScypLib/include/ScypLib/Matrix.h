#pragma once
#include "Vec3.h"
#include <cmath>

namespace sl
{
    template<typename T, size_t N>
    class Mat
    {
    public:
        Mat()
        {
            Identity();
        }

        void Identity()
        {
            for (size_t i = 0; i < N; i++)
            {
                for (size_t j = 0; j < N; j++)
                {
                    mat[i][j] = (i == j) ? T(1) : T(0);
                }
            }
        }

        void Translate(const Vec3<T>& vec)
        {
            static_assert(N >= 4, "Translate requires N >= 4");
            mat[N - 1][0] += vec.x;
            mat[N - 1][1] += vec.y;
            mat[N - 1][2] += vec.z;
        }

        void Scale(const Vec3<T>& vec)
        {
            if constexpr (N > 0)
            {
                mat[0][0] *= vec.x;
            }
            if constexpr (N > 1)
            {
                mat[1][1] *= vec.y;
            }
            if constexpr (N > 2)
            {
                mat[2][2] *= vec.z;
            }
        }

        void Rotate(const Vec3<T>& vec)
        {
            static_assert(N >= 3, "Rotate requires N >= 3");

            if (vec.x != T(0))
            {
                T c = std::cos(vec.x);
                T s = std::sin(vec.x);
                T rot[N][N] = { T(0) };
                for (size_t i = 0; i < N; i++)
                {
                    for (size_t j = 0; j < N; j++)
                    {
                        rot[i][j] = (i == j) ? T(1) : T(0);
                    }
                }
                rot[1][1] = c;
                rot[1][2] = -s;
                rot[2][1] = s;
                rot[2][2] = c;
                Multiply(rot);
            }

            if (vec.y != T(0))
            {
                T c = std::cos(vec.y);
                T s = std::sin(vec.y);
                T rot[N][N] = { T(0) };
                for (size_t i = 0; i < N; i++)
                {
                    for (size_t j = 0; j < N; j++)
                    {
                        rot[i][j] = (i == j) ? T(1) : T(0);
                    }
                }
                rot[0][0] = c;
                rot[0][2] = s;
                rot[2][0] = -s;
                rot[2][2] = c;
                Multiply(rot);
            }

            if (vec.z != T(0))
            {
                T c = std::cos(vec.z);
                T s = std::sin(vec.z);
                T rot[N][N] = { T(0) };
                for (size_t i = 0; i < N; i++)
                {
                    for (size_t j = 0; j < N; j++)
                    {
                        rot[i][j] = (i == j) ? T(1) : T(0);
                    }
                }
                rot[0][0] = c;
                rot[0][1] = -s;
                rot[1][0] = s;
                rot[1][1] = c;
                Multiply(rot);
            }
        }

        T* Data()
        {
            return &mat[0][0];
        }

        const T* Data() const
        {
            return &mat[0][0];
        }

    private:
        void Multiply(const T other[N][N])
        {
            T result[N][N] = { T(0) };
            for (size_t i = 0; i < N; i++)
            {
                for (size_t j = 0; j < N; j++)
                {
                    for (size_t k = 0; k < N; k++)
                    {
                        result[i][j] += mat[k][j] * other[i][k];
                    }
                }
            }
            for (size_t i = 0; i < N; i++)
            {
                for (size_t j = 0; j < N; j++)
                {
                    mat[i][j] = result[i][j];
                }
            }
        }

    private:
        T mat[N][N];
    };

    template<typename T, size_t N>
    inline Mat<T, N> Ortho(T left, T right, T bottom, T top, T near, T far)
    {
        static_assert(N >= 4, "Ortho requires N >= 4");

        Mat<T, N> result;
        result.Identity();

        T* m = result.Data();

        m[0] = T(2) / (right - left);
        m[5] = T(2) / (top - bottom);
        m[10] = -T(2) / (far - near);
        m[12] = -(right + left) / (right - left);
        m[13] = -(top + bottom) / (top - bottom);
        m[14] = -(far + near) / (far - near);

        return result;
    }

    using Mat4f = Mat<float, 4>;
    using Mat4d = Mat<double, 4>;
}
