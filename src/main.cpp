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

Scene setupFluidScene();

Scene scene;

/* REMOVE LATER

GLfloat particles_pos[] = {
		-0.5f, 0.2f,  0.5f,
		-0.25f, 0.2f,  0.5f,
		-0.2f, 0.2f,  0.5f,
		-0.15f, 0.2f,  0.5f,
		-0.f, 0.2f,  0.5f,
		-0.5f, 0.2f,  -0.1f,
		-0.25f, 0.2f,  -0.1f,
		-0.2f, 0.2f,  -0.1f,
		-0.15f, 0.2f,  -0.1f,
		-0.f, 0.2f,  -0.1f,
		-0.5f, 0.2f,  -0.4f,
		-0.25f, 0.2f,  -0.4f,
		-0.2f, 0.2f,  -0.4f,
		-0.15f, 0.2f,  -0.4f,
		-0.f, 0.2f,  -0.4f,
};

GLfloat particles_vel[] = {
		0.f, 0.f,  0.f,
		0.f, 0.f,  0.f,
		0.f, 0.f,  0.f,
		0.f, 0.f,  0.f,
		0.f, 0.f,  0.f,
		0.f, 0.f,  0.f,
		0.f, 0.f,  0.f,
		0.f, 0.f,  0.f,
		0.f, 0.f,  0.f,
		0.f, 0.f,  0.f,
		0.f, 0.f,  0.f,
		0.f, 0.f,  0.f,
		0.f, 0.f,  0.f,
		0.f, 0.f,  0.f,
		0.f, 0.f,  0.f,
};
*/


int main() {
	// Some code taken from learnopengl.com
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);
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
		applyVel(1.f / 60.f);
		handleSolidCellCollision();
		handleParticleParticleCollision();

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
	for (GLuint i = 0; i < scene.num_p * 3; i += 3)
	{
		scene.particles_pos[i] += scene.particles_vel[i] * dt;
		scene.particles_vel[i + 1] -= 9.82f * dt;
		scene.particles_pos[i + 1] += scene.particles_vel[i + 1] * dt;
		scene.particles_pos[i + 2] += scene.particles_vel[i + 2] * dt;
	}
}

