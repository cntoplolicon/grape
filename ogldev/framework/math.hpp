#ifndef __MATH__H__
#define __MATH__H__

#include "GL/glus.h"

float clamp(float v, float l, float r)
{
    return glusMathClampf(v, l, r);
}

struct Vector3f
{
    GLfloat x, y, z;
    GLfloat * value_ptr()
    {
        return &x;
    }

    const float * const_value_ptr() const
    {
        return &x;
    }

    Vector3f & operator += (const Vector3f &v)
    {
        glusVector3AddVector3f(value_ptr(), const_value_ptr(), v.const_value_ptr());
        return *this;
    }

    Vector3f operator + (const Vector3f &v) const
    {
        Vector3f r = *this;
        r += v;
        return r;
    }

    Vector3f & operator -= (const Vector3f &v)
    {
        glusVector3SubtractVector3f(value_ptr(), const_value_ptr(), v.const_value_ptr());
        return *this;
    }

    Vector3f operator - (const Vector3f &v) const
    {
        Vector3f r = *this;
        r -= v;
        return r;
    }

    Vector3f & operator *= (float scalar)
    {
        glusVector3MultiplyScalarf(value_ptr(), const_value_ptr(), scalar);
        return *this;
    }

    Vector3f operator * (float scalar) const
    {
        Vector3f r = *this;
        r *= scalar;
        return r;
    }

    Vector3f & operator /= (float scalar)
    {
        return (*this) *= (1.0 / scalar);
    }

    Vector3f operator / (float scalar) const
    {
        Vector3f r = *this;
        r /= scalar;
        return r;;
    }

    float length() const
    {
        return glusVector3Lengthf(const_value_ptr());
    }

    Vector3f normalized() const
    {
        Vector3f v = *this;
        glusVector3Normalizef(v.value_ptr());
        return v;
    }

    float dot(const Vector3f &v)
    {
        return glusVector3Dotf(const_value_ptr(), v.const_value_ptr());
    }

    Vector3f cross(const Vector3f &v)
    {
        Vector3f r;
        glusVector3Crossf(r.value_ptr(), const_value_ptr(), v.const_value_ptr());
        return r;
    }
};

Vector3f operator*(float scalar, const Vector3f &v)
{
    return v * scalar;
}

struct Matrix4x4f
{
    float v[16];
    float * value_ptr()
    {
        return v;
    }

    const float * const_value_ptr() const
    {
        return v;
    }

    Matrix4x4f & operator += (const Matrix4x4f &m)
    {
        glusMatrix4x4Addf(v, v, m.const_value_ptr());
        return *this;
    }

    Matrix4x4f operator + (const Matrix4x4f &m) const
    {
        Matrix4x4f r = *this;
        r += m;
        return r;
    }

    Matrix4x4f & operator -= (const Matrix4x4f &m)
    {
        glusMatrix4x4Subtractf(v, v, m.const_value_ptr());
        return *this;
    }

    Matrix4x4f operator - (const Matrix4x4f &m) const
    {
        Matrix4x4f r = *this;
        r -= m;
        return r;
    }

    Matrix4x4f & operator *= (const Matrix4x4f &m)
    {
        glusMatrix4x4Multiplyf(v, v, m.const_value_ptr());
        return *this;
    }

    Matrix4x4f operator * (const Matrix4x4f &m) const
    {
        Matrix4x4f r = *this;
        r *= m;
        return r;
    }

    Vector3f operator * (const Vector3f &vf) const
    {
        Vector3f r;
        glusMatrix4x4MultiplyVector3f(r.value_ptr(), v, vf.const_value_ptr());
        return r;
    }

    Matrix4x4f inverse() const
    {
        Matrix4x4f r = *this;
        glusMatrix4x4Inversef(r.value_ptr());
        return r;
    }

    Matrix4x4f transpose() const
    {
        Matrix4x4f r = *this;
        glusMatrix4x4Transposef(r.value_ptr());
        return r;
    }

    static Matrix4x4f identity()
    {
        Matrix4x4f r;
        glusMatrix4x4Identityf(r.value_ptr());
        return r;
    }

    Matrix4x4f translate(const Vector3f &vf) const
    {
        Matrix4x4f r = *this;
        glusMatrix4x4Translatef(r.value_ptr(), vf.x, vf.y, vf.z);
        return r;
    }

    Matrix4x4f rotatex(float scalar) const
    {
        Matrix4x4f r = *this;
        glusMatrix4x4RotateRxf(r.value_ptr(), scalar);
        return r;
    }

    Matrix4x4f rotatey(float scalar) const
    {
        Matrix4x4f r = *this;
        glusMatrix4x4RotateRyf(r.value_ptr(), scalar);
        return r;
    }

    Matrix4x4f rotatez(float scalar) const
    {
        Matrix4x4f r = *this;
        glusMatrix4x4RotateRzf(r.value_ptr(), scalar);
        return r;
    }

    Matrix4x4f scale(const Vector3f &vf) const
    {
        Matrix4x4f r = *this;
        glusMatrix4x4Scalef(r.value_ptr(), vf.x, vf.y, vf.z);
        return r;
    }

    static Matrix4x4f lookAt(const Vector3f &eye, const Vector3f &target, const Vector3f &up)
    {
        Matrix4x4f r;
        glusMatrix4x4LookAtf(r.value_ptr(), eye.x, eye.y, eye.z, 
                target.x, target.y, target.z, up.x, up.y, up.z);
        return r;
    }

    static Matrix4x4f perspective(float fov, float aspect, float zNear, float zFar)
    {
        Matrix4x4f r;
        glusMatrix4x4Perspectivef(r.value_ptr(), fov, aspect, zNear, zFar);
        return r;
    }
};

struct Quaternion
{
    Vector3f v;
    float w;

    float * value_ptr()
    {
        return v.value_ptr();
    }

    const float * const_value_ptr() const
    {
        return v.const_value_ptr();
    }

    static Quaternion identity()
    {
        Quaternion r;
        glusQuaternionIdentityf(r.value_ptr());
        return r;
    }
    
    Quaternion & operator *= (const Quaternion &q)
    {
        glusQuaternionMultiplyQuaternionf(value_ptr(), const_value_ptr(), q.const_value_ptr());
        return *this;
    }

    Quaternion operator * (const Quaternion &q) const
    {
        Quaternion r = *this;
        r *= q;
        return r;
    }

    Vector3f operator * (const Vector3f &vf) const
    {
        return getMatrix() * vf;
    }

    Quaternion normalized() const
    {
        Quaternion r = *this;
        glusQuaternionNormalizef(r.value_ptr());
        return r;
    }

    Quaternion inverse() const
    {
        Quaternion r = *this;
        glusQuaternionInversef(r.value_ptr());
        return r;
    }

    static Quaternion rotate(float degree, Vector3f axis)
    {
        Quaternion r;
        glusQuaternionRotatef(r.value_ptr(), degree, axis.x, axis.y, axis.z);
        return r;
    }

    Matrix4x4f getMatrix() const
    {
        Matrix4x4f r;
        glusQuaternionGetMatrix4x4f(r.value_ptr(), const_value_ptr());
        return r;
    }
};

#endif
