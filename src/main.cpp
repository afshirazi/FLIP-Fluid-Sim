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
	model = glm::scale(model, glm::vec3(0.3, 0.3, 0.3));
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

		std::cout << scene.particles_pos[0] << " " << scene.particles_vel[0] << std::endl;

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
	}
}

void handleSolidCellCollision()
{
	for (GLuint i = 0; i < scene.num_p; i++)
	{
		const GLfloat x_pos = scene.particles_pos[3 * i];
		const GLfloat y_pos = scene.particles_pos[3 * i + 1];

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

		if (x_pos > max_pos_x)
		{
			scene.particles_pos[3 * i] = max_pos_x;
			scene.particles_vel[3 * i] = 0.f;
		}

		
	}
}

void handleParticleParticleCollision()
{
	// hashing method taken from Matthias Muller
	int num_cells = scene.num_c_x * scene.num_c_y;
	int* cell_p_count = new int[num_cells]();
	int* first_cell = new int[num_cells + 1]();
	int* sorted_particles = new int[scene.num_p];

	for (int i = 0; i < scene.num_p; i++)
	{
		int x_int = std::floor(scene.particles_pos[3 * i] / scene.c_size);
		int y_int = std::floor(scene.particles_pos[3 * i + 1] / scene.c_size);

		if (x_int < 0) x_int = 0;
		if (x_int > scene.num_c_x) x_int = scene.num_c_x - 1;
		if (y_int < 0) y_int = 0;
		if (y_int > scene.num_c_y) y_int = scene.num_c_y - 1;

		cell_p_count[x_int * scene.num_c_y + y_int]++;
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

		if (x_int < 0) x_int = 0;
		if (x_int > scene.num_c_x) x_int = scene.num_c_x;
		if (y_int < 0) y_int = 0;
		if (y_int > scene.num_c_y) y_int = scene.num_c_y;

		int p_pos = --first_cell[x_int * scene.num_c_y + y_int];
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
		int x_int = std::floor(px / scene.num_c_x);
		int y_int = std::floor(py / scene.num_c_y);

		int xs = std::max({ x_int - 1, 0 });
		int xe = std::min({ x_int + 1.f, scene.num_c_x - 1.f });
		int ys = std::max({ y_int - 1, 0 });
		int ye = std::min({ y_int + 1.f, scene.num_c_x - 1.f });

		// check all particles in the neighborhood around the current cell 
		for (int xi = xs; xi < xe; xi++)
			for (int yi = ys; yi < ye; yi++)
			{
				int start_pos = first_cell[xi * scene.num_c_y  + yi ];
				int end_pos = first_cell[xi * scene.num_c_y  + yi];

				for (int j = start_pos; j < end_pos; j++)
				{
					int q = sorted_particles[j];

					if (q == i) continue;

					GLfloat qx = scene.particles_pos[q * 3];
					GLfloat qy = scene.particles_pos[q * 3 + 1];

					GLfloat pq_dist_sq = (px - qx) * (px - qx) + (py - qy) * (py - qy) ;
					if (pq_dist_sq > min_dist_sq) continue;

					GLfloat pq_dist = std::sqrt(pq_dist_sq);

					if (pq_dist == 0) continue; // avoid div by 0

					GLfloat push_factor = 0.5f * (min_dist - pq_dist) / pq_dist;

					GLfloat push_x = (px - qx) * push_factor;
					GLfloat push_y = (py - qy) * push_factor;

					scene.particles_pos[i * 3] += push_x;
					scene.particles_pos[q * 3] -= push_x;
					scene.particles_pos[i * 3 + 1] += push_y;
					scene.particles_pos[q * 3 + 1] -= push_y;
				}
			}
	}
}

