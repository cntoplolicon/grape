#include <cmath>
#include "ssre.h"
#include "internal/ssre_internal.h"

namespace ssre 
{
    namespace internal
    {
        Matrix matrix_model_view;
        Matrix model_view_inverse_transpose;
        Matrix matrix_projection;
        Matrix matrix_view_port;
    }

    void multiply_matrix_model_view(const Matrix &m)
    {
        internal::matrix_model_view *= m;
        internal::model_view_inverse_transpose = 
            internal::matrix_model_view.inverse().transpose();
    }

    void multiply_projection_matrix(const Matrix &m)
    {
        internal::matrix_projection *= m;
    }

    void translate(float tx, float ty, float tz)
    {
        multiply_matrix_model_view(Matrix {{
            {1.0f, 0.0f, 0.0f, tx},
            {0.0f, 1.0f, 0.0f, ty},
            {0.0f, 0.0f, 1.0f, tz},
            {0.0f, 0.0f, 0.0f, 1.0f}
        }});
    }

    void rotate(float theta, float vx, float vy, float vz)
    {
        theta = theta * M_PI / 180.0f;
        Vector v = {vx, vy, vz};
        v.normalize();
        float ux = v.x(), uy = v.y(), uz = v.z();
        float cosA = cos(theta);
        float oneC = 1 - cosA;
        float sinA = sin(theta);

        Matrix factor;
        load_identity_matrix(factor);
        factor.v[0][0] = ux * ux * oneC + cosA;
        factor.v[0][1] = ux * uy * oneC - uz * sinA;
        factor.v[0][2] = ux * uz * oneC + uy * sinA;
        factor.v[1][0] = uy * ux * oneC + uz * sinA;
        factor.v[1][1] = uy * uy * oneC + cosA;
        factor.v[1][2] = uy * uz * oneC - ux * sinA;
        factor.v[2][0] = uz * ux * oneC - uy * sinA;
        factor.v[2][1] = uz * uy * oneC + ux * sinA;
        factor.v[2][2] = uz * uz * oneC + cosA;

        multiply_matrix_model_view(factor);
    }

    void scale(float sx, float sy, float sz)
    {
        multiply_matrix_model_view(Matrix {{
            {sx, 0.0f, 0.0f, 0.0f},
            {0.0f, sy, 0.0f, 0.0f},
            {0.0f, 0.0f, sz, 0.0f},
            {0.0f, 0.0f, 0.0f, 1.0f},
        }});
    }

    void load_identity_matrix(Matrix &matrix) 
    {
        for (int i = 0; i < SSRE_MATRIX_DIMENSION; ++i)
            for (int j = 0; j < SSRE_MATRIX_DIMENSION; ++j)
                matrix.v[i][j] = i == j ? 1 : 0;
    }

    void load_identity_model_view()
    {
        load_identity_matrix(internal::matrix_model_view);
        load_identity_matrix(internal::model_view_inverse_transpose);
    }

    void load_identity_projection()
    {
        load_identity_matrix(internal::matrix_projection);
    }

   void init_3d_viewing()
    {
        load_identity_model_view();
        load_identity_projection();
        view_port(0, 0, internal::window_width, internal::window_height);
    }

    void view_look_at(
            float x0, float y0, float z0, 
            float xref, float yref, float zref,
            float vx, float vy, float vz)
    {
        Vector f = Vector {xref - x0, yref - y0, zref - z0}.normalize();
        Vector up = {vx, vy, vz};
        Vector s = (f * up).normalize();
        Vector u = s * f;

        Matrix m = {{
            {s.x(), s.y(), s.z(), 0.0f},
            {u.x(), u.y(), u.z(), 0.0f},
            {-f.x(), -f.y(), -f.z(), 0.0f},
            {0.0f, 0.0f, 0.0f, 1.0f}
        }};

        multiply_matrix_model_view(m);
        translate(-x0, -y0, -z0);
    }

    void project_ortho(
            float xmin, float xmax, 
            float ymin, float ymax,
            float dnear, float dfar)
    {
        float zn = -dnear, zf = -dfar;
        multiply_projection_matrix(Matrix {{
            {2.0f / (xmax - xmin), 0.0f, 0.0f, -(xmax + xmin) / (xmax - xmin)},
            {0.0f, 2.0f / (ymax - ymin), 0.0f, -(ymax + ymin) / (ymax - ymin)},
            {0.0f, 0.0f, -2.0f / (zn - zf), (zn + zf) / (zn - zf)},
            {0.0f, 0.0f, 0.0f, 1.0f}
        }});
    }

    void project_perspective(
            float theta, float aspect,
            float dnear, float dfar)
    {
        float zn = -dnear, zf = -dfar;
        float cot = 1.0f / tan(theta / 2.0f * M_PI / 180.0f);

        multiply_projection_matrix(Matrix {{
            {cot / aspect, 0.0f, 0.0f, 0.0f},
            {0.0f, cot, 0.0f, 0.0f},
            {0.0f, 0.0f, (zn + zf) / (zn - zf), -2.0f * zn * zf / (zn - zf)},
            {0.0f, 0.0f, -1.0f, 0.0f}
        }});
    }

    void view_port(int vp_xmin, int vp_ymin, int vp_width, int vp_height)
    {
        int xmin = vp_xmin, xmax = vp_xmin + vp_width;
        int ymin = vp_ymin, ymax = vp_ymin + vp_height;
        internal::matrix_view_port = {{
            {(xmax - xmin) / 2.0f, 0.0f, 0.0f, (xmax + xmin) / 2.0f},
            {0.0f, (ymax - ymin) / 2.0f, 0.0f, (ymax + ymin) / 2.0f},
            {0.0f, 0.0f, 1 / 2.0f, 1 / 2.0f},
            {0.0f, 0.0f, 0.0f, 1.0f}
        }};
    }

    void transform_positions(internal::InternalPolygon &polygon, const Matrix &matrix)
    {
        for (int i = 0; i < polygon.count; i++)
            polygon.vertices[i].position = 
                (matrix * polygon.vertices[i].position).divideH();
    }

    void transform_normals(internal::InternalPolygon &polygon, const Matrix &matrix)
    {
        for (int i = 0; i < polygon.count; i++)
            polygon.vertices[i].normal =
                (matrix * polygon.vertices[i].normal).discardH();
        polygon.normal =(matrix * polygon.normal).discardH();
    }

    namespace internal
    {
        InternalPolygon::InternalPolygon(const Polygon &p) :
            count(p.count), material(*p.material)
        {
            for (int i = 0; i < count; i++)
            {
                vertices[i].position = p.vertices[i]->position;
                vertices[i].normal = p.vertices[i]->normal;
                vertices[i].color = {{0.0f, 0.0f, 0.0f, 0.0f}};
                vertices[i].tex_coord = p.vertices[i]->tex_coord;
            }

            // surface normal
            internal::compute_normal(*this);
        }
    }
       
    void render_polygon(const Polygon &p)
    {
        internal::InternalPolygon polygon(p);
        transform_positions(polygon, internal::matrix_model_view);
        transform_normals(polygon, internal::model_view_inverse_transpose);
        if (internal::culling_enabled && internal::culling(polygon))
            return;
        internal::compute_lighting_color(polygon);
        transform_positions(polygon, internal::matrix_projection);
        if (internal::clipping_enabled && internal::clipping(polygon))
            return;
        transform_positions(polygon, internal::matrix_view_port);
        internal::rasterize_polygon(polygon);
    }
}
