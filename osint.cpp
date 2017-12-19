
#include <SDL2/SDL.h>
/* #include <SDL/SDL_gfxPrimitives.h> */
/* #include <SDL/SDL_image.h> */
/* #include <SDL/SDL_rotozoom.h> */

#include "e8910.h"
#include "osint.h"
#include "vecx.h"

#include "cscreen.h"

#include <vector>

#define EMU_TIMER 20 /* the emulators heart beats at 20 milliseconds */

static SDL_Surface *screen = NULL;
static SDL_Surface *overlay_original = NULL;
static SDL_Surface *overlay = NULL;

static long scl_factor;
static long offx;
static long offy;


void renderLines(SDL_Renderer * _r, vector_type * _vecs, size_t _num) {

    SDL_SetRenderDrawColor(_r, 255, 255, 255, 200);

    for (size_t i = 0; i < _num; i++ ) {

        auto const & v = _vecs[i];

        auto x1 = offx + v.x0 / scl_factor;
        auto y1 = offy + v.y0 / scl_factor;
        auto x2 = offx + v.x1 / scl_factor;
        auto y2 = offy + v.y1 / scl_factor;

        SDL_RenderDrawLine(_r, x1, y1, x2, y2);
    }

}

static char *cartfilename = NULL;

static void loadCart(void) {

    memset(cart, 0, sizeof(cart));

    if (cartfilename) {
        FILE *f;
        if (!(f = fopen(cartfilename, "rb"))) {
            perror(cartfilename);
            exit(EXIT_FAILURE);
        }
        fread(cart, 1, sizeof(cart), f);
        fclose(f);
    }

}

static void init() {
    FILE *f;


    char const *romfilename = getenv("VECTREX_ROM");

    if (romfilename == NULL) {
        romfilename = "rom.dat";
    }
    if (!(f = fopen(romfilename, "rb"))) {
        perror(romfilename);
        exit(EXIT_FAILURE);
    }
    if (fread(rom, 1, sizeof(rom), f) != sizeof(rom)) {
        printf("Invalid rom length\n");
        exit(EXIT_FAILURE);
    }
    fclose(f);

    memset(cart, 0, sizeof(cart));

    loadCart();
}

void resize(int width, int height) {
    long sclx, scly;

    long screenx = width;
    long screeny = height;

    sclx = ALG_MAX_X / width;
    scly = ALG_MAX_Y / height;

    scl_factor = sclx > scly ? sclx : scly;

    offx = (screenx - ALG_MAX_X / scl_factor) / 2;
    offy = (screeny - ALG_MAX_Y / scl_factor) / 2;
}



static void readevents() {

    SDL_Event e;

    while (SDL_PollEvent(&e)) {
        switch (e.type) {
            case SDL_QUIT:
                exit(EXIT_SUCCESS);
                break;

                /* case SDL_VIDEORESIZE: */
                /*     resize(e.resize.w, e.resize.h); */
                /*     break; */

            case SDL_KEYDOWN:
                switch (e.key.keysym.sym) {
                    case SDLK_r:
                        // reload the cartridge
                        loadCart();
                        // reset the vectrex
                        vecx_reset();
                        break;
                    case SDLK_q:
                    case SDLK_ESCAPE:
                        exit(EXIT_SUCCESS);
                    case SDLK_a:
                        snd_regs[14] &= ~0x01;
                        break;
                    case SDLK_s:
                        snd_regs[14] &= ~0x02;
                        break;
                    case SDLK_d:
                        snd_regs[14] &= ~0x04;
                        break;
                    case SDLK_f:
                        snd_regs[14] &= ~0x08;
                        break;
                    case SDLK_LEFT:
                        alg_jch0 = 0x00;
                        break;
                    case SDLK_RIGHT:
                        alg_jch0 = 0xff;
                        break;
                    case SDLK_UP:
                        alg_jch1 = 0xff;
                        break;
                    case SDLK_DOWN:
                        alg_jch1 = 0x00;
                        break;
                    default:
                        break;
                }
                break;
            case SDL_KEYUP:
                switch (e.key.keysym.sym) {
                    case SDLK_a:
                        snd_regs[14] |= 0x01;
                        break;
                    case SDLK_s:
                        snd_regs[14] |= 0x02;
                        break;
                    case SDLK_d:
                        snd_regs[14] |= 0x04;
                        break;
                    case SDLK_f:
                        snd_regs[14] |= 0x08;
                        break;
                    case SDLK_LEFT:
                        alg_jch0 = 0x80;
                        break;
                    case SDLK_RIGHT:
                        alg_jch0 = 0x80;
                        break;
                    case SDLK_UP:
                        alg_jch1 = 0x80;
                        break;
                    case SDLK_DOWN:
                        alg_jch1 = 0x80;
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }
    }
}



int main(int argc, char *argv[]) {


    float aspect = float( logicalWidth ) / float( logicalHeight );

    unsigned desiredWidth = 480;

    unsigned desiredHeight = float(desiredWidth) / aspect;


    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        fprintf(stderr, "Failed to initialize SDL: %s\n", SDL_GetError());
        exit(-1);
    }

    auto sdlWindow = SDL_CreateWindow("My Game Window",
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            desiredWidth, desiredHeight,
            0);

    auto sdlRenderer = SDL_CreateRenderer(sdlWindow, -1, SDL_RENDERER_ACCELERATED);
    /* auto sdlRenderer = SDL_CreateRenderer(sdlWindow, -1, 0); */

    auto sdlTexture = SDL_CreateTexture(sdlRenderer,
            SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STREAMING,
            logicalWidth, logicalHeight);

    SDL_SetTextureBlendMode(sdlTexture, SDL_BLENDMODE_NONE);

    SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 255);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
    SDL_RenderSetLogicalSize(sdlRenderer, logicalWidth, logicalHeight);

    cScreen scr;

    e8910_init_sound();
    init();
    vecx_reset();

    Uint32 next_time = SDL_GetTicks() + EMU_TIMER;

    while (true) {

        resize(logicalWidth, logicalHeight);

        SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 255);
        SDL_RenderClear(sdlRenderer);

        Uint32 next_time = SDL_GetTicks() + EMU_TIMER;
        vecx_emu((VECTREX_MHZ / 1000) * EMU_TIMER);
        renderLines(sdlRenderer, vectors_draw, vector_draw_cnt);
        SDL_RenderPresent(sdlRenderer);

        readevents();

        {
            Uint32 now = SDL_GetTicks();

            if (now < next_time)
                SDL_Delay(next_time - now);
            else
                next_time = now;
            next_time += EMU_TIMER;
        }

    }




    return 0;
}