void updateDensity()
{
	GLfloat half_cs = 0.5 * scene.c_size;
	int cell_num = scene.num_c_x * scene.num_c_y;
	std::fill(scene.density, scene.density + cell_num, 0.f);

	for (int i = 0; i < scene.num_p; i++)
	{
		GLfloat px = scene.particles_pos[3 * i];
		GLfloat py = scene.particles_pos[3 * i + 1];

		if (px < scene.c_size) px = scene.c_size;
		if (px > scene.c_size * (scene.num_c_x - 1)) px = scene.c_size * (scene.num_c_x - 1);
		if (py < scene.c_size) py = scene.c_size;
		if (py > scene.c_size * (scene.num_c_y - 1)) py = scene.c_size * (scene.num_c_y - 1);

		// Get corners of grid
		GLuint x0 = std::floor((px - half_cs) / scene.c_size);
		GLuint x1 = std::min(x0 + 1, scene.num_c_x - 2);
		GLuint y0 = std::floor((py - half_cs) / scene.c_size);
		GLuint y1 = std::min(y0 + 1, scene.num_c_y - 2);

		// Get dx, dy, and dz (local coordinates of particle in its cell)
		GLfloat local_x = (px - half_cs - x0 * scene.c_size) / scene.c_size;
		GLfloat local_y = (py - half_cs - y0 * scene.c_size) / scene.c_size;

		// Add projection to corners of grid
		if (x0 < scene.num_c_x && y0 < scene.num_c_y)
			scene.density[x0 * scene.num_c_y + y0] += (1 - local_x) * (1 - local_y) * scene.p_mass;
		if (x1 < scene.num_c_x && y0 < scene.num_c_y)
			scene.density[x1 * scene.num_c_y + y0] += (local_x) * (1 - local_y) * scene.p_mass;
		if (x0 < scene.num_c_x && y1 < scene.num_c_y)
			scene.density[x0 * scene.num_c_y + y1] += (1 - local_x) * (local_y)*scene.p_mass;
		if (x1 < scene.num_c_x && y1 < scene.num_c_y)
			scene.density[x1 * scene.num_c_y + y1] += (local_x) * (local_y)*scene.p_mass;
	}

	if (scene.p_rest_density == 0.f) // First time, set default density
	{
		GLfloat sum = 0.f;
		int fluid_cell_num = 0;
		int cell_num = scene.num_c_x * scene.num_c_y ;

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
	int cell_num = scene.num_c_x * scene.num_c_y;

	if (toGrid) {
		std::copy(scene.u, scene.u + cell_num, scene.prevU);
		std::copy(scene.v, scene.v + cell_num, scene.prevV);
		std::fill(scene.du, scene.du + cell_num, 0.0f);
		std::fill(scene.dv, scene.dv + cell_num, 0.0f);
		std::fill(scene.u, scene.u + cell_num, 0.0f);
		std::fill(scene.v, scene.v + cell_num, 0.0f);


		for (int i = 0; i < cell_num; i++)
			scene.cell_type[i] = scene.s[i] == 0.0 ? SOLID : AIR;

		for (int j = 0; j < scene.num_p; j++)
		{
			GLfloat x = scene.particles_pos[3 * j];
			GLfloat y = scene.particles_pos[3 * j + 1];
			GLfloat z = scene.particles_pos[3 * j + 2];
			int xi = glm::clamp(static_cast<int>(std::floor(x / scene.c_size)), 0, static_cast<int>(scene.num_c_x - 1));
			int yi = glm::clamp(static_cast<int>(std::floor(y / scene.c_size)), 0, static_cast<int>(scene.num_c_y - 1));

			int cellNr = xi * scene.num_c_y  + yi ;
			if (scene.cell_type[cellNr] == AIR)
				scene.cell_type[cellNr] = FLUID;
		}
	}

	GLfloat* f;
	GLfloat* d;

	for (int component = 0; component < 2; component++)
	{
		GLfloat dx = component == 0 ? 0.0f : half_cs;
		GLfloat dy = component == 1 ? 0.0f : half_cs;

		f = (component == 0) ? scene.u : scene.v;
		GLfloat* prevF = (component == 0) ? scene.prevU :  scene.prevV;
		d = (component == 0) ? scene.du : scene.dv;
		GLfloat* weight = new GLfloat[scene.num_c_x * scene.num_c_y];

		for (int i = 0; i < scene.num_p; ++i) {
			GLfloat x = scene.particles_pos[3 * i];
			GLfloat y = scene.particles_pos[3 * i + 1];
			GLfloat z = scene.particles_pos[3 * i + 2];

			x = glm::clamp(x, scene.c_size, (scene.num_c_x - 1) * scene.c_size);
			y = glm::clamp(y, scene.c_size, (scene.num_c_y - 1) * scene.c_size);

			int x0 = std::min(static_cast<int>(std::floor((x - dx) / scene.c_size)), static_cast<int>(scene.num_c_x - 2));
			GLfloat tx = ((x - dx) - x0 * scene.c_size) / scene.c_size;
			int x1 = std::min(x0 + 1, static_cast<int>(scene.num_c_x - 2));

			int y0 = std::min(static_cast<int>(std::floor((y - dy) / scene.c_size)), static_cast<int>(scene.num_c_y - 2));
			GLfloat ty = ((y - dy) - y0 * scene.c_size) / scene.c_size;
			int y1 = std::min(y0 + 1, static_cast<int>(scene.num_c_y - 2));

			GLfloat sx = 1.0f - tx;
			GLfloat sy = 1.0f - ty;

			GLfloat d0 = sx * sy * scene.p_mass;
			GLfloat d1 = tx * sy * scene.p_mass;
			GLfloat d2 = tx * ty * scene.p_mass;
			GLfloat d3 = sx * ty * scene.p_mass;

			int nr0 = x0 * scene.num_c_y + y0;
			int nr1 = x1 * scene.num_c_y + y0;
			int nr2 = x1 * scene.num_c_y + y1;
			int nr3 = x0 * scene.num_c_y + y1;

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
			}
			else {
				int offset = (component == 0) ? scene.num_c_y : 1;
				float valid0 = (scene.cell_type[nr0] != AIR || scene.cell_type[nr0 - offset] != AIR) ? 1.0f : 0.0f;
				float valid1 = (scene.cell_type[nr1] != AIR || scene.cell_type[nr1 - offset] != AIR) ? 1.0f : 0.0f;
				float valid2 = (scene.cell_type[nr2] != AIR || scene.cell_type[nr2 - offset] != AIR) ? 1.0f : 0.0f;
				float valid3 = (scene.cell_type[nr3] != AIR || scene.cell_type[nr3 - offset] != AIR) ? 1.0f : 0.0f;


				float v = scene.particles_vel[3 * i + component];
				float d_val = valid0 * d0 + valid1 * d1 + valid2 * d2 + valid3 * d3;

				if (d_val > 0.0f) {
					float picV = (valid0 * d0 * f[nr0] + valid1 * d1 * f[nr1] + valid2 * d2 * f[nr2] + valid3 * d3 * f[nr3]) / d_val;
					float corr = (valid0 * d0 * (f[nr0] - prevF[nr0]) + valid1 * d1 * (f[nr1] - prevF[nr1])
						+ valid2 * d2 * (f[nr2] - prevF[nr2]) + valid3 * d3 * (f[nr3] - prevF[nr3])) / d_val;
					float flipV = v + corr;

					scene.particles_vel[3 * i + component] = (1.0f - flipRatio) * picV + flipRatio * flipV;
				}
			}
		}

		delete[] weight;
	}

	if (toGrid) {
		for (int i = 0; i < cell_num; ++i) {
			if (d[i] > 0.0f)
				f[i] /= d[i];
		}

		// Restore solid cells
		for (int i = 0; i < scene.num_c_x; ++i) {
			for (int j = 0; j < scene.num_c_y; ++j) {
				int idx = i * scene.num_c_y  + j ;
				bool solid = scene.cell_type[idx] == SOLID;
				if (solid || (i > 0 && scene.cell_type[(i - 1) * scene.num_c_y  + j]  == SOLID))
					scene.u[idx] = scene.prevU[idx];
				if (solid || (j > 0 && scene.cell_type[i * scene.num_c_y  + (j - 1)] == SOLID))
					scene.v[idx] = scene.prevV[idx];

			}
		}
	}
}

