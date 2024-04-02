#include <glad/glad.h>
#include <vector>

enum CellType {
    AIR, FLUID, SOLID
};

struct Scene {
    GLfloat* particles_pos;
    GLfloat* particles_vel;
    CellType* cell_type;
    GLfloat* density;
    GLfloat* u;
    GLfloat* v;
    GLfloat* w;
    GLfloat* du;
    GLfloat* dv;
    GLfloat* dw;
    GLfloat* prevU;
    GLfloat* prevV;
    GLfloat* prevW;
    GLfloat* p;
    GLfloat* s;
    GLuint num_p;
    GLuint num_c_x;
    GLuint num_c_y;
    GLuint num_c_z;
    GLfloat p_rad; // radius of particle
    GLfloat p_mass; 
    GLfloat c_size; // volume of each individual grid
    GLfloat p_rest_density; // density of particles at initiation
    GLfloat p_density;

    GLfloat min_density, max_density;
    std::vector<GLfloat> *vertices;
    GLuint vert_size;

    Scene() = default;
    Scene(GLuint num_particles, GLuint num_cells_x, GLuint num_cells_y, GLuint num_cells_z, GLfloat particle_radius, GLfloat particle_mass, GLfloat cell_size)
    {
        int num_cells = num_cells_x * num_cells_y * num_cells_z;

        particles_pos = new GLfloat[num_particles * 3];
        particles_vel = new GLfloat[num_particles * 3]();
        cell_type = new CellType[num_cells];
        density = new GLfloat[num_cells]();
        u = new GLfloat[num_cells]();
        v = new GLfloat[num_cells]();
        w = new GLfloat[num_cells]();
        du = new GLfloat[num_cells]();
        dv = new GLfloat[num_cells]();
        dw = new GLfloat[num_cells]();
        prevU = new GLfloat[num_cells]();
        prevV = new GLfloat[num_cells]();
        prevW = new GLfloat[num_cells]();
        p = new GLfloat[num_cells]();
        s = new GLfloat[num_cells]();
        num_p = num_particles;
        num_c_x = num_cells_x;
        num_c_y = num_cells_y;
        num_c_z = num_cells_z;
        p_rad = particle_radius;
        p_mass = particle_mass;
        c_size = cell_size;
        p_density = 1000.f;
        p_rest_density = 0.f;
        min_density = 1000.f;
        max_density = 0.f;
        vertices = new std::vector<GLfloat>();
        vert_size = 0;
    }
};

