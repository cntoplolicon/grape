#include <cmath>
#include "ssre.h"
#include "internal/ssre_internal.h"

namespace ssre 
{
    uint32 MaterialColor::toARGB()
    {
        int ia = (int)(a() * 255.0f);
        int ir = (int)(r() * 255.0f);
        int ig = (int)(g() * 255.0f);
        int ib = (int)(b() * 255.0f);
        return SSRE_ARGB(ia, ir, ig, ib);
    }

    MaterialColor& MaterialColor::operator+= (const MaterialColor &c)
    {
        for (int i = 0; i < SSRE_LIGHTING_COMPONENT; i++)
            color[i] = color[i] + c.color[i];
        return *this;
    }

    MaterialColor MaterialColor::operator+ (const MaterialColor &c) const
    {
        MaterialColor r = *this;
        r += c;
        return r;
    }

    MaterialColor& MaterialColor::operator-= (const MaterialColor &c)
    {
        for (int i = 0; i < SSRE_LIGHTING_COMPONENT; i++)
            color[i] = color[i] - c.color[i];
        return *this;
    }

    MaterialColor MaterialColor::operator- (const MaterialColor &c) const
    {
        MaterialColor r = *this;
        r -= c;
        return r;
    }

    MaterialColor& MaterialColor::operator*= (float f)
    {
        for (int i = 0; i < SSRE_LIGHTING_COMPONENT; i++)
            color[i] = color[i] * f;
        return *this;
    }

    MaterialColor MaterialColor::operator* (float f) const
    {
        MaterialColor r = *this;
        r *= f;
        return r;
    }

    MaterialColor& MaterialColor::operator/= (float f)
    {
        for (int i = 0; i < SSRE_LIGHTING_COMPONENT; i++)
            color[i] = color[i] / f;
        return *this;
    }

    MaterialColor MaterialColor::operator/ (float f) const
    {
        MaterialColor r = *this;
        r /= f;
        return r;
    }

    void LightingSource::contribute_lighting(
            const Material &material, const Vector &vertex_position, 
            const Vector &vertex_normal, MaterialColor &color) const
    {
        Vector N = vertex_normal.normalize();
        Vector V = -vertex_position.discardH().normalize();
        Vector L = type == DirectionalSource ? direction : (position - vertex_position);
        float dist = L.length();
        L = L / dist;
        Vector H = (L + V).normalize();

        float NL = N.dot_product(L);
        float NH = N.dot_product(H);
        float NHS = std::max(0.0, pow(NH, material.shininess));

        float attenuation_factor = 1.0f;
        if (type != DirectionalSource)
            attenuation_factor = 1.0f / (attenuation.constant + 
                    attenuation.linear * dist + 
                    attenuation.quadratic * dist * dist);

        float spotlight_factor = 1.0f;
        if (type == SpotLight)
        {
            Vector E = -L;
            float ED = E.dot_product(direction);
            if (ED < cos(spotlight_cutoff * M_PI / 180.0))
                spotlight_factor = 0.0f;
            else
                spotlight_factor = std::max(0.0f, ED);
        }

        for (int i = 0; i < SSRE_LIGHTING_COMPONENT; i++)
        {
            float contribution = material.ambient.color[i] * 
                colors.ambient.color[i];
            if (NL > 0.0f)
            {
                contribution += 
                    NL * material.diffuse.color[i] * 
                    colors.diffuse.color[i]
                    + NHS * material.specular.color[i] * 
                    colors.specular.color[i];
            }
            contribution = attenuation_factor * 
                spotlight_factor * contribution;
            color.color[i] += contribution;
        }
    }

    void LightingSource::transform()
    {
        position = (internal::matrix_model_view * position).divideH();
        direction = (internal::model_view_inverse_transpose
                * direction).discardH().normalize();
    }

    int enable_light(const LightingSource &source)
    {
        int i = internal::buffer.lighting_source_count++;
        LightingSource *internal_source = &internal::buffer.lighting_sources[i];
        *internal_source = source;
        internal_source->transform();
        return i;
    }

    void enable_light(int handle)
    {
        internal::buffer.lighting_sources[handle].disabled = false;
    }

    void disable_light(int handle)
    {
        internal::buffer.lighting_sources[handle].disabled = true;
    }

    namespace internal 
    {
        void compute_lighting_color(InternalPolygon &polygon)
        {
            // clear colors
            for (int i = 0; i < polygon.count; i++)
                polygon.vertices[i].color = {{0.0f, 0.0f, 0.0f, 0.0f}};
            // compute the contribution of each point source
            for (int i = 0; i < polygon.count; i++)
            {
                InternalVertex &vertex = polygon.vertices[i];
                for (int j = 0; j < buffer.lighting_source_count; j++)
                {
                    const LightingSource &source= buffer.lighting_sources[j];
                    if (!source.disabled)
                        source.contribute_lighting(polygon.material, 
                                vertex.position, vertex.normal, vertex.color);
                }
            }
            // normalize
            for (int i = 0; i < polygon.count; i++)
                for (int j = 0; j < SSRE_LIGHTING_COMPONENT; j++)
                    polygon.vertices[i].color.color[j] = std::min(1.0f, 
                        std::max(polygon.vertices[i].color.color[j], 0.0f));
        }    
    }
}
