#include "ssre.h"

ssre::Vector positions[] = {
            {100, 100, 100, 1},
            {-100, 100, 100, 1},
            {-100, -100, 100, 1},
            {100, -100, 100, 1},

            {100, 100, -100, 1},
            {100, -100, -100, 1},
            {-100, -100, -100, 1},
            {-100, 100, -100, 1},

            {100, 100, 100, 1},
            {100, -100, 100, 1},
            {100, -100, -100, 1},
            {100, 100, -100, 1},

            {-100, 100, 100, 1},
            {-100, 100, -100, 1},
            {-100, -100, -100, 1},
            {-100, -100, 100, 1},

            {100, 100, 100, 1},
            {100, 100, -100, 1},
            {-100, 100, -100, 1},
            {-100, 100, 100, 1},

            {100, -100, 100, 1},
            {-100, -100, 100, 1},
            {-100, -100, -100, 1},
            {100, -100, -100, 1}         
};

uint32 checkImage[256][256];

ssre::Texture check_texture = {
    256, 256, (uint32 *)checkImage
};

ssre::Texture wood_texture;

ssre::TextureCoordiate tc0 = {0.0f, 0.0f};
ssre::TextureCoordiate tc1 = {0.0f, 1.0f};
ssre::TextureCoordiate tc2 = {1.0f, 1.0f};
ssre::TextureCoordiate tc3 = {1.0f, 0.0f};

ssre::Vertex vertices[] = {
    {positions[0], positions[0], tc0},
    {positions[1], positions[1], tc1},
    {positions[2], positions[2], tc2},
    {positions[3], positions[3], tc3},
    {positions[4], positions[4], tc0},
    {positions[5], positions[5], tc1},
    {positions[6], positions[6], tc2},
    {positions[7], positions[7], tc3},
    {positions[8], positions[8], tc0},
    {positions[9], positions[9], tc1},
    {positions[10], positions[10], tc2},
    {positions[11], positions[11], tc3},
    {positions[12], positions[12], tc0},
    {positions[13], positions[13], tc1},
    {positions[14], positions[14], tc2},
    {positions[15], positions[15], tc3},
    {positions[16], positions[16], tc0},
    {positions[17], positions[17], tc1},
    {positions[18], positions[18], tc2},
    {positions[19], positions[19], tc3},
    {positions[20], positions[20], tc0},
    {positions[21], positions[21], tc1},
    {positions[22], positions[22], tc2},
    {positions[23], positions[23], tc3},
};

ssre::MaterialColor zero = {{0, 0, 0, 1}};
ssre::MaterialColor dark = {{0.2, 0.2, 0.2, 1.0}};
ssre::MaterialColor light_dark = {{0.4, 0.4, 0.4, 1.0}};
ssre::MaterialColor light_light = {{0.6, 0.6, 0.6, 1.0}};
ssre::MaterialColor light = {{0.8, 0.8, 0.8, 1.0}};
ssre::MaterialColor one = {{1, 1, 1, 1}};

ssre::Material material = {
    zero, light_dark, light_light, 0
};

ssre::Polygon polygons[] = {
    {3, {&vertices[0], &vertices[1], &vertices[3]}, &material},
    {3, {&vertices[1], &vertices[2], &vertices[3]}, &material},
    {3, {&vertices[4], &vertices[5], &vertices[7]}, &material},
    {3, {&vertices[5], &vertices[6], &vertices[7]}, &material},
    {3, {&vertices[8], &vertices[9], &vertices[11]}, &material},
    {3, {&vertices[9], &vertices[10], &vertices[11]}, &material},

    {3, {&vertices[12], &vertices[13], &vertices[15]}, &material},
    {3, {&vertices[13], &vertices[14], &vertices[15]}, &material},

    {3, {&vertices[16], &vertices[17], &vertices[19]}, &material},
    {3, {&vertices[17], &vertices[18], &vertices[19]}, &material},

    {3, {&vertices[20], &vertices[21], &vertices[23]}, &material},
    {3, {&vertices[21], &vertices[22], &vertices[23]}, &material},
};

ssre::LightingSource light1 = {
    ssre::DirectionalSource,
    {400, 400, 400, 1},
    {0, 0, -1},
    {
        {{0, 0, 0, 0}},
        {{1, 1, 1, 1}},
        {{1, 1, 1, 1}},
    },
    {1, 0, 0},
    1.0f,
    false
};

void render()
{
    using namespace ssre;
    clear(0);
    clear_depth(1.0f);
    enable_light(light1);
    //enable_texture(wood_texture);
    int size = sizeof(polygons) / sizeof(Polygon);
    for (int i = 0; i < size; i++)
        render_polygon(polygons[i]);
} 

void make_check_image()
{
    for (int i = 0; i < 256; i++) 
    {
        for (int j = 0; j < 256; j++) 
        {
            uint8 c = ((((i & 0x10) == 0)^(((j & 0x10)) == 0))) * 255;
            checkImage[i][j] =  SSRE_ARGB(255, c, c, c);
        }
    }
}


int main() 
{
    using namespace ssre;
    init_window("SSRE Window",
            SSRE_WINDOW_DEFAULT_X, SSRE_WINDOW_DEFAULT_Y,
            800, 400, SSRE_WINDOW_OPENGL);
    init_3d_viewing();
    view_look_at(400.0f, 300.0f, 500.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
    project_perspective(45.0f, 2.0f, 200.0f, 1000.0f);
    enable_clipping();
    enable_z_buffer();
    make_check_image();
    wood_texture = load_external_texture("img/wood.jpg");
    main_loop(render);
    release_external_texture(wood_texture);
    destroy_window();
    return 0;
}
