#include "array.h"
#include "display.h"
#include "mesh.h"
#include "vector.h"

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

triangle_t *triangles_to_render = NULL;

bool is_running         = false;
int previous_frame_time = 0;

vec3_t camera_position = {0, 0, -6};
float fov_factor       = 640.0;

void setup(void) {
    // Initialize render mode and triangle culling method
    render_method = RENDER_WIRE;
    cull_method   = CULL_BACKFACE;

    // allocate color buffer
    color_buffer = (uint32_t *) malloc(sizeof(uint32_t) * window_width * window_height);

    // texture to render (not sure why we have this and our own)
    color_buffer_texture = SDL_CreateTexture(renderer,                    //
                                             SDL_PIXELFORMAT_ARGB8888,    //
                                             SDL_TEXTUREACCESS_STREAMING, //
                                             window_width, window_height);

    load_cube_mesh_data();
    // load_obj_file_data("./assets/cube.obj");
    // load_obj_file_data("./assets/f22.obj");
}

void process_input(void) {
    SDL_Event event;
    SDL_PollEvent(&event);

    switch (event.type) {
    case SDL_QUIT:
        is_running = false;
        break;
    case SDL_KEYDOWN:
        if (event.key.keysym.sym == SDLK_ESCAPE || event.key.keysym.sym == SDLK_q)
            is_running = false;

        if (event.key.keysym.sym == SDLK_1)
            render_method = RENDER_WIRE_VERTEX;
        if (event.key.keysym.sym == SDLK_2)
            render_method = RENDER_WIRE;
        if (event.key.keysym.sym == SDLK_3)
            render_method = RENDER_FILL_TRIANGLE;
        if (event.key.keysym.sym == SDLK_4)
            render_method = RENDER_FILL_TRIANGLE_WIRE;
        if (event.key.keysym.sym == SDLK_c)
            cull_method = CULL_BACKFACE;
        if (event.key.keysym.sym == SDLK_d)
            cull_method = CULL_NONE;

        break;
    }
}

vec2_t project(vec3_t point, float fov_factor) {
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

    triangles_to_render = NULL;

    mesh.rotation.x += 0.01;
    mesh.rotation.y += 0.01;
    mesh.rotation.z += 0.01;

    // Loop all triangle faces of our mesh
    int num_faces = array_length(mesh.faces);
    for (int i = 0; i < num_faces; i++) {
        face_t mesh_face = mesh.faces[i];

        // load vertices (3) of triangle
        vec3_t face_vertices[3];
        face_vertices[0] = mesh.vertices[mesh_face.a - 1];
        face_vertices[1] = mesh.vertices[mesh_face.b - 1];
        face_vertices[2] = mesh.vertices[mesh_face.c - 1];

        // APPLY TRANSFORMATIONS
        vec3_t transformed_vertices[3]; // for backface culling
        for (int j = 0; j < 3; j++) {
            vec3_t transformed_vertex = face_vertices[j];

            // apply transformations
            transformed_vertex = vec3_rotate_x(transformed_vertex, mesh.rotation.x);
            transformed_vertex = vec3_rotate_y(transformed_vertex, mesh.rotation.y);
            transformed_vertex = vec3_rotate_z(transformed_vertex, mesh.rotation.z);

            // apply camera position
            transformed_vertex.z -= camera_position.z;

            // save transformed vertex in the array of transformed vertices
            transformed_vertices[j] = transformed_vertex;
        }

        // BACKFACE CULLING
        if (cull_method == CULL_BACKFACE) {
            vec3_t vector_a = transformed_vertices[0]; /*   A   */
            vec3_t vector_b = transformed_vertices[1]; /*  / \  */
            vec3_t vector_c = transformed_vertices[2]; /* C---B */

            // get the vector subtraction of B-A and C-A
            vec3_t vector_ab = vec3_sub(vector_b, vector_a);
            vec3_t vector_ac = vec3_sub(vector_c, vector_a);
            vec3_normalize(&vector_ab);
            vec3_normalize(&vector_ac);

            // computer the face normal (using cross product)
            // (right handed coordinate system)
            vec3_t normal = vec3_cross(vector_ab, vector_ac);

            // normalize the face normal vrector
            vec3_normalize(&normal);

            // find the vector between a point in the triangle and the camera origin
            vec3_t camera_ray = vec3_sub(camera_position, vector_a);

            // Calculate how aligned the camera ray is with the face normal (using dot product)
            float dot_normal_camera = vec3_dot(normal, camera_ray);

            // bypass the triangles that are looking away from the camera
            if (dot_normal_camera < 0) {
                continue; // move to next triangle
            }
        }

        // project points
        vec2_t projected_points[3];
        for (int j = 0; j < 3; j++) {
            // project the current vertex
            projected_points[j] = project(transformed_vertices[j], fov_factor);

            // offset point to middle of screen
            projected_points[j].x += (window_width / 2.0);
            projected_points[j].y += (window_height / 2.0);
        }

        // calculate the average depth for each face based on the vertices after transformation
        float avg_depth = (transformed_vertices[0].z + transformed_vertices[1].z + transformed_vertices[2].z) / 3.0;

        // PROJECT TO SCREENSPACE
        triangle_t projected_triangle = {.points =
                                             {
                                                 {projected_points[0].x, projected_points[0].y},
                                                 {projected_points[1].x, projected_points[1].y},
                                                 {projected_points[2].x, projected_points[2].y},
                                             },
                                         .color     = mesh_face.color,
                                         .avg_depth = avg_depth};

        array_push(triangles_to_render, projected_triangle);
    }
}

void render(void) {
    clear_color_buffer(0xFF000000);

    // loop all the projected triangles and render
    int num_triangles = array_length(triangles_to_render);
    for (int i = 0; i < num_triangles; i++) {
        triangle_t triangle = triangles_to_render[i];

        // draw filled triangle
        if (render_method == RENDER_FILL_TRIANGLE || render_method == RENDER_FILL_TRIANGLE_WIRE) {
            draw_filled_triangle(triangle.points[0].x, triangle.points[0].y, // A
                                 triangle.points[1].x, triangle.points[1].y, // B
                                 triangle.points[2].x, triangle.points[2].y, // C
                                 triangle.color);
        }

        // draw triangle wireframe
        if (render_method == RENDER_WIRE || render_method == RENDER_WIRE_VERTEX ||
            render_method == RENDER_FILL_TRIANGLE_WIRE) {
            draw_triangle(triangle.points[0].x, triangle.points[0].y, //
                          triangle.points[1].x, triangle.points[1].y, //
                          triangle.points[2].x, triangle.points[2].y, //
                          0xFFFFFFFF);
        }

        // draw triangle vertex points
        if (render_method == RENDER_WIRE_VERTEX) {
            draw_rect(triangle.points[0].x - 3, triangle.points[0].y - 3, 6, 6, 0xFFFF0000); // vertex A
            draw_rect(triangle.points[1].x - 3, triangle.points[1].y - 3, 6, 6, 0xFFFF0000); // vertex B
            draw_rect(triangle.points[2].x - 3, triangle.points[2].y - 3, 6, 6, 0xFFFF0000); // vertex C
        }
    }

    // clear the array of triangles to render every frame loop
    array_free(triangles_to_render);

    render_color_buffer();
    SDL_RenderPresent(renderer);
}

void free_resources(void) {
    // free(color_buffer);
    array_free(mesh.faces);
    array_free(mesh.vertices);
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
    free_resources();

    return 0;
}
