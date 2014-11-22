#include <algorithm>
#include <new>
#include <cstring>
#include <cassert>
#include <stdexcept>
#include "ssre.h"
#include "ssre_util.h"
#include "internal/ssre_internal.h"

namespace ssre
{
    enum PolygonRenderingMode
    {
        FILL, WIREFRAME
    };
    PolygonRenderingMode polygon_rendering_mode = FILL;
    uint32 wireframe_color = 0xffffffff;

    void polygon_render_fill()
    {
        polygon_rendering_mode = FILL;
    }

    void polygon_render_wireframe()
    {
        polygon_rendering_mode = WIREFRAME;
    }

    void set_wireframe_color(uint32 color)
    {
        wireframe_color = color;
    }

    namespace internal 
    {
        int window_width = 0;
        int window_height = 0;

        void fill_polygon(const InternalPolygon &polygon)
        {
            Pointi points[SSRE_MAX_VERTEX_COUNT];
            MaterialColor colors[SSRE_MAX_VERTEX_COUNT];
            float z_values[SSRE_MAX_VERTEX_COUNT];
            float u_values[SSRE_MAX_VERTEX_COUNT];
            float v_values[SSRE_MAX_VERTEX_COUNT];
            for (int i = 0; i < polygon.count; i++)
            {
                const Vector &v = polygon.vertices[i].position;
                points[i] = {(int)(v.x() + 0.5f), (int)(v.y() + 0.5f)};
                colors[i] = polygon.vertices[i].color;
                z_values[i] = polygon.vertices[i].position.z();
                u_values[i] = polygon.vertices[i].tex_coord.u;
                v_values[i] = polygon.vertices[i].tex_coord.v;
            }
            fill_polygon(points, colors, z_values, 
                    u_values, v_values, polygon.count);
        }

        void draw_wire_frame(const InternalPolygon &polygon)
        {
            for (int i = 0, n = polygon.count; i < n; i++)
            {
                const Vector &v0 = polygon.vertices[i].position;
                const Vector &v1 = polygon.vertices[(i + 1) % n].position;
                Pointi p0 = {(int)(v0.x() + 0.5f), (int)(v0.y() + 0.5f)};
                Pointi p1 = {(int)(v1.x() + 0.5f), (int)(v1.y() + 0.5f)};
                draw_line(p0, p1, wireframe_color);
            }
        }

        void rasterize_polygon(const InternalPolygon &polygon)
        {
            if (polygon_rendering_mode == FILL)
                fill_polygon(polygon);
            else
                draw_wire_frame(polygon);
        }
    }

    /* width and height of the buffer, should be same as window */
    int width = 0;
    int height = 0;
    uint32 *pixels = nullptr;
    float *depths = nullptr;

    void init_window(const char *title, int x, int y,
            int _width, int _height, uint32 flags)
    {
        width = _width;
        height = _height;
        internal::window_width = width;
        internal::window_height = height;
        pixels = new uint32[width * height];
        depths = new float[width * height];
        init_window_impl(title, x, y, width, height, flags);
    }

    void destroy_window()
    {
        destroy_window_impl();

        delete[] pixels;
        pixels = nullptr;

        delete[] depths;
        depths = nullptr;
    }

    void clear(uint32 color)
    {
        for (int i = 0; i < width * height; ++i)
            pixels[i] = color;
    }

    void clear_depth(float d)
    {
        for (int i = 0; i < width * height; i++)
            depths[i] = d;
    }

    void present()
    {
        present_impl(pixels, width << 2);
        internal::buffer.reset();
    }

    void draw_points(const Pointi *points, uint32 color, int n)
    {
        for (int i = 0; i < n; ++i) 
        {
            int index = (height - 1 - points[i].y) * width + points[i].x;
#ifdef DEBUG
            assert(index >= 0 && index < width * height);
            assert(points[i].x >= 0 && points[i].x < width && 
                    points[i].y >= 0 && points[i].y < height);
#endif
            pixels[index] = color;        
        }
    }

