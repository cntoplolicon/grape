#ifndef _SSRE_H_
#define _SSRE_H_

#include <iostream>
#include <memory>
#include <map>
#include <vector>
#include "ssre_types.h"

#define DISABLE_COPY_AND_ASSIGN(Class) \
    Class(const Class &) = delete; \
    Class &operator=(const Class &) = delete;

#define SSRE_ARGB(a, r, g, b) ((((a) & 0xff) << 24) | \
                              (((r) & 0xff) << 16) | \
                              (((g) & 0xff) << 8) | \
                              (((b) & 0xff)))
#define SSRE_A(c) ((c >> 24) & 0xff)
#define SSRE_R(c) ((c >> 16) & 0xff)
#define SSRE_G(c) ((c >> 8) & 0xff)
#define SSRE_B(c) ((c) & 0xff)

namespace ssre
{
    extern const int SSRE_WINDOW_DEFAULT_X;
    extern const int SSRE_WINDOW_DEFAULT_Y;

    extern const uint32 SSRE_WINDOW_FULLSCREEN;
    extern const uint32 SSRE_WINDOW_RESIZABLE;
    extern const uint32 SSRE_WINDOW_OPENGL;

    const int SSRE_MAX_VERTEX_COUNT = 16;
    const int SSRE_MATRIX_DIMENSION = 4;
    const int SSRE_VECTOR_DIMENSION = 4;

    const int SSRE_LIGHTING_COMPONENT = 4;

    struct Pointi
    {
        int x, y;
    };

    struct Rect
    {
        float top, right, bottom, left;
    };

    struct Vector
    {
        float v[SSRE_VECTOR_DIMENSION];
        Vector();
        Vector(float x, float y, float z);
        Vector(float x, float y, float z, float h);

        Vector& operator+= (const Vector &v);
        Vector operator+ (const Vector &v) const;
        Vector& operator-= (const Vector &v);
        Vector operator- (const Vector &v) const;
        Vector operator- () const;
        Vector& operator*= (float f);
        Vector operator* (float f) const;
        Vector& operator*= (const Vector &v);
        Vector operator* (const Vector &v) const;
        Vector& operator/= (float f);
        Vector operator/ (float f) const;
        float length() const;
        float dot_product(const Vector &v) const;
        Vector normalize() const;
        Vector divideH() const;
        Vector discardH() const;

        float x() const { return v[0]; }
        float y() const { return v[1]; }
        float z() const { return v[2]; }
        float h() const { return v[3]; }

        bool is_zero_vector() const;
    };

    struct Matrix
    {
        float v[SSRE_MATRIX_DIMENSION][SSRE_MATRIX_DIMENSION];
        Matrix& operator+= (const Matrix &m);
        Matrix operator+ (const Matrix &m) const;
        Matrix& operator-= (const Matrix &m);
        Matrix operator- (const Matrix &m) const;
        Matrix operator- () const;
        Matrix& operator*= (const Matrix &m);
        Matrix operator* (const Matrix &m) const;
        Matrix& operator/= (float f);
        Matrix operator/ (float f) const;
        Matrix transpose() const;
        Matrix inverse() const;
    };
    Vector operator*(const Matrix &matrix, const Vector &v);
    bool is_zero(float x);

    struct TextureCoordiate
    {
        float u;
        float v;
    };

    struct Vertex
    {
        Vector normal;
        Vector position;
        TextureCoordiate tex_coord;
    };

    struct MaterialColor
    {
        float color[SSRE_LIGHTING_COMPONENT];
        float r() { return color[0]; }
        float g() { return color[1]; }
        float b() { return color[2]; }
        float a() { return color[3]; }
        MaterialColor operator+ (const MaterialColor &c) const;
        MaterialColor& operator+= (const MaterialColor &c);
        MaterialColor operator- (const MaterialColor &c) const;
        MaterialColor& operator-= (const MaterialColor &c);
        MaterialColor operator* (float f) const;
        MaterialColor& operator*= (float f);
        MaterialColor operator/ (float f) const;
        MaterialColor& operator/= (float f);
        uint32 toARGB();
    };

    struct Material
    {
        MaterialColor ambient;
        MaterialColor diffuse;
        MaterialColor specular;
        float shininess;
    };

    struct LightColors
    {
        MaterialColor ambient;
        MaterialColor diffuse;
        MaterialColor specular;
    };

    struct Polygon
    {
        int count;
        Vertex *vertices[SSRE_MAX_VERTEX_COUNT];
        Material *material;
    };

    struct Attenuation
    {
        float constant;
        float linear;
        float quadratic;
    };

    enum LightingSourceType
    {
        PointSource,
        DirectionalSource,
        SpotLight
    };

    struct LightingSource
    {
        LightingSourceType type;
        Vector position;
        Vector direction;
        LightColors colors;
        Attenuation attenuation;
        float spotlight_cutoff;
        bool disabled;
        void contribute_lighting(const Material &material, 
                const Vector &vertex_position, const Vector &vertex_normal,
                MaterialColor &color) const;
        void transform();
    };

    struct Texture
    {
        int width, height;
        uint32 *pixels;
    };

    // rasterize
    void present();
    void present_impl(uint32 *pixels, int pitch);
    void clear(uint32 color);
    void draw_points(const Pointi[], uint32 color, int n);
    void draw_points(const Pointi[], uint32 colors[], int n);
    void draw_line(
            const Pointi &p0, const Pointi &p1, uint32 color);
    void fill_polygon(const Pointi[], const MaterialColor[], 
            const float z[], const float u[], const float v[], int n);
    void polygon_render_fill();
    void polygon_render_wireframe();
    void set_wireframe_color(uint32 color);

    // basic functions
    void init_window(const char *title, int x, int y,
            int width, int height, uint32 flags);
    void init_window_impl(const char *title, int x, int y,
            int width, int height, uint32 flags);
    void delay(uint32 time);
    void destroy_window();
    void destroy_window_impl();

    // main loop
    typedef void (*render_function)();
    void main_loop(render_function func);

    // 3d viewing
    void init_3d_viewing();
    void view_look_at(
            float x0, float y0, float z0, 
            float xref, float yref, float zref,
            float vx, float vy, float vz);
    void project_ortho(
            float xmin, float xmax, 
            float ymin, float ymax,
            float dnear, float nfar);
    void project_perspective(
            float theta, float aspect,
            float dnear, float dfar);
    void load_identity_matrix(Matrix &m);
    void load_identity_model_view();
    void load_identity_projection();
    void render_polygon(const Polygon &polygon);
    void view_port(int vp_xmin, int vp_ymin, int vp_width, int vp_height);
    void translate(float tx, float ty, float tz);
    void rotate(float theta, float vx, float vy, float vz);
    void scale(float sx, float sy, float sz);

    // hidden face removal
    void enable_culling();
    void disable_culling();
    void enable_clipping();
    void disable_clipping();
    void enable_z_buffer();
    void disable_z_buffer();
    void clear_depth(float d);

    // lighting
    int enable_light(const LightingSource &source);
    void disable_light(int handle);
    void enable_light(int handle);

    // texture
    void enable_texture(const Texture &texture);
    void disable_texture();
    void texture_mode_decal();
    void texture_mode_modulate();
    Texture load_external_texture(const char *file);
    Texture load_external_texture_impl(const char *file);
    void release_external_texture(Texture &texture);
}

#endif
