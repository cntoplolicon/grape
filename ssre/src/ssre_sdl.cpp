#include <memory>
#include <stdexcept>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "ssre.h"

namespace ssre
{
    const int SSRE_WINDOW_DEFAULT_X = SDL_WINDOWPOS_UNDEFINED;
    const int SSRE_WINDOW_DEFAULT_Y = SDL_WINDOWPOS_UNDEFINED;

    const uint32 SSRE_WINDOW_OPENGL = SDL_WINDOW_OPENGL;

    static SDL_Window *sdl_window = nullptr;
    static SDL_Renderer *sdl_renderer = nullptr;
    static SDL_Texture *sdl_texture = nullptr;

    void present_impl(uint32 *pixels, int pitch)
    {
        SDL_UpdateTexture(sdl_texture, nullptr, (const void *)pixels, pitch);
        SDL_RenderCopy(sdl_renderer, sdl_texture, nullptr, nullptr);
        SDL_RenderPresent(sdl_renderer);
    }

    void init_window_impl(const char *title, int x, int y,
            int width, int height, uint32 flags)
    {   
        SDL_Init(SDL_INIT_EVERYTHING);
        sdl_window = SDL_CreateWindow(title, x, y, width, height, flags);
        if (!sdl_window)
            throw new std::runtime_error("failed initializing SDL window");
        sdl_renderer = SDL_CreateRenderer(sdl_window, -1,
                SDL_RENDERER_ACCELERATED);
        if (!sdl_renderer)
            throw new std::runtime_error("failed initializing SDL renderer");
        sdl_texture = SDL_CreateTexture(sdl_renderer, SDL_PIXELFORMAT_ARGB8888, 
                SDL_TEXTUREACCESS_STREAMING, width, height);
        if (!sdl_texture)
            throw new std::runtime_error("failed initializing SDL texture");
    }

    void delay(uint32 time)
    {
        SDL_Delay(time);
    }

    void destroy_window_impl()
    {
        if (sdl_texture)
            SDL_DestroyTexture(sdl_texture);
        if (sdl_renderer)
            SDL_DestroyRenderer(sdl_renderer);
        if (sdl_window)
            SDL_DestroyWindow(sdl_window);
        SDL_Quit();
    }

    Texture load_external_texture_impl(const char *file)
    {
        SDL_Surface *image = IMG_Load(file);
        if (!image)
            throw new std::runtime_error("failed loading exteral texture");
        SDL_Surface *converted = SDL_ConvertSurfaceFormat(image, SDL_PIXELFORMAT_ARGB8888, 0);
        if (!converted)
            throw new std::runtime_error("failed to convert pixel format");
        if (SDL_LockSurface(converted) != 0)
            throw new std::runtime_error("failed locking surface");
        int w = converted->w, h = converted->h;
        Texture texture = {w, h, new uint32[w * h]};
        if (converted->pitch == (w << 2))
            memcpy(texture.pixels, converted->pixels, sizeof(uint32) * w * h);
        else
        {
            uint32 *dest = texture.pixels;
            uint8 *src = (uint8*)converted->pixels;
            for (int i = 0; i < h; i++, dest += w, src += converted->pitch)
                memcpy(dest, src, w << 2);
        }
        SDL_UnlockSurface(converted);
        if (image)
            SDL_FreeSurface(image);
        if (converted)
            SDL_FreeSurface(converted);
        return texture;
    }

    void main_loop(render_function func)
    {
        SDL_Event event;
        bool running = true;
        while (running)
        {
            if (SDL_PollEvent(&event))
            {
                switch (event.type)
                {
                    case SDL_QUIT:
                        running = false;
                        break;
                }    
            }
            if (func)
            {
                func();
                present();
            }
        }
    }
}
