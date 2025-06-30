#ifndef MESH_H
#define MESH_H

#include "triangle.h"
#include "vector.h"

typedef struct {
    vec3_t *vertices; // dynamic array
    face_t *faces;    // dynamic array
    vec3_t rotation;  // {x, y, z} angles
} mesh_t;

extern mesh_t mesh;

void load_cube_mesh_data(void);
void load_obj_file_data(char* filename);

#define N_CUBE_VERTICES 8
#define N_CUBE_FACES (6 * 2) // 6 cube faces, 2 triangles per face

extern vec3_t cube_vertices[N_CUBE_VERTICES];
extern face_t cube_faces[N_CUBE_FACES];

#endif
