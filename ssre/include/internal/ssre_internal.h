#ifndef _SSRE_INTERNAL_H_
#define _SSRE_INTERNAL_H_

#include "ssre.h"

namespace ssre 
{
    namespace internal  
    {
        template<typename T>
        T copy(const T &t)
        {
            T r(t);
            return r;
        }

        extern int window_width;
        extern int window_height;
        extern Matrix matrix_model_view;
        extern Matrix model_view_inverse_transpose;
        extern Matrix matrix_projection;
        extern Matrix matrix_view_port;

        struct InternalVertex
        {
            Vector position;
            Vector normal;
            TextureCoordiate tex_coord;
            MaterialColor color;
        };

        struct InternalPolygon
        {
            int count;
            Vector normal;
            InternalVertex vertices[SSRE_MAX_VERTEX_COUNT];
            const Material &material;
            InternalPolygon(const Polygon &p);
        };

        struct SSREBuffer
        {
            const static int LIGHTING_SOURCE_BUFFER_SIZE = 32;
            int lighting_source_count = 0;
            LightingSource lighting_sources[LIGHTING_SOURCE_BUFFER_SIZE];

            void reset();
        };
        extern SSREBuffer buffer;
        extern bool culling_enabled;
        extern bool clipping_enabled;
        extern bool z_buffer_enabled;

        bool culling(const InternalPolygon &polygon);
        bool clipping(InternalPolygon &polygon);
        void rasterize_polygon(const InternalPolygon &polygon);
        void compute_normal(InternalPolygon &polygon);

        void compute_lighting_color(InternalPolygon &polygon);

        enum TextureMode
        {
            Decal, Modulate
        };
        extern Texture texture;
        extern bool texture_enabled;
        extern TextureMode texture_mode;
        uint32 get_texture_color(float u, float v);
        uint32 modulate_color(uint32 c0, uint32 c1);
    }
}

#endif
