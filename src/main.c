#include "display.h"
#include "mesh.h"
#include "vector.h"

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

triangle_t triangles_to_render[N_MESH_FACES];

vec3_t camera_position = {0, 0, -4};
vec3_t cube_rotation   = {.x = 0, .y = 0, .z = 0};

bool is_running         = false;
int previous_frame_time = 0;

void setup(void) {
    // allocate color buffer
    color_buffer = (uint32_t *) malloc(sizeof(uint32_t) * window_width * window_height);

    // texture to render (not sure why we have this and our own)
    color_buffer_texture = SDL_CreateTexture(renderer,                    //
                                             SDL_PIXELFORMAT_ARGB8888,    //
                                             SDL_TEXTUREACCESS_STREAMING, //
                                             window_width, window_height);
}

void process_input(void) {
    SDL_Event event;
    SDL_PollEvent(&event);

    switch (event.type) {
    case SDL_QUIT:
        is_running = false;
        break;
    case SDL_KEYDOWN:
        if (event.key.keysym.sym == SDLK_ESCAPE)
            is_running = false;
        break;
    }
}

vec2_t ortho_project(vec3_t point, float fov_factor) {
    vec2_t projected_point = {
        .x = (fov_factor * point.x) / point.z,
        .y = (fov_factor * point.y) / point.z,
    };
    return projected_point;
}

void update(void) {
    // only delay execution if we are running too fast
    int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - previous_frame_time);
    if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME) {
        SDL_Delay(time_to_wait);
    }
    previous_frame_time = SDL_GetTicks();

    cube_rotation.x += 0.01;
    cube_rotation.y += 0.01;
    cube_rotation.z += 0.01;

    // for each face
    for (int i = 0; i < N_MESH_FACES; i++) {
        face_t mesh_face = mesh_faces[i];

        vec3_t face_vertices[3];
        face_vertices[0] = mesh_vertices[mesh_face.a - 1];
        face_vertices[1] = mesh_vertices[mesh_face.b - 1];
        face_vertices[2] = mesh_vertices[mesh_face.c - 1];

        // loop all 3 vertices of current face
        triangle_t projected_triangle;
        for (int j = 0; j < 3; j++) {
            vec3_t transformed_vertex = face_vertices[j];

            // apply transformations
            transformed_vertex = vec3_rotate_x(transformed_vertex, cube_rotation.x);
            transformed_vertex = vec3_rotate_y(transformed_vertex, cube_rotation.y);
            transformed_vertex = vec3_rotate_z(transformed_vertex, cube_rotation.z);

            // apply camera position
            transformed_vertex.z -= camera_position.z;

            // project!
            vec2_t projected_vertex = ortho_project(transformed_vertex, 640.0);

            // offset point to middle of screen
            projected_vertex.x += (window_width / 2);
            projected_vertex.y += (window_height / 2);

            projected_triangle.points[j] = projected_vertex;
        }

        // save in the triangles to render array
        triangles_to_render[i] = projected_triangle;
    }
}

void render(void) {
    clear_color_buffer(0xFF000000);

    // loop all the projected triangles and render
    for (int i = 0; i < N_MESH_FACES; i++) {
        triangle_t triangle = triangles_to_render[i];
        for (int j = 0; j < 3; j++) {
            vec2_t vertex = triangle.points[j];
            draw_rect(vertex.x, //
                      vertex.y, //
                      4, 4,     //
                      0xFFFF00FF);
        }
    }

    render_color_buffer();
    SDL_RenderPresent(renderer);
}

int main(void) {
    is_running = init_window();

    setup();

    while (is_running) {
        process_input();
        update();
        render();
    }

    destroy_window();

    return 0;
}
