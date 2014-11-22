#include "internal/ssre_internal.h"

namespace ssre 
{
    namespace internal 
    {
        bool texture_enabled = false;
        Texture texture;
        TextureMode texture_mode = Modulate;

        uint32 merge_color_component(uint8 c0, uint8 c1, float radio)
        {
            float c = (c0 / 255.0f) * (1 - radio) + (c1 / 255.0f) * radio;
            return (uint8)(255.0f * c);
        }

        uint32 merge_color(uint32 c0, uint32 c1, float radio)
        {
            uint8 a = merge_color_component(SSRE_A(c0), SSRE_A(c1), radio);
            uint8 r = merge_color_component(SSRE_R(c0), SSRE_R(c1), radio);
            uint8 g = merge_color_component(SSRE_G(c0), SSRE_G(c1), radio);
            uint8 b = merge_color_component(SSRE_B(c0), SSRE_B(c1), radio);
            return SSRE_ARGB(a, r, g, b);
        }

        uint32 get_texture_color(float u, float v)
        {
            u = u * (texture.width - 1);
            v = v * (texture.height - 1);
            int x = (int)u, y = (int)v;
            uint32 *cl0 = &texture.pixels[y * texture.width + x];
            uint32 *cl1 = cl0 + texture.width;
            uint32 c0 = cl0[0], c1 = 0, c2 = 0, c3 = 0;
            if (x + 1 < texture.width)
                c1 = cl0[1];
            if (y + 1 < texture.height)
            {
                c2 = cl1[0];
                if (x + 1 < texture.width)
                    c3 = cl1[1];
            }

            float ur = u - x, vr = v - y;
            return merge_color(merge_color(c0, c1, ur), 
                    merge_color(c2, c3, ur), vr);
            return *cl0;
        }

        uint8 modulate_color_component(uint8 c0, uint8 c1)
        {
            return (uint8)((c0 / 255.0f ) * (c1 / 255.0f) * 255.0f);
        }

        uint32 modulate_color(uint32 c0, uint32 c1)
        {
            uint8 a = modulate_color_component(SSRE_A(c0), SSRE_A(c1));
            uint8 r = modulate_color_component(SSRE_R(c0), SSRE_R(c1));
            uint8 g = modulate_color_component(SSRE_G(c0), SSRE_G(c1));
            uint8 b = modulate_color_component(SSRE_B(c0), SSRE_B(c1));
            return SSRE_ARGB(a, r, g, b);
        }
    }
    void texture_mode_decal()
    {
        internal::texture_mode = internal::Decal;
    }

    void texture_mode_modulate()
    {
        internal::texture_mode = internal::Modulate;
    }

    void enable_texture(const Texture &texture)
    {
        internal::texture = texture;
        internal::texture_enabled = true;
    }

    void disable_texture()
    {
        internal::texture_enabled = false;
    }

    Texture load_external_texture(const char *file)
    {
        return load_external_texture_impl(file);
    }

    void release_external_texture(Texture &tex)
    {
        delete[] tex.pixels;
        tex.pixels = nullptr;
        tex.width = tex.height = 0;
    }
}