void handleSolidCellCollision()
{
	for (GLuint i = 0; i < scene.num_p * 3; i += 3)
	{
		const GLfloat x_pos = scene.particles_pos[i];
		const GLfloat y_pos = scene.particles_pos[i + 1];
		const GLfloat z_pos = scene.particles_pos[i + 2];


		/*
		* For cell walls checking
		* Minimum position of particles at any bound
		* cell size is c_size at most, and particle has to be beyond the first cell in every dimension,
		* so min position is cell size + particle radius (puts the particle in a fluid cell rather than a solid cell)
		* Max pos is similar, but y direction is unbounded
		*/
		GLfloat min_pos = scene.c_size + scene.p_rad;
		GLfloat max_pos_x = scene.c_size * (scene.num_c_x - 1) - scene.p_rad;
		GLfloat max_pos_z = scene.c_size * (scene.num_c_z - 1) - scene.p_rad;

		if (x_pos < min_pos)
		{
			scene.particles_pos[i] = min_pos;
			scene.particles_vel[i] = 0.f;
		}

		if (y_pos < min_pos)
		{
			scene.particles_pos[i + 1] = min_pos;
			scene.particles_vel[i + 1] = 0.f;
		}

		if (z_pos < min_pos)
		{
			scene.particles_pos[i + 2] = min_pos;
			scene.particles_vel[i + 2] = 0.f;
		}

		if (x_pos > max_pos_x)
		{
			scene.particles_pos[i] = max_pos_x;
			scene.particles_vel[i] = 0.f;
		}

		if (z_pos > max_pos_z)
		{
			scene.particles_pos[i + 2] = max_pos_z;
			scene.particles_vel[i + 2] = 0.f;
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

	for (int i = 0; i < scene.num_p * 3; i += 3)
	{
		int x_int = std::floor(scene.particles_pos[i] / scene.num_c_x);
		int y_int = std::floor(scene.particles_pos[i + 1] / scene.num_c_y);
		int z_int = std::floor(scene.particles_pos[i + 2] / scene.num_c_z);

		if (x_int < 0) x_int = 0;
		if (x_int > scene.num_c_x) x_int = scene.num_c_x;
		if (y_int < 0) y_int = 0;
		if (y_int > scene.num_c_y) y_int = scene.num_c_y;
		if (z_int < 0) z_int = 0;
		if (z_int > scene.num_c_z) z_int = scene.num_c_z;

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
		int x_int = std::floor(scene.particles_pos[i * 3] / scene.num_c_x);
		int y_int = std::floor(scene.particles_pos[i * 3 + 1] / scene.num_c_y);
		int z_int = std::floor(scene.particles_pos[i * 3 + 2] / scene.num_c_z);

		if (x_int < 0) x_int = 0;
		if (x_int > scene.num_c_x) x_int = scene.num_c_x;
		if (y_int < 0) y_int = 0;
		if (y_int > scene.num_c_y) y_int = scene.num_c_y;
		if (z_int < 0) z_int = 0;
		if (z_int > scene.num_c_z) z_int = scene.num_c_z;

		int p_pos = --first_cell[x_int * scene.num_c_y * scene.num_c_z + y_int * scene.num_c_z + z_int];
		sorted_particles[p_pos] = i;
	}

	// push particles apart
	GLfloat min_dist = 2 * scene.p_rad;
	GLfloat min_dist_sq = min_dist * min_dist;

	for (int i = 0; i < scene.num_p; i++)
	{
		GLfloat px = scene.particles_pos[i * 3];
		GLfloat py = scene.particles_pos[i * 3 + 1];
		GLfloat pz = scene.particles_pos[i * 3 + 2];
		int x_int = std::floor(px / scene.num_c_x);
		int y_int = std::floor(py / scene.num_c_y);
		int z_int = std::floor(pz / scene.num_c_z);

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

Scene setupFluidScene()
{
	const GLuint num_p_x = 16;
	const GLuint num_p_y = 16;
	const GLuint num_p_z = 16;
	const GLuint num_c_x = 20;
	const GLuint num_c_y = 20;
	const GLuint num_c_z = 20;

	const GLuint num_particles = num_p_x * num_p_y * num_p_z;
	const GLuint num_cells = num_c_x * num_c_y * num_c_z;

	const GLfloat p_rad = 0.0045f; // particle radius
	const GLfloat cell_size = 0.6 / std::max({ num_c_x, num_c_y, num_c_z }); // finds largest dimension, and bounds it to coordinates [0, 0.6] (arbitrary choice)

	Scene scene(num_particles, num_c_x, num_c_y, num_c_z, p_rad, cell_size);

	int particle = 0;
	for (int i = 0; i < num_p_x; i++)
		for (int j = 0; j < num_p_y; j++)
			for (int k = 0; k < num_p_z; k++)
			{
				scene.particles_pos[particle++] = 0.1f + p_rad + 2 * i * p_rad + (j % 2 == 0 ? 0 : p_rad);
				scene.particles_pos[particle++] = 0.1f + p_rad + 2 * j * p_rad;
				scene.particles_pos[particle++] = 0.1f + p_rad + 2 * k * p_rad + (j % 2 == 0 ? 0 : p_rad);
			}

	for (int i = 0; i < num_c_x; i++)
		for (int j = 0; j < num_c_y; j++)
			for (int k = 0; k < num_c_z; k++)
			{
				CellType curr_c_type = FLUID;
				if (i == 0 || i == num_c_x - 1 || j == 0 || k == 0 || k == num_c_z - 1)
					curr_c_type = SOLID;

				scene.cell_type[i * num_c_y * num_c_z + j * num_c_z + k] = curr_c_type;
			}

	return scene;
}
