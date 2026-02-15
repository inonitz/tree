#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <gtest/gtest.h>
#include <cstdio>


// int main(int argc, char* argv[])
// {
//     int width = 1280;
//     int height = 720;

//     SDL_Init(SDL_INIT_VIDEO);
//     SDL_Window* window = SDL_CreateWindow("SDL pixels", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);
//     SDL_Surface* screen = SDL_GetWindowSurface(window);
//     SDL_Surface* pixels = SDL_CreateRGBSurfaceWithFormat(0, width, height, 32, SDL_PIXELFORMAT_RGBX8888);

//     unsigned int t1 = SDL_GetTicks();
//     float pos = 0;

//     for (;;)
//     {
//         SDL_Event ev;
//         while (SDL_PollEvent(&ev))
//         {
//             if (ev.type == SDL_QUIT)
//             {
//                 return 0;
//             }
//         }

//         unsigned int t2 = SDL_GetTicks();
//         float delta = (t2 - t1) / 1000.0f;
//         t1 = t2;

//         // clear pixels to black background
//         SDL_FillRect(pixels, NULL, 0);

//         // write the pixels
//         SDL_LockSurface(pixels);
//         {
//             int pitch = pixels->pitch;

//             // move 100 pixels/second
//             pos += delta * 100.0f;
//             pos = fmodf(pos, width);

//             // draw red diagonal line
//             for (int i=0; i<height; i++)
//             {
//                 int y = i;
//                 int x = ((int)pos + i) % width;

//                 unsigned int* row = (unsigned int*)((char*)pixels->pixels + pitch * y);
//                 row[x] = 0xff0000ff; // 0xAABBGGRR
//             }
//         }
//         SDL_UnlockSurface(pixels);

//         // copy to window
//         SDL_BlitSurface(pixels, NULL, screen, NULL);
//         SDL_UpdateWindowSurface(window);
//     }
// }



int main(int argc, char *argv[]) {
    // Also needed for my dual main problem
    SDL_SetMainReady();


    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::printf("FATAL: Cannot init SDL: %s\n", SDL_GetError());
        return -1;
    }

    ::testing::InitGoogleTest(&argc, argv);
    int result = RUN_ALL_TESTS();  // Store the results in a variable

    SDL_Quit();  // Quit SDL properly
    return result;  // Return the result, as required by google test
}