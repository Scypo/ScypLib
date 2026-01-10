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
    template<typename T>
    class Mat4
    {
    public:
        Mat4(T val = T(1))
        {
            for (int i = 0; i < 16; i++) mat[i] = T(0);
            mat[0] = mat[5] = mat[10] = mat[15] = val;
        }
        void Identity()
        {
            for (int i = 0; i < 16; i++) mat[i] = T(0);
            mat[0] = mat[5] = mat[10] = mat[15] = T(1);
        }
        void Translate(const Vec3<T>& v)
        {
            Mat4<T> m(T(1));
            m[12] = v.x;
            m[13] = v.y;
            m[14] = v.z;

            *this = m * *this;
        }

        void Scale(const Vec3<T>& v)
        {
            Mat4<T> m(T(1));
            m[0] = v.x;
            m[5] = v.y;
            m[10] = v.z;
            *this = m * *this;
        }

        void Rotate(const Vec3<T>& v)
        {
            T cx = std::cos(v.x), sx = std::sin(v.x);
            T cy = std::cos(v.y), sy = std::sin(v.y);
            T cz = std::cos(v.z), sz = std::sin(v.z);

            Mat4<T> Rz(T(1)), Ry(T(1)), Rx(T(1));
            
            Rz[0] = cz;
            Rz[1] = sz;
            Rz[4] = -sz;
            Rz[5] = cz;

            Ry[0] = cy;
            Ry[2] = -sy;
            Ry[8] = sy;
            Ry[10] = cy;

            Rx[5] = cx;
            Rx[6] = sx;
            Rx[9] = -sx;
            Rx[10] = cx;

            *this = Rz * Ry * Rx * (*this);
        }

        T* Data()
        {
            return mat;
        }

        const T* Data() const
        {
            return mat;
        }
        T& operator[](int index) { return mat[index]; }
        const T& operator[](int index) const { return mat[index]; }
        Mat4<T> operator*(const Mat4<T>& other) const
        {
            Mat4<T> res(T(0));

            for (int col = 0; col < 4; col++)
            {
                for (int row = 0; row < 4; row++)
                {
                    for (int i = 0; i < 4; i++)
                    {
                        res.mat[col * 4 + row] +=
                            mat[i * 4 + row] * other.mat[col * 4 + i];
                    }
                }
            }

            return res;
        }

    private:
        T mat[16];
    };

    template<typename T>
    inline Mat4<T> Ortho(T left, T right, T bottom, T top, T zNear, T zFar)
    {
        Mat4 r(T(1));

        r[0] = T(2) / (right - left);
        r[5] = T(2) / (top - bottom);
        r[10] = T(-2) / (zFar - zNear);

        r[12] = -(right + left) / (right - left);
        r[13] = -(top + bottom) / (top - bottom);
        r[14] = -(zFar + zNear) / (zFar - zNear);

        return r;
    }
    template<typename T>
    inline Mat4<T> LookAt(const Vec3<T>& eye, const Vec3<T>& center, const Vec3<T>& up)
    {
        Vec3<T> f = (center - eye).Normalized();
        Vec3<T> s = f.Cross(up).Normalized();
        Vec3<T> u = s.Cross(f);   

        Mat4 r(T(1));

        r[0] = s.x;
        r[1] = u.x;
        r[2] = -f.x;

        r[4] = s.y;
        r[5] = u.y;
        r[6] = -f.y;

        r[8] = s.z;
        r[9] = u.z;
        r[10] = -f.z;

        r[12] = -s.Dot(eye);
        r[13] = -u.Dot(eye);
        r[14] = f.Dot(eye);

        return r;
    }
    using Mat4f = Mat4<float>;
    using Mat4d = Mat4<double>;
}