    void draw_points(const Pointi *points, uint32 *colors, int n)
    {
        for (int i = 0; i < n; ++i) 
        {
            int index = (height - 1 - points[i].y) * width + points[i].x;
#ifdef DEBUG
            assert(index >= 0 && index < width * height);
            assert(points[i].x >= 0 && points[i].x < width && 
                    points[i].y >= 0 && points[i].y < height);
#endif
            pixels[index] = colors[i];        
        }
    }

    void draw_line(const Pointi &p0, const Pointi &p1, uint32 color)
    {
        int x0 = p0.x, y0 = p0.y, x1 = p1.x, y1 = p1.y;
        int dx = abs(x1 - x0);
        int dy = abs(y1 - y0);
        int sx = x0 < x1 ? 1 : -1;
        int sy = y0 < y1 ? 1 : -1;
        int err = dx - dy;
        int index = (height - 1 - y0) * width + x0;
        int index_dy = sy == 1 ? -width : width;

        while (true)
        {
            if (x0 == x1 && y0 == y1)
                break;
#ifdef DEBUG
            assert(index >= 0 && index < width * height);
            assert(x0 >= 0 && x0 < width && y0 >= 0 && y0 < height);
#endif
            pixels[index] = color;
            int e2 = (err << 1);
            if (e2 > -dy) {
                err = err - dy;
                x0 = x0 + sx;
                index += sx;
            }
            if (e2 < dx)
            {
                err = err + dx;
                y0 = y0 + sy;
                index += index_dy;
            }
        }
    }

    struct iEdgeNode
    {
        const Pointi *p0 = nullptr;
        const Pointi *p1 = nullptr;
        int dx = 0;
        int dy = 0;
        int sx = 0;
        int di = 0;
        int err = 0;
        int x = 0;
        MaterialColor color = {{0.0f, 0.0f, 0.0f, 0.0f}};
        MaterialColor dc = {{0.0f, 0.0f, 0.0f, 0.0f}};
        float z;
        float dz;
        float u;
        float du;
        float v;
        float dv;

        iEdgeNode() {}

        iEdgeNode(const ssre::Pointi *_p0, const ssre::Pointi *_p1, 
                const MaterialColor* c0, const MaterialColor *c1, 
                float z0, float z1, float u0, float u1, float v0, float v1) :
            p0(_p0), p1(_p1)
        {
            if (p0->y > p1->y)
            {
                std::swap(z0, z1);
                std::swap(c0, c1);
                std::swap(p0, p1);
                std::swap(u0, u1);
                std::swap(v0, v1);
            }
            dy = p1->y - p0->y;
            dx = abs(p1->x - p0->x);
            sx = dx == 0 ? 0 : (p1->x > p0->x ? 1 : -1);
            di = dx / dy * sx;
            dx = dx % dy;
            x = p0->x;
            color = *c0;
            dc = (*c1 - *c0) / dy;
            z = z0;
            dz = (z1 - z0) / dy;
            u = u0;
            du = (u1 - u0) / dy;
            v = v0;
            dv = (v1 - v0) / dy;
        }

        bool operator < (const iEdgeNode &node) const
        {
            if (p0->y != node.p0->y)
                return p0->y < node.p0->y;
            if (p0->x != node.p0->x)
                return p0->x < node.p0->x;
            return (node.p1->y - node.p0->y) * (p1->x - p0->x) <
            (p1->y - p0->y) * (node.p1->x - node.p0->x);
        }
    };

