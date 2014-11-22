#include <stdexcept>
#include <cstring>
#include "ssre.h"
#include "internal/ssre_internal.h"

namespace ssre 
{
    void enable_culling()
    {
        internal::culling_enabled = true;
    }

    void disable_culling()
    {
        internal::culling_enabled = false;
    }

    void enable_clipping()
    {
        internal::clipping_enabled = true;
    }

    void disable_clipping()
    {
        internal::clipping_enabled = false;
    }

    void enable_z_buffer()
    {
        internal::z_buffer_enabled = true;
    }

    void disable_z_buffer()
    {
        internal::z_buffer_enabled = false;
    }

    namespace internal 
    {
        bool culling_enabled = false;
        bool clipping_enabled = false;
        bool z_buffer_enabled = false;

        void compute_normal(InternalPolygon &polygon)
        {
            int n = polygon.count;
            for (int i = 0; i < n; i++)
            {
                Vector& v0 = polygon.vertices[i].position;
                Vector& v1 = polygon.vertices[(i + 1) % n].position;
                Vector& v2 = polygon.vertices[(i + 2) % n].position;
                Vector e0 = v0 - v1;
                Vector e1 = v1 - v2;
                polygon.normal = (e0 * e1).normalize();
                if (!polygon.normal.is_zero_vector())
                    break;
            }
        }

        bool culling(const InternalPolygon &polygon)
        {
            const Vector &position = polygon.vertices[0].position;
            const Vector &normal = polygon.normal;
            return position.dot_product(normal) > 0.0f;
        }

        enum ClipBoundary
        {
            LEFT, RIGHT, BOTTOM, TOP, FRONT, BACK
        };

        bool inside_clip_bound(const Vector &v, ClipBoundary boundary)
        {
            switch (boundary)
            {
                case ClipBoundary::LEFT:
                    return v.x() >= -1.0f;
                case ClipBoundary::RIGHT:
                    return v.x() <= 1.0f;
                case ClipBoundary::BOTTOM:
                    return v.y() >= -1.0f;
                case ClipBoundary::TOP:
                    return v.y() <= 1.0f;
                case ClipBoundary::FRONT:
                    return v.z() >= -1.0f;
                case ClipBoundary::BACK:
                    return v.z() <= 1.0f;
            }
            throw new std::invalid_argument("unkown clipping boundary");
        }

        Vector intersect(const Vector &v0, const Vector &v1, 
                ClipBoundary boundary)
        {
            float x0 = v0.x(), y0 = v0.y(), z0 = v0.z();
            float x1 = v1.x(), y1 = v1.y(), z1 = v1.z();
            float x = 0.0f, y = 0.0f, z = 0.0f;
            switch (boundary)
            {
                case ClipBoundary::LEFT:
                case ClipBoundary::RIGHT:
                    x = boundary == ClipBoundary::LEFT ? -1.0f : 1.0f;
                    y = y0 + (y1 - y0) / (x1 - x0) * (x - x0);
                    z = z0 + (z1 - z0) / (x1 - x0) * (x - x0);
                    break;
                case ClipBoundary::BOTTOM:
                case ClipBoundary::TOP:
                    y = boundary == ClipBoundary::BOTTOM ? -1.0f : 1.0f;
                    x = x0 + (x1 - x0) / (y1 - y0) * (y - y0);
                    z = z0 + (z1 - z0) / (y1 - y0) * (y - y0);
                    break;
                case ClipBoundary::FRONT:
                case ClipBoundary::BACK:
                    z = boundary == ClipBoundary::FRONT ? -1.0f : 1.0f;
                    x = x0 + (x1 - x0) / (z1 - z0) * (z - z0);
                    y = y0 + (y1 - y0) / (z1 - z0) * (z - z0);
                    break;

            }
            return {x, y, z, 1.0f};
        }

        InternalVertex intersect(const InternalVertex &v0, const InternalVertex &v1,
                ClipBoundary boundary)
        {
            InternalVertex v;
            v.position = intersect(v0.position, v1.position, boundary);
            float full_length = (v1.position - v0.position).length();
            float part_length = (v.position - v0.position).length();
            MaterialColor cdiff = (v1.color - v0.color) / full_length;
            v.color = v0.color + cdiff * part_length;
            float udiff = (v1.tex_coord.u - v0.tex_coord.u) / full_length;
            v.tex_coord.u = v0.tex_coord.u + udiff * part_length;
            float vdiff = (v1.tex_coord.v - v0.tex_coord.v) / full_length;
            v.tex_coord.v = v0.tex_coord.v + vdiff * part_length;
            return v;
        }

        bool clipping(InternalPolygon &polygon)
        {
            InternalVertex new_vertices[SSRE_MAX_VERTEX_COUNT];
            for (int i = ClipBoundary::LEFT; i <= ClipBoundary::BACK; i++)
            {
                ClipBoundary boundary = static_cast<ClipBoundary>(i);
                int count = 0;
                for (int j = 0, n = polygon.count; j < n; j++)
                {
                    const InternalVertex &v0 = polygon.vertices[
                        (j - 1 + n) % n];
                    const InternalVertex &v1 = polygon.vertices[j];
                    bool v0_inside = inside_clip_bound(v0.position, boundary);
                    bool v1_inside = inside_clip_bound(v1.position, boundary);
                    if (v0_inside != v1_inside)
                        new_vertices[count++] = intersect(v0, v1, boundary);
                    if (v1_inside)
                        new_vertices[count++] = v1;
                }
                for (int j = 0; j < count; j++)
                    polygon.vertices[j] = new_vertices[j];
                polygon.count = count;
            }
            return polygon.count <= 1;
        }
    }
}

