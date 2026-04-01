#pragma once
#include <SDL3/SDL.h>

class Platform {
    private:
        SDL_Window* window{};
        SDL_Renderer* renderer{};
        SDL_Texture* texture{};
    
    public:
        Platform(char const* title, int windowWidth, int windowHeight, int textureWidth, int textureHeight);
        ~Platform();
        void Update(void const* buffer, int pitch, int windowWidth, int windowHeight);
        bool ProcessInput(uint8_t* keys);
};