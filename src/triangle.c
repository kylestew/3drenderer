#include "triangle.h"

#include "display.h"

void int_swap(int *a, int *b) {
    int tmp = *a;
    *a      = *b;
    *b      = tmp;
}

// draw a filled triangle with a flat bottom
//      (x0, y0)
//        / \
//       /   \
//      /     \
//     /       \
//    /         \
//   /           \
// (x1,y1)-----(x2,y2)
void fill_flat_bottom_triangle(int x0, int y0, int x1, int y1, int x2, int y2, u_int32_t color) {
    // find the two slopes (two triangle legs)
    float inv_slope_start = (float) (x1 - x0) / (y1 - y0);
    float inv_slope_end   = (float) (x2 - x0) / (y2 - y0);

    // find start and end of first scanline
    float x_start = x0;
    float x_end   = x0;

    // draw scanlines: for every y from y0 to y1/y2, find x start and x end
    for (int y = y0; y <= y2; y++) {
        draw_line(x_start, y, x_end, y, color);

        // increment start and end of line based on slope
        x_start += inv_slope_start;
        x_end += inv_slope_end;
    }
}

void fill_flat_top_triangle(int x0, int y0, int x1, int y1, int x2, int y2, u_int32_t color) {
    // find the two slopes (two triangle legs)
    float inv_slope_start = (float) (x2 - x0) / (y2 - y0);
    float inv_slope_end   = (float) (x2 - x1) / (y2 - y1);

    // find start and end of first scanline
    float x_start = x2;
    float x_end   = x2;

    // draw scanlines: for every y from y0 to y1/y2, find x start and x end
    for (int y = y2; y >= y0; y--) {
        draw_line(x_start, y, x_end, y, color);

        // increment start and end of line based on slope
        x_start -= inv_slope_start;
        x_end -= inv_slope_end;
    }
}

void draw_filled_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color) {
    // sort vertices by y-coordinate ascending (y0 < y1 < y2)
    if (y0 > y1) {
        int_swap(&y0, &y1);
        int_swap(&x0, &x1);
    }
    if (y1 > y2) {
        int_swap(&y1, &y2);
        int_swap(&x1, &x2);
    }
    if (y0 > y1) {
        int_swap(&y0, &y1);
        int_swap(&x0, &x1);
    }

    // Calculate the new vertex (Mx, My) using triangle similarity
    int My = y1;
    int Mx = ((float) ((x2 - x0) * (y1 - y0)) / (float) (y2 - y0)) + x0;

    // Draw flat-bottom triangle
    fill_flat_bottom_triangle(x0, y0, x1, y1, Mx, My, color);

    // Draw flat-top triangle
    fill_flat_top_triangle(x1, y1, Mx, My, x2, y2, color);
}
