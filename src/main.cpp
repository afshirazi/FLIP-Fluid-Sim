#include <glad/glad.h>
#include <glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>

#include "util.h"
#include "scene.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);

void applyVel(GLfloat dt);
void handleSolidCellCollision();
void handleParticleParticleCollision();
void transferVelocities(bool, GLfloat);
void updateDensity();
void solveIncompressibility(int, GLfloat, GLfloat, bool);

Scene setupFluidScene();

Scene scene;

int main() {
	// Some code taken from learnopengl.com
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(800, 600, "FLIP Fluid Simulator", NULL, NULL);
	glfwMakeContextCurrent(window);

	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

	glViewport(0, 0, 800, 600);

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	GLfloat floor_vertices[] = {
	 -0.5f, 0.f,  0.5f,
	 -0.5f, 0.f, -0.5f,
	  0.5f, 0.f,  0.5f,
	  0.5f, 0.f, -0.5f
	};
	GLuint floor_indices[] = {
		0, 1, 2,
		1, 3, 2
	};

	// VAO for "floor"
	GLuint floor_VAO;
	glGenVertexArrays(1, &floor_VAO);
	glBindVertexArray(floor_VAO);

	GLuint floor_VBO;
	glGenBuffers(1, &floor_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, floor_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(floor_vertices), floor_vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(0);

	GLuint floor_EBO;
	glGenBuffers(1, &floor_EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, floor_EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(floor_indices), floor_indices, GL_STATIC_DRAW);

	scene = setupFluidScene();

	// VAO for particles
	GLuint particles_VAO;
	glGenVertexArrays(1, &particles_VAO);
	glBindVertexArray(particles_VAO);

	GLuint particles_VBO;
	glGenBuffers(1, &particles_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, particles_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * scene.num_p, scene.particles_pos, GL_STREAM_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(0);

	GLuint fShaderProg = createAndLinkFloorShaderProg();
	GLuint pShaderProg = createAndLinkParticleShaderProg();

	glm::mat4 proj = glm::mat4(1.0f);
	glm::mat4 model = glm::mat4(1.0f);
	glm::mat4 view = glm::mat4(1.0f);
	glm::mat4 floor_transform;
	glm::mat4 particles_transform;

	// Transformations for floor
	proj = glm::perspective(glm::radians(55.0f), 8.f / 6.f, 0.1f, 10.0f);
	model = glm::rotate(model, glm::radians(40.f), glm::vec3(1.f, 0.f, 0.f));
	model = glm::rotate(model, glm::radians(-120.0f), glm::vec3(0.f, 1.f, 0.f));
	model = glm::scale(model, glm::vec3(1.5, 1.5, 1.5));
	view = glm::translate(view, glm::vec3(0.f, 0.1f, -1.1f));
	floor_transform = proj * view * model;

	// Transformations for particles
	view = glm::mat4(1.0f);
	model = glm::scale(model, glm::vec3(0.67, 0.67, 0.67));
	view = glm::translate(view, glm::vec3(0.5f, 0.1f, -1.1f));
	particles_transform = proj * view * model;

	GLint fTransformLoc = glGetUniformLocation(fShaderProg, "transform");
	GLint fColorLoc = glGetUniformLocation(fShaderProg, "vertColor");
	GLint pTransformLoc = glGetUniformLocation(pShaderProg, "transform");
	GLint pColorLoc = glGetUniformLocation(pShaderProg, "vertColor");

	// rendering loop
	while (!glfwWindowShouldClose(window))
	{
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use particle shaders
		glLinkProgram(fShaderProg);
		glUseProgram(fShaderProg);

		// Draw floor
		glBindVertexArray(floor_VAO);
		glUniformMatrix4fv(fTransformLoc, 1, GL_FALSE, glm::value_ptr(floor_transform));
		glUniform3f(fColorLoc, 0.29f, 0.29f, 0.29f); // color red
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		// Use particle shaders
		glLinkProgram(pShaderProg);
		glUseProgram(pShaderProg);

		// Draw particles
		glBindVertexArray(particles_VAO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * scene.num_p, scene.particles_pos, GL_STREAM_DRAW); // Update particle positions in VBO
		glUniformMatrix4fv(pTransformLoc, 1, GL_FALSE, glm::value_ptr(particles_transform));
		glUniform3f(pColorLoc, 0.f, 0.f, 0.5f); // color blue
		glPointSize(5);
		glDrawArrays(GL_POINTS, 0, scene.num_p);

		// Apply forces/adjustments
		applyVel(1.f / 120.f);
		handleSolidCellCollision();
		handleParticleParticleCollision();
		transferVelocities(true, 0.0f);
		updateDensity();
		solveIncompressibility(400, 1.f / 120.f, 1.9f, true);
		transferVelocities(false, 0.9f);

		std::cout << scene.du[1] << " " << scene.particles_vel[0] << std::endl;

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void applyVel(GLfloat dt)
{
	for (GLuint i = 0; i < scene.num_p; i++)
	{
		scene.particles_pos[3 * i] += scene.particles_vel[3 * i] * dt;
		scene.particles_vel[3 * i + 1] -= 9.82f * dt;
		scene.particles_pos[3 * i + 1] += scene.particles_vel[3 * i + 1] * dt;
		scene.particles_pos[3 * i + 2] += scene.particles_vel[3 * i + 2] * dt;
	}
}

void handleSolidCellCollision()
{
	for (GLuint i = 0; i < scene.num_p; i++)
	{
		const GLfloat x_pos = scene.particles_pos[3 * i];
		const GLfloat y_pos = scene.particles_pos[3 * i + 1];
		const GLfloat z_pos = scene.particles_pos[3 * i + 2];


		/*
		* For cell walls checking
		* Minimum position of particles at any bound
		* cell size is c_size at most, and particle has to be beyond the first cell in every dimension,
		* so min position is cell size + particle radius (puts the particle in a fluid cell rather than a solid cell)
		* Max pos is similar, but y direction is unbounded
		*/
		GLfloat min_pos = scene.c_size + scene.p_rad;
		GLfloat max_pos_x = scene.c_size * (scene.num_c_x - 1) - scene.p_rad;
		GLfloat max_pos_y = scene.c_size * (scene.num_c_y - 1) - scene.p_rad;
		GLfloat max_pos_z = scene.c_size * (scene.num_c_z - 1) - scene.p_rad;

		if (x_pos < min_pos)
		{
			scene.particles_pos[3 * i] = min_pos;
			scene.particles_vel[3 * i] = 0.f;
		}

		if (y_pos < min_pos)
		{
			scene.particles_pos[3 * i + 1] = min_pos;
			scene.particles_vel[3 * i + 1] = 0.f;
		}

		if (z_pos < min_pos)
		{
			scene.particles_pos[3 * i + 2] = min_pos;
			scene.particles_vel[3 * i + 2] = 0.f;
		}

		if (x_pos > max_pos_x)
		{
			scene.particles_pos[3 * i] = max_pos_x;
			scene.particles_vel[3 * i] = 0.f;
		}

		if (y_pos > max_pos_y)
		{
			scene.particles_pos[3 * i + 1] = max_pos_y;
			scene.particles_vel[3 * i + 1] = 0.f;
		}

		if (z_pos > max_pos_z)
		{
			scene.particles_pos[3 * i + 2] = max_pos_z;
			scene.particles_vel[3 * i + 2] = 0.f;
		}
	}
}

void handleParticleParticleCollision()
{
	// hashing method taken from Matthias Muller
	int num_cells = scene.num_c_x * scene.num_c_y * scene.num_c_z;
	int* cell_p_count = new int[num_cells]();
	int* first_cell = new int[num_cells + 1]();
	int* sorted_particles = new int[scene.num_p];

	for (int i = 0; i < scene.num_p; i ++)
	{
		int x_int = std::floor(scene.particles_pos[3 * i] / scene.c_size);
		int y_int = std::floor(scene.particles_pos[3 * i + 1] / scene.c_size);
		int z_int = std::floor(scene.particles_pos[3 * i + 2] / scene.c_size);

		if (x_int < 0) x_int = 0;
		if (x_int > scene.num_c_x) x_int = scene.num_c_x - 1;
		if (y_int < 0) y_int = 0;
		if (y_int > scene.num_c_y) y_int = scene.num_c_y - 1;
		if (z_int < 0) z_int = 0;
		if (z_int > scene.num_c_z) z_int = scene.num_c_z - 1;

		cell_p_count[x_int * scene.num_c_y * scene.num_c_z + y_int * scene.num_c_z + z_int]++;
	}

	first_cell[0] = cell_p_count[0];
	for (int i = 1; i < num_cells; i++)
	{
		first_cell[i] = first_cell[i - 1];
		first_cell[i] += cell_p_count[i];
	}
	first_cell[num_cells] = first_cell[num_cells - 1];

	for (int i = 0; i < scene.num_p; i++)
	{
		int x_int = std::floor(scene.particles_pos[i * 3] / scene.c_size);
		int y_int = std::floor(scene.particles_pos[i * 3 + 1] / scene.c_size);
		int z_int = std::floor(scene.particles_pos[i * 3 + 2] / scene.c_size);

		if (x_int < 0) x_int = 0;
		if (x_int > scene.num_c_x - 1) x_int = scene.num_c_x - 1;
		if (y_int < 0) y_int = 0;
		if (y_int > scene.num_c_y - 1) y_int = scene.num_c_y - 1;
		if (z_int < 0) z_int = 0;
		if (z_int > scene.num_c_z - 1) z_int = scene.num_c_z - 1;

		int p_pos = --first_cell[x_int * scene.num_c_y * scene.num_c_z + y_int * scene.num_c_z + z_int];
		sorted_particles[p_pos] = i;
	}

	// TODO make more efficient neighborhood lookup
	// push particles apart
	GLfloat min_dist = 2 * scene.p_rad;
	GLfloat min_dist_sq = min_dist * min_dist;

	for (int i = 0; i < scene.num_p; i++)
	{
		GLfloat px = scene.particles_pos[i * 3];
		GLfloat py = scene.particles_pos[i * 3 + 1];
		GLfloat pz = scene.particles_pos[i * 3 + 2];
		int x_int = std::floor(px / scene.c_size);
		int y_int = std::floor(py / scene.c_size);
		int z_int = std::floor(pz / scene.c_size);

		int xs = std::max({ x_int - 1, 0 });
		int xe = std::min({ x_int + 1.f, scene.num_c_x - 1.f });
		int ys = std::max({ y_int - 1, 0 });
		int ye = std::min({ y_int + 1.f, scene.num_c_x - 1.f });
		int zs = std::max({ z_int - 1, 0 });
		int ze = std::min({ z_int + 1.f, scene.num_c_x - 1.f });

		// check all particles in the neighborhood around the current cell 
		for (int xi = xs; xi < xe; xi++)
			for (int yi = ys; yi < ye; yi++)
				for (int zi = zs; zi < ze; zi++)
				{
					int start_pos = first_cell[xi * scene.num_c_y * scene.num_c_z + yi * scene.num_c_z + zi];
					int end_pos = first_cell[xi * scene.num_c_y * scene.num_c_z + yi * scene.num_c_z + zi + 1];

					for (int j = start_pos; j < end_pos; j++)
					{
						int q = sorted_particles[j];

						if (q == i) continue;

						GLfloat qx = scene.particles_pos[q * 3];
						GLfloat qy = scene.particles_pos[q * 3 + 1];
						GLfloat qz = scene.particles_pos[q * 3 + 2];

						GLfloat pq_dist_sq = (px - qx) * (px - qx) + (py - qy) * (py - qy) + (pz - qz) * (pz - qz);
						if (pq_dist_sq > min_dist_sq) continue;

						GLfloat pq_dist = std::sqrt(pq_dist_sq);

						if (pq_dist == 0) continue; // avoid div by 0

						GLfloat push_factor = 0.5f * (min_dist - pq_dist) / pq_dist;

						GLfloat push_x = (px - qx) * push_factor;
						GLfloat push_y = (py - qy) * push_factor;
						GLfloat push_z = (pz - qz) * push_factor;

						scene.particles_pos[i * 3] += push_x;
						scene.particles_pos[q * 3] -= push_x;
						scene.particles_pos[i * 3 + 1] += push_y;
						scene.particles_pos[q * 3 + 1] -= push_y;
						scene.particles_pos[i * 3 + 2] += push_z;
						scene.particles_pos[q * 3 + 2] -= push_z;
					}
				}
	}
}

void updateDensity()
{
	GLfloat half_cs = 0.5 * scene.c_size;
	int cell_num = scene.num_c_x * scene.num_c_y * scene.num_c_z;
	std::fill(scene.density, scene.density + cell_num, 0.f);

	for (int i = 0; i < scene.num_p; i++)
	{
		GLfloat px = scene.particles_pos[3 * i];
		GLfloat py = scene.particles_pos[3 * i + 1];
		GLfloat pz = scene.particles_pos[3 * i + 2];

		if (px < scene.c_size) px = scene.c_size;
		if (px > scene.c_size * (scene.num_c_x - 1)) px = scene.c_size * (scene.num_c_x - 1);
		if (py < scene.c_size) py = scene.c_size;
		if (py > scene.c_size * (scene.num_c_y - 1)) py = scene.c_size * (scene.num_c_y - 1);
		if (pz < scene.c_size) pz = scene.c_size;
		if (pz > scene.c_size * (scene.num_c_z - 1)) pz = scene.c_size * (scene.num_c_z - 1);

		// Get corners of grid
		GLuint x0 = std::floor((px - half_cs) / scene.c_size);
		GLuint x1 = std::min(x0 + 1, scene.num_c_x - 2);
		GLuint y0 = std::floor((py - half_cs) / scene.c_size);
		GLuint y1 = std::min(y0 + 1, scene.num_c_y - 2);
		GLuint z0 = std::floor((pz - half_cs) / scene.c_size);
		GLuint z1 = std::min(z0 + 1, scene.num_c_z - 2);

		// Get dx, dy, and dz (local coordinates of particle in its cell)
		GLfloat local_x = (px - half_cs - x0 * scene.c_size) / scene.c_size;
		GLfloat local_y = (py - half_cs - y0 * scene.c_size) / scene.c_size;
		GLfloat local_z = (pz - half_cs - z0 * scene.c_size) / scene.c_size;

		// Add projection to corners of grid
		if (x0 < scene.num_c_x && y0 < scene.num_c_y && z0 < scene.num_c_z) 
			scene.density[x0 * scene.num_c_y * scene.num_c_z + y0 * scene.num_c_z + z0] += (1 - local_x) * (1 - local_y) * (1 - local_z) * scene.p_mass;
		if (x1 < scene.num_c_x && y0 < scene.num_c_y && z0 < scene.num_c_z)
			scene.density[x1 * scene.num_c_y * scene.num_c_z + y0 * scene.num_c_z + z0] += (local_x) * (1 - local_y) * (1 - local_z) * scene.p_mass;
		if (x0 < scene.num_c_x && y1 < scene.num_c_y && z0 < scene.num_c_z)
			scene.density[x0 * scene.num_c_y * scene.num_c_z + y1 * scene.num_c_z + z0] += (1 - local_x) * (local_y) * (1 - local_z) * scene.p_mass;
		if (x1 < scene.num_c_x && y1 < scene.num_c_y && z0 < scene.num_c_z)
			scene.density[x1 * scene.num_c_y * scene.num_c_z + y1 * scene.num_c_z + z0] += (local_x) * (local_y) * (1 - local_z) * scene.p_mass;
		if (x0 < scene.num_c_x && y0 < scene.num_c_y && z1 < scene.num_c_z)
			scene.density[x0 * scene.num_c_y * scene.num_c_z + y0 * scene.num_c_z + z1] += (1 - local_x) * (1 - local_y) * (local_z) * scene.p_mass;
		if (x1 < scene.num_c_x && y0 < scene.num_c_y && z1 < scene.num_c_z)
			scene.density[x1 * scene.num_c_y * scene.num_c_z + y0 * scene.num_c_z + z1] += (local_x) * (1 - local_y) * (local_z)*scene.p_mass;
		if (x0 < scene.num_c_x && y1 < scene.num_c_y && z1 < scene.num_c_z)
			scene.density[x0 * scene.num_c_y * scene.num_c_z + y1 * scene.num_c_z + z1] += (1 - local_x) * (local_y) * (local_z)*scene.p_mass;
		if (x1 < scene.num_c_x && y1 < scene.num_c_y && z1 < scene.num_c_z)
			scene.density[x1 * scene.num_c_y * scene.num_c_z + y1 * scene.num_c_z + z1] += (local_x) * (local_y) * (local_z)*scene.p_mass;
	}

	if (scene.p_rest_density == 0.f) // First time, set default density
	{
		GLfloat sum = 0.f;
		int fluid_cell_num = 0;
		int cell_num = scene.num_c_x * scene.num_c_y * scene.num_c_z;

		for (int i = 0; i < cell_num; i++)
		{
			if (scene.cell_type[i] == FLUID && scene.density[i] > 0.f)
			{
				sum += scene.density[i];
				fluid_cell_num++;
			}
		}

		if (fluid_cell_num > 0)
			scene.p_rest_density = sum / fluid_cell_num;
	}
}


void transferVelocities(bool toGrid, GLfloat flipRatio)
{
	GLfloat half_cs = 0.5 * scene.c_size;
	int cell_num = scene.num_c_x * scene.num_c_y * scene.num_c_z;

	if (toGrid) {
		std::copy(scene.u, scene.u + cell_num, scene.prevU);
		std::copy(scene.v, scene.v + cell_num, scene.prevV);
		std::copy(scene.w, scene.w + cell_num, scene.prevW);
		std::fill(scene.du, scene.du + cell_num, 0.0f);
		std::fill(scene.dv, scene.dv + cell_num, 0.0f);
		std::fill(scene.dw, scene.dw + cell_num, 0.0f);
		std::fill(scene.u, scene.u + cell_num, 0.0f);
		std::fill(scene.v, scene.v + cell_num, 0.0f);
		std::fill(scene.w, scene.w + cell_num, 0.0f);


		for (int i = 0; i < cell_num; i++)
			scene.cell_type[i] = scene.s[i] == 0.0 ? SOLID : AIR;

		for (int j = 0; j < scene.num_p; j++)
		{
			GLfloat x = scene.particles_pos[3 * j];
			GLfloat y = scene.particles_pos[3 * j + 1];
			GLfloat z = scene.particles_pos[3 * j + 2];
			int xi = glm::clamp(static_cast<int>(std::floor(x / scene.c_size)), 0, static_cast<int>(scene.num_c_x - 1));
			int yi = glm::clamp(static_cast<int>(std::floor(y / scene.c_size)), 0, static_cast<int>(scene.num_c_y - 1));
			int zi = glm::clamp(static_cast<int>(std::floor(z / scene.c_size)), 0, static_cast<int>(scene.num_c_z - 1));

			int cellNr = xi * scene.num_c_y  * scene.num_c_z + yi * scene.num_c_z + zi;
			if (scene.cell_type[cellNr] == AIR)
				scene.cell_type[cellNr] = FLUID;
		}
	}

	GLfloat* f;
	GLfloat* d;

	for (int component = 0; component < 3; component++)
	{
		GLfloat dx = component == 0 ? 0.0f : half_cs;
		GLfloat dy = component == 1 ? 0.0f : half_cs;
		GLfloat dz = component == 2 ? 0.0f : half_cs;

		f = (component == 0) ? scene.u : ((component == 1) ? scene.v : scene.w);
		GLfloat* prevF = (component == 0) ? scene.prevU : ((component == 1) ? scene.prevV : scene.prevW);
		d = (component == 0) ? scene.du : ((component == 1) ? scene.dv : scene.dw);

		for (int i = 0; i < scene.num_p; ++i) {
			GLfloat x = scene.particles_pos[3 * i];
			GLfloat y = scene.particles_pos[3 * i + 1];
			GLfloat z = scene.particles_pos[3 * i + 2];

			x = glm::clamp(x, scene.c_size, (scene.num_c_x - 1) * scene.c_size);
			y = glm::clamp(y, scene.c_size, (scene.num_c_y - 1) * scene.c_size);
			z = glm::clamp(z, scene.c_size, (scene.num_c_z - 1) * scene.c_size);

			int x0 = std::min(static_cast<int>(std::floor((x - dx) / scene.c_size)), static_cast<int>(scene.num_c_x - 2));
			GLfloat tx = ((x - dx) - x0 * scene.c_size) / scene.c_size;
			int x1 = std::min(x0 + 1, static_cast<int>(scene.num_c_x - 2));

			int y0 = std::min(static_cast<int>(std::floor((y - dy) / scene.c_size)), static_cast<int>(scene.num_c_y - 2));
			GLfloat ty = ((y - dy) - y0 * scene.c_size) / scene.c_size;
			int y1 = std::min(y0 + 1, static_cast<int>(scene.num_c_y - 2));

			int z0 = std::min(static_cast<int>(std::floor((z - dz) / scene.c_size)), static_cast<int>(scene.num_c_z - 2));
			GLfloat tz = ((z - dz) - z0 * scene.c_size) / scene.c_size;
			int z1 = std::min(z0 + 1, static_cast<int>(scene.num_c_z - 2));

			GLfloat sx = 1.0f - tx;
			GLfloat sy = 1.0f - ty;
			GLfloat sz = 1.0f - tz;

			GLfloat d0 = sx * sy * sz * scene.p_mass;
			GLfloat d1 = tx * sy * sz * scene.p_mass;
			GLfloat d2 = tx * ty * sz * scene.p_mass;
			GLfloat d3 = sx * ty * sz * scene.p_mass;
			GLfloat d4 = sx * sy * tz * scene.p_mass;
			GLfloat d5 = tx * sy * tz * scene.p_mass;
			GLfloat d6 = tx * ty * tz * scene.p_mass;
			GLfloat d7 = sx * ty * tz * scene.p_mass;

			int nr0 = x0 * scene.num_c_y * scene.num_c_z + y0 * scene.num_c_z + z0;
			int nr1 = x1 * scene.num_c_y * scene.num_c_z + y0 * scene.num_c_z + z0;
			int nr2 = x1 * scene.num_c_y * scene.num_c_z + y1 * scene.num_c_z + z0;
			int nr3 = x0 * scene.num_c_y * scene.num_c_z + y1 * scene.num_c_z + z0;
			int nr4 = x0 * scene.num_c_y * scene.num_c_z + y0 * scene.num_c_z + z1;
			int nr5 = x1 * scene.num_c_y * scene.num_c_z + y0 * scene.num_c_z + z1;
			int nr6 = x1 * scene.num_c_y * scene.num_c_z + y1 * scene.num_c_z + z1;
			int nr7 = x0 * scene.num_c_y * scene.num_c_z + y1 * scene.num_c_z + z1;

			if (toGrid) {
				GLfloat pv = scene.particles_vel[3 * i + component];
				f[nr0] += pv * d0;
				d[nr0] += d0;
				f[nr1] += pv * d1;
				d[nr1] += d1;
				f[nr2] += pv * d2;
				d[nr2] += d2;
				f[nr3] += pv * d3;
				d[nr3] += d3;
				f[nr4] += pv * d4;
				d[nr4] += d4;
				f[nr5] += pv * d5;
				d[nr5] += d5;
				f[nr6] += pv * d6;
				d[nr6] += d6;
				f[nr7] += pv * d7;
				d[nr7] += d7;
			}
			else {
				int offset = (component == 0) ? scene.num_c_y * scene.num_c_z : ((component == 1) ? scene.num_c_z : 1);
				float valid0 = (scene.cell_type[nr0] != AIR || scene.cell_type[nr0 - offset] != AIR) ? 1.0f : 0.0f;
				float valid1 = (scene.cell_type[nr1] != AIR || scene.cell_type[nr1 - offset] != AIR) ? 1.0f : 0.0f;
				float valid2 = (scene.cell_type[nr2] != AIR || scene.cell_type[nr2 - offset] != AIR) ? 1.0f : 0.0f;
				float valid3 = (scene.cell_type[nr3] != AIR || scene.cell_type[nr3 - offset] != AIR) ? 1.0f : 0.0f;
				float valid4 = (scene.cell_type[nr4] != AIR || scene.cell_type[nr4 - offset] != AIR) ? 1.0f : 0.0f;
				float valid5 = (scene.cell_type[nr5] != AIR || scene.cell_type[nr5 - offset] != AIR) ? 1.0f : 0.0f;
				float valid6 = (scene.cell_type[nr6] != AIR || scene.cell_type[nr6 - offset] != AIR) ? 1.0f : 0.0f;
				float valid7 = (scene.cell_type[nr7] != AIR || scene.cell_type[nr7 - offset] != AIR) ? 1.0f : 0.0f;


				float v = scene.particles_vel[3 * i + component];
				float d_val = valid0 * d0 + valid1 * d1 + valid2 * d2 + valid3 * d3 + valid4 * d4 + valid5 * d5 + valid6 * d6 + valid7 * d7;

				if (d_val > 0.0f) {
					float picV = (valid0 * d0 * f[nr0] + valid1 * d1 * f[nr1] + valid2 * d2 * f[nr2] + valid3 * d3 * f[nr3]
						+ valid4 * d4 * f[nr4] + valid5 * d5 * f[nr5] + valid6 * d6 * f[nr6] + valid7 * d7 * f[nr7]) / d_val;
					float corr = (valid0 * d0 * (f[nr0] - prevF[nr0]) + valid1 * d1 * (f[nr1] - prevF[nr1])
						+ valid2 * d2 * (f[nr2] - prevF[nr2]) + valid3 * d3 * (f[nr3] - prevF[nr3])
						+ valid4 * d4 * (f[nr4] - prevF[nr4]) + valid5 * d5 * (f[nr5] - prevF[nr5])
						+ valid6 * d6 * (f[nr6] - prevF[nr6]) + valid7 * d7 * (f[nr7] - prevF[nr7])) / d_val;
					float flipV = v + corr;

					scene.particles_vel[3 * i + component] = (1.0f - flipRatio) * picV + flipRatio * flipV;
				}
			}
		}

	}

	if (toGrid) {
		for (int i = 0; i < cell_num; ++i) {
			if (d[i] > 0.0f)
				f[i] /= d[i];
		}

		// Restore solid cells
		for (int i = 0; i < scene.num_c_x; ++i) {
			for (int j = 0; j < scene.num_c_y; ++j) {
				for (int k = 0; k < scene.num_c_z; ++k) {
					int idx = i * scene.num_c_y * scene.num_c_z + j * scene.num_c_z + k;
					bool solid = scene.cell_type[idx] == SOLID;
					if (solid || (i > 0 && scene.cell_type[(i - 1) * scene.num_c_y * scene.num_c_z + j * scene.num_c_z + k] == SOLID))
						scene.u[idx] = scene.prevU[idx];
					if (solid || (j > 0 && scene.cell_type[i * scene.num_c_y * scene.num_c_z + (j - 1) * scene.num_c_z + k] == SOLID))
						scene.v[idx] = scene.prevV[idx];
					if (solid || (k > 0 && scene.cell_type[i * scene.num_c_y * scene.num_c_z + j * scene.num_c_z + (k - 1)] == SOLID))
						scene.w[idx] = scene.prevW[idx];
				}
			}
		}
	}
}

void solveIncompressibility(int numIters, GLfloat dt, GLfloat overRelaxation, bool compensateDrift)
{
	int cell_num = scene.num_c_x * scene.num_c_y * scene.num_c_z;

	std::fill(scene.p, scene.p + cell_num, 0.0f);
	std::copy(scene.u, scene.u + cell_num, scene.prevU);
	std::copy(scene.v, scene.v + cell_num, scene.prevV);
	std::copy(scene.w, scene.w + cell_num, scene.prevW);

	int n = scene.num_c_y;
	float cp = scene.p_density * scene.c_size / dt;

	for (int iter = 0; iter < numIters; ++iter) {
		for (int i = 1; i < scene.num_c_x - 1; ++i) {
			for (int j = 1; j < scene.num_c_y - 1; ++j) {
				for (int k = 1; k < scene.num_c_z - 1; ++k) {
					int center = i * n * scene.num_c_z + j * scene.num_c_z + k;
					int left = (i - 1) * n * scene.num_c_z + j * scene.num_c_z + k;
					int right = (i + 1) * n * scene.num_c_z + j * scene.num_c_z + k;
					int bottom = i * n * scene.num_c_z + (j - 1) * scene.num_c_z + k;
					int top = i * n * scene.num_c_z + (j + 1) * scene.num_c_z + k;
					int front = i * n * scene.num_c_z + j * scene.num_c_z + (k - 1);
					int back = i * n * scene.num_c_z + j * scene.num_c_z + (k + 1);

					if (scene.cell_type[center] != FLUID)
						continue;

					float s = scene.s[center];
					float sx0 = scene.s[left];
					float sx1 = scene.s[right];
					float sy0 = scene.s[bottom];
					float sy1 = scene.s[top];
					float sz0 = scene.s[front];
					float sz1 = scene.s[back];
					s = sx0 + sx1 + sy0 + sy1 + sz0 + sz1;
					if (s == 0.0f)
						continue;

					float div = scene.u[right] - scene.u[center] + scene.v[top] - scene.v[center] + scene.w[back] - scene.w[center];

					if (scene.p_rest_density > 0.0f && compensateDrift) {
						float k = 1.f; // TODO change k
						float compression = scene.density[center] - scene.p_rest_density;
						if (compression > 0.0f)
							div = div - k * compression;
					}

					float p = -div / s * scene.p_mass;
					p *= overRelaxation;
					scene.p[center] += cp * p;

					scene.u[center] -= sx0 * p;
					scene.u[right] += sx1 * p;
					scene.v[center] -= sy0 * p;
					scene.v[top] += sy1 * p;
					scene.w[center] -= sz0 * p;
					scene.w[back] += sz1 * p;
				}
			}
		}
	}
}

Scene setupFluidScene()
{
	const GLuint num_p_x = 22;
	const GLuint num_p_y = 22;
	const GLuint num_p_z = 22;
	const GLuint num_c_x = 30;
	const GLuint num_c_y = 30;
	const GLuint num_c_z = 20;

	const GLuint num_particles = num_p_x * num_p_y * num_p_z;
	const GLuint num_cells = num_c_x * num_c_y * num_c_z;

	const GLfloat p_rad = 0.003f; // particle radius
	const GLfloat p_mass = 0.05f;
	const GLfloat cell_size = 0.4f / std::max({ num_c_x, num_c_y, num_c_z }); // finds largest dimension, and bounds it to coordinates [0, 0.6] (arbitrary choice)

	Scene scene(num_particles, num_c_x, num_c_y, num_c_z, p_rad, p_mass, cell_size);

	int particle = 0;
	for (int i = 0; i < num_p_x; i++)
		for (int j = 0; j < num_p_y; j++)
			for (int k = 0; k < num_p_z; k++)
			{
				scene.particles_pos[particle++] = 2 * cell_size + p_rad + 2 * i * p_rad + (j % 2 == 0 ? 0 : p_rad);
				scene.particles_pos[particle++] = 6 * cell_size + p_rad + 2 * j * p_rad;
				scene.particles_pos[particle++] = 2 * cell_size + p_rad + 2 * k * p_rad + (j % 2 == 0 ? 0 : p_rad);
			}

	for (int i = 0; i < num_c_x; i++)
		for (int j = 0; j < num_c_y; j++)
			for (int k = 0; k < num_c_z; k++)
			{
				CellType curr_c_type = AIR;
				if (i == 0 || i == num_c_x - 1 || j == 0 || k == 0 || k == num_c_z - 1)
					curr_c_type = SOLID;

				scene.cell_type[i * num_c_y * num_c_z + j * num_c_z + k] = curr_c_type;
				scene.s[i * num_c_y * num_c_z + j * num_c_z + k] = curr_c_type == SOLID ? 0.f : 1.f;
			}

	return scene;
}
