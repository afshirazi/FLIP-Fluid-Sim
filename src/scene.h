#include<glad/glad.h>

enum CellType {
    AIR, FLUID, SOLID
};

struct Scene {
    GLfloat *particles_pos;
    GLfloat *particles_vel;
    CellType* cell_type;
    GLuint num_p;

    Scene() = default;
    Scene(GLuint num_particles, GLuint num_cells)
    {
        particles_pos = new GLfloat[num_particles * 3];
        particles_vel = new GLfloat[num_particles * 3]();
        cell_type = new CellType[num_cells];
        num_p = num_particles;
    }
};

