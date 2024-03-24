#include<glad/glad.h>

enum CellType {
    AIR, FLUID, SOLID
};

struct Scene {
    GLfloat* particles_pos;
    GLfloat* particles_vel;
    CellType* cell_type;
    GLfloat* density;
    GLuint num_p;
    GLuint num_c_x;
    GLuint num_c_y;
    GLuint num_c_z;
    GLfloat p_rad; // radius of particle
    GLfloat c_size; // volume of each individual grid
    GLfloat p_rest_density; // density of particles at initiation

    Scene() = default;
    Scene(GLuint num_particles, GLuint num_cells_x, GLuint num_cells_y, GLuint num_cells_z, GLfloat particle_radius, GLfloat cell_size)
    {
        particles_pos = new GLfloat[num_particles * 3];
        particles_vel = new GLfloat[num_particles * 3]();
        cell_type = new CellType[num_cells_x * num_cells_y * num_cells_z];
        density = new GLfloat[num_cells_x * num_cells_y * num_cells_z]();
        num_p = num_particles;
        num_c_x = num_cells_x;
        num_c_y = num_cells_y;
        num_c_z = num_cells_z;
        p_rad = particle_radius;
        c_size = cell_size;
        p_rest_density = 0.f;
    }
};