void solveIncompressibility(int numIters, GLfloat dt, GLfloat overRelaxation, bool compensateDrift)
{
	int cell_num = scene.num_c_x * scene.num_c_y;

	std::fill(scene.p, scene.p + cell_num, 0.0f);
	std::copy(scene.u, scene.u + cell_num, scene.prevU);
	std::copy(scene.v, scene.v + cell_num, scene.prevV);

	int n = scene.num_c_y;
	float cp = scene.p_density * scene.c_size / dt;

	for (int iter = 0; iter < numIters; ++iter) {
		for (int i = 1; i < scene.num_c_x - 1; ++i) {
			for (int j = 1; j < scene.num_c_y - 1; ++j) {
				int center = i * n * j;
				int left = (i - 1) * n + j;
				int right = (i + 1) * n + j;
				int bottom = i * n + (j - 1);
				int top = i * n + (j + 1);

				if (scene.cell_type[center] != FLUID)
					continue;

				float s = scene.s[center];
				float sx0 = scene.s[left];
				float sx1 = scene.s[right];
				float sy0 = scene.s[bottom];
				float sy1 = scene.s[top];
				s = sx0 + sx1 + sy0 + sy1;
				if (s == 0.0f)
					continue;

				float div = scene.u[right] - scene.u[center] + scene.v[top] - scene.v[center];

				if (scene.p_rest_density > 0.0f && compensateDrift) {
					float k = 1.f; // TODO change k
					float compression = scene.density[center] - scene.p_rest_density;
					if (compression != 0.0f)
						div = div - k * compression;
				}

				float p = -div / s;
				p *= overRelaxation;
				scene.p[center] += cp * p;

				scene.u[center] -= sx0 * p;
				scene.u[right] += sx1 * p;
				scene.v[center] -= sy0 * p;
				scene.v[top] += sy1 * p;

			}
		}
	}
}