    void fill_polygon(const Pointi *points, const MaterialColor *colors, 
            const float *z_values, 
            const float *u_values, const float *v_values, int n)
    {
        if (n < 3)
            throw new std::invalid_argument("less than 3 vertices");
        // prepare the edge list
        typedef LinkedListNode<iEdgeNode> ListNode;
        iEdgeNode *nodes =(iEdgeNode *)::operator new(n * sizeof(iEdgeNode));
        ListNode *list_nodes = (ListNode *)::operator new(n * sizeof(ListNode));
        int count = 0;
        for (int i = 0; i < n; i++)
        {
            int j = (i + 1) % n;
            const Pointi *p0 = &points[i], *p1 = &points[j];
            const MaterialColor *c0 = &colors[i], *c1 = &colors[j];
            float z0 = z_values[i], z1 = z_values[j];
            float u0 = u_values[i], u1 = u_values[j];
            float v0 = v_values[i], v1 = v_values[j];
            if (p0->y == p1->y)
                continue;
            new (&nodes[count]) iEdgeNode(p0, p1, c0, c1, z0, z1, 
                    u0, u1, v0, v1);
            new (&list_nodes[count]) ListNode(&nodes[count]);
            count++;
        }
        std::sort(nodes, nodes + count);

        // initialize the intersect list
        iEdgeNode head_node;
        LinkedListNode<iEdgeNode> head(&head_node);
        head.insert(&list_nodes[0]);
        int i = 1;
        for (; i < count && nodes[i].p0->y == nodes[0].p0->y; ++i)
            list_nodes[i - 1].insert(&list_nodes[i]);
        int y = nodes[0].p0->y;
        uint32 *pixel_line = &pixels[(height - 1 - y) * width];
        float *depths_line = &depths[(height - 1 - y) * width];
        while (head.next)
        {
            // render pixels
            for (ListNode *node = head.next; node; node = node->next->next)
            {
                iEdgeNode *curr = node->payload;
                // left edge. the actual position of x rx =
                // x + sx * err / dy. Always start from ceil(rx)
                // round up if there is positive fractional part
                int x_left = curr->x;
                if (curr->err && curr->sx == 1)
                    ++x_left;
                MaterialColor c = curr->color;
                float z = curr->z;
                float u = curr->u;
                float v = curr->v;

                // right edge. If it's an integer, it's counted
                // outside the polygon, otherwise end at floor(rx)
                curr = node->next->payload;
                int x_right = curr->x;
                if (!curr->err || curr->sx == -1)
                    --x_right;
                MaterialColor cdif = curr->color - c;
                float zdif = curr->z - z;
                float udif = curr->u - u;
                float vdif = curr->v - v;
                if (x_right != x_left)
                {
                    float dif = x_right - x_left;
                    cdif /= dif;
                    zdif /= dif;
                    udif /= dif;
                    vdif /= dif;
                }
                uint32 *segment = pixel_line + x_left;
                float *d_segment = depths_line + x_left;
                for (int x = x_left; x <= x_right; x++)
                {
#ifdef DEBUG
                    int index = segment - pixels;
                    assert(index >= 0 && index < width * height);
                    assert(x >= 0 && x < width && y >= 0 && y < height);
#endif
                    if (!internal::z_buffer_enabled || z < *d_segment)
                    {
                        *d_segment = z;
                        uint32 color = c.toARGB();
                        if (internal::texture_enabled)
                        {
                            uint32 tc = internal::get_texture_color(u, v);
                            if (internal::texture_mode == internal::Modulate)
                                color = internal::modulate_color(color, tc);
                            else if (internal::texture_mode == internal::Decal)
                                color = tc;
                        }
                        *segment = color;
                    }
                    c += cdif;
                    z += zdif;
                    u += udif;
                    v += vdif;
                    segment++;
                    d_segment++;
                }

            }
            // move the scan line up one pixel
            ++y;
            pixel_line -= width;
            depths_line -= width;
            // remove all edges whose top is reached by the scan line
            for (ListNode *node = head.next; node; node = node->next)
            {
                if (node->payload->p1->y == y)
                    node->remove();
            }
            // update the intersect point
            for (ListNode *node = head.next; node; node = node->next)
            {
                iEdgeNode *curr = node->payload;
                curr->x += curr->di;
                curr->err += curr->dx;
                if (curr->err >= curr->dy)
                {
                    curr->err -= curr->dy;
                    curr->x += curr->sx;
                }
                curr->color += curr->dc;
                curr->z += curr->dz;
                curr->u += curr->du;
                curr->v += curr->dv;
            }
            // insert all edges whose bottom is reached by the scan line
            ListNode *node = &head;
            while (i < count && nodes[i].p0->y == y)
            {
                while (node->next && node->next->payload->x < nodes[i].x)
                    node = node->next;
                node->insert(&list_nodes[i]);
                ++i;
                node = node->next;
            }
        }

        delete list_nodes;
        delete nodes;
    }
}
