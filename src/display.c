#include "display.h"

SDL_Window *window     = NULL;
SDL_Renderer *renderer = NULL;

u_int32_t *color_buffer           = NULL;
SDL_Texture *color_buffer_texture = NULL;

int window_width  = 800;
int window_height = 600;

bool init_window(void) {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        fprintf(stderr, "Error initing SDL.\n");
        return false;
    }

    SDL_DisplayMode display_mode;
    SDL_GetCurrentDisplayMode(0, &display_mode);

    // larger pixels please - but keep aspect ratio
    window_width  = display_mode.w / 2;
    window_height = display_mode.h / 2;

    // create SDL window
    window = SDL_CreateWindow(NULL,                        //
                              SDL_WINDOWPOS_CENTERED,      //
                              SDL_WINDOWPOS_CENTERED,      //
                              window_width, window_height, //
                              SDL_WINDOW_BORDERLESS);
    if (!window) {
        fprintf(stderr, "Error creating SDL window.\n");
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, 0);
    if (!renderer) {
        fprintf(stderr, "Error creating SDL renderer.\n");
        return false;
    }

    SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);

    return true;
}

void clear_color_buffer(uint32_t color) {
    for (int i = 0; i < window_width * window_height; i++) {
        color_buffer[i] = color;
    }
}

void render_color_buffer(void) {
    SDL_UpdateTexture(color_buffer_texture, NULL, color_buffer, (int) (window_width * sizeof(uint32_t)));
    SDL_RenderCopy(renderer, color_buffer_texture, NULL, NULL);
}

void destroy_window(void) {
    free(color_buffer);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void draw_pixel(int x, int y, uint32_t color) {
    if (x >= 0 && x < window_width && y >= 0 && y < window_height) {
        color_buffer[(window_width * y) + x] = color;
    }
}

void swap(int *a, int *b) {
    int temp = *a;
    *a       = *b;
    *b       = temp;
}

void draw_line(int x0, int y0, int x1, int y1, uint32_t color) {
    // Bresenham - incremental error algorithm
    // determines which pixel is closest to the ideal line between two points, and
    // steps one pixel at a time in either the x or y direction
    bool steep = abs(x1 - x0) < abs(y1 - y0);
    if (steep) { // if line is steep, we transpose it
        swap(&x0, &y0);
        swap(&x1, &y1);
    }
    if (x0 > x1) { // make it left-to-right
        swap(&x0, &x1);
        swap(&y0, &y1);
    }

    int dx = x1 - x0;
    int dy = abs(y1 - y0);

    int err   = 0;
    int ystep = (y0 < y1) ? 1 : -1;
    int y     = y0;

    for (int x = x0; x <= x1; x++) {
        if (steep) { // if transposed, de-transpose
            draw_pixel(y, x, color);
        } else {
            draw_pixel(x, y, color);
        }

        err += 2 * dy;
        if (err > dx) {
            y += ystep;
            err -= 2 * dx;
        }
    }
}

void draw_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color) {
    draw_line(x0, y0, x1, y1, color);
    draw_line(x1, y1, x2, y2, color);
    draw_line(x2, y2, x0, y0, color);
}

void draw_grid(void) {
    for (int y = 0; y < window_height; y += 10) {
        for (int x = 0; x < window_width; x += 10) {
            color_buffer[(window_width * y) + x] = 0xFFFFFFFF;
        }
    }
}

void draw_rect(int x, int y, int width, int height, uint32_t color) {
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            int current_x = x + i;
            int current_y = y + j;
            draw_pixel(current_x, current_y, color);
        }
    }
}