Scene setupFluidScene()
{
	const GLuint num_p_x = 16;
	const GLuint num_p_y = 16;
	const GLuint num_c_x = 13;
	const GLuint num_c_y = 13;

	const GLuint num_particles = num_p_x * num_p_y;
	const GLuint num_cells = num_c_x * num_c_y;

	const GLfloat p_rad = 0.05f; // particle radius
	const GLfloat p_mass = 1.f;
	const GLfloat cell_size = 1.f / std::max({ num_c_x, num_c_y }); // finds largest dimension, and bounds it to coordinates [0, 0.6] (arbitrary choice)

	Scene scene(num_particles, num_c_x, num_c_y, p_rad, p_mass, cell_size);

	int particle = 0;
	for (int i = 0; i < num_p_x; i++)
		for (int j = 0; j < num_p_y; j++)
		{
			scene.particles_pos[particle++] = 2 * cell_size + p_rad + 2 * i * p_rad + (j % 2 == 0 ? 0 : p_rad);
			scene.particles_pos[particle++] = 2 * cell_size + p_rad + 2 * j * p_rad;
			scene.particles_pos[particle++] = 0.f;
		}

	for (int i = 0; i < num_c_x; i++)
		for (int j = 0; j < num_c_y; j++)
		{
			CellType curr_c_type = AIR;
			if (i == 0 || i == num_c_x - 1 || j == 0)
				curr_c_type = SOLID;

			scene.cell_type[i * num_c_y + j] = curr_c_type;
			scene.s[i * num_c_y + j] = curr_c_type == SOLID ? 0.f : 1.f;
		}

	return scene;
}
