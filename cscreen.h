#ifndef CSCREEN_H_JQDWHTDS
#define CSCREEN_H_JQDWHTDS

#include <SDL2/SDL.h>
#include <vector>


const unsigned int logicalWidth = 330;
const unsigned int logicalHeight = 410;
const unsigned int logicalArea  = logicalWidth * logicalHeight;


class cScreen {

    public:
        cScreen() {
            mScreen.resize(logicalArea);
        }

        void clear(float r, float g, float b, float a ) {

            auto ur = uint8_t(r * 255);
            auto ug = uint8_t(g * 255);
            auto ub = uint8_t(b * 255);
            auto ua = uint8_t(a * 255);

            auto pixel = mkRGBA(ur,ug,ub,ua);

            for(size_t i = 0; i < logicalArea; i++) {
                mScreen[i] = pixel;
            }
        }

        void flip(SDL_Texture * _texture) const {
            SDL_UpdateTexture(_texture, NULL, mScreen.data(), logicalWidth * 4) ;
        }


    protected:
        static uint32_t mkRGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a )  {
            return 
                r |
                g << 8 |
                b << 16 |
                a << 24;

        }

        std::vector<uint32_t> mScreen;
};


#endif /* end of include guard: CSCREEN_H_JQDWHTDS */
