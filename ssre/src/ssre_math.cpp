#include <stdexcept>
#include <cstring>
#include <cmath>
#include "ssre.h"

namespace ssre
{
    Matrix& Matrix::operator+=(const Matrix &m)
    {
        for (int i = 0; i < SSRE_MATRIX_DIMENSION; ++i)
            for (int j = 0; j < SSRE_MATRIX_DIMENSION; ++j)
                v[i][j] = v[i][j] + m.v[i][j];
        return *this;
    }

    Matrix Matrix::operator+ (const Matrix &m) const
    {
        Matrix matrix = *this;
        matrix += m;
        return matrix;
    }

    Matrix& Matrix::operator-= (const Matrix &m)
    {
        for (int i = 0; i < SSRE_MATRIX_DIMENSION; ++i)
            for (int j = 0; j < SSRE_MATRIX_DIMENSION; ++j)
                v[i][j] = v[i][j] - m.v[i][j];
        return *this;
    }

    Matrix Matrix::operator- (const Matrix &m) const
    {
        Matrix matrix = *this;
        matrix -= m;
        return matrix;
    }

    Matrix Matrix::operator-() const
    {
        Matrix matrix;
        for (int i = 0; i < SSRE_MATRIX_DIMENSION; ++i)
            for (int j = 0; j < SSRE_MATRIX_DIMENSION; ++j)
                matrix.v[i][j] = -v[i][j];
        return matrix;
    }

    Matrix& Matrix::operator*= (const Matrix &m)
    {
        return *this = *this * m;
    }

    Matrix Matrix::operator* (const Matrix &m) const
    {
        Matrix matrix;
        memset(matrix.v, 0, sizeof(matrix.v));
        for (int i = 0; i < SSRE_MATRIX_DIMENSION; ++i)
            for (int j = 0; j < SSRE_MATRIX_DIMENSION; ++j)
                for (int k = 0; k < SSRE_MATRIX_DIMENSION; ++k)
                    matrix.v[i][j] += v[i][k] * m.v[k][j];
        return matrix;
    }

    Matrix& Matrix::operator/= (float f)
    {
        for (int i = 0; i < SSRE_MATRIX_DIMENSION; ++i)
            for (int j = 0; j < SSRE_MATRIX_DIMENSION; ++j)
                v[i][j] /= f;
        return *this;
    }

    Matrix Matrix::operator/ (float f) const
    {
        Matrix matrix = *this;
        matrix /= f;
        return matrix;
    }

    Matrix Matrix::transpose() const
    {
        Matrix matrix;
        for (int i = 0; i < SSRE_MATRIX_DIMENSION; i++)
            for (int j = 0; j < SSRE_MATRIX_DIMENSION; j++)
                matrix.v[i][j] = v[j][i];
        return matrix;
    }

    inline float determinant3x3(float *v)
    {
        return v[0] * v[4] * v[8] + v[1] * v[5] * v[6] + v[2] * v[3] * v[7] -
               v[2] * v[4] * v[6] - v[1] * v[3] * v[8] - v[0] * v[5] * v[7];
    }

    Matrix Matrix::inverse() const
    {
        float reduced[9]; 
        Matrix cofactor;
        for (int i = 0; i < SSRE_MATRIX_DIMENSION; i++)
            for (int j = 0; j < SSRE_MATRIX_DIMENSION; j++)
            {
                int count = 0;
                for (int k = 0; k < SSRE_MATRIX_DIMENSION; k++)
                    for (int l = 0; l < SSRE_MATRIX_DIMENSION; l++)
                        if (k != i && l != j)
                            reduced[count++] = v[k][l];
                float d = determinant3x3(reduced);
                d = (i + j) % 2 == 0 ? d : -d;
                cofactor.v[j][i] = d;
            }
        float d = 0.0f;
        for (int j = 0; j < SSRE_MATRIX_DIMENSION; j++)
            d += v[0][j] * cofactor.v[j][0];
		if (d == 0.0f)
			throw std::invalid_argument("cannot inverse singular matrix");
        return cofactor / d;
    }

    Vector::Vector() {}
    Vector::Vector(float x, float y, float z) : v{x, y, z, 0.0f} {}
    Vector::Vector(float x, float y, float z, float h) : v{x, y, z, h}{}

    Vector operator*(const Matrix &matrix, const Vector &v)
    {
        float output[SSRE_MATRIX_DIMENSION] = {0.0f, 0.0f, 0.0f, 0.0f};
        for (int i = 0; i < SSRE_MATRIX_DIMENSION; ++i)
            for (int j = 0; j < SSRE_MATRIX_DIMENSION; ++j)
                output[i] += matrix.v[i][j] * v.v[j];
        return Vector {output[0], output[1], output[2], output[3]};
    }

    Vector& Vector::operator+= (const Vector &vector)
    {
        for (int i = 0; i < SSRE_VECTOR_DIMENSION; ++i)
            v[i] = v[i] + vector.v[i];
        return *this;
    }

    Vector Vector::operator+ (const Vector &vector) const
    {
        Vector r = *this;
        r += vector;
        return r;
    }

    Vector& Vector::operator-= (const Vector &vector)
    {
        for (int i = 0; i < SSRE_VECTOR_DIMENSION; ++i)
            v[i] = v[i] - vector.v[i];
        return *this;
    }

    Vector Vector::operator- (const Vector &vector) const
    {
        Vector r = *this;
        r -= vector;
        return r;
    }

    Vector Vector::operator-() const
    {
        Vector r;
        for (int i = 0; i < SSRE_VECTOR_DIMENSION; ++i)
            r.v[i] = -v[i];
        return r;
    }

    Vector& Vector::operator*= (const Vector &vector)
    {
        return *this = *this * vector;
    }

    Vector Vector::operator* (const Vector &vector) const
    {
        const float *_u = v;
        const float *_v = vector.v;
        return Vector {
            _u[1] * _v[2] - _u[2] * _v[1],
            _u[2] * _v[0] - _u[0] * _v[2],
            _u[0] * _v[1] - _u[1] * _v[0]
        };
    }

    Vector Vector::operator*(float f)const
    {
        Vector r;
        for (int i = 0; i < SSRE_VECTOR_DIMENSION; ++i)
            r.v[i] = v[i] * f;
        return r;
    }

    Vector Vector::operator/(float f) const
    {
        Vector r;
        for (int i = 0; i < SSRE_VECTOR_DIMENSION; ++i)
            r.v[i] = v[i] / f;
        return r;
    }

    float Vector::length() const
    {
        return sqrt(dot_product(*this));
    }

    float Vector::dot_product(const Vector &vector) const
    {
        float r = 0.0f;
        for (int i = 0; i < SSRE_VECTOR_DIMENSION; ++i)
            r += v[i] * vector.v[i];
        return r;
    }

    Vector Vector::normalize() const
    {
        return *this / length();
    }

    Vector Vector::divideH() const
    {
        return {x() / h(), y() / h(), z() / h(), 1.0f};
    }

    Vector Vector::discardH() const
    {
        return {x(), y(), z(), 0.0f};
    }

    bool Vector::is_zero_vector() const
    {
        return is_zero(x()) && is_zero(y()) && is_zero(z());
    }

    bool is_zero(float v)
    {
        return abs(v) < 1e-8f;
    }
}
