#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <SOIL.h>
#include <vector>
#include <stack>

#include <iostream>

glm::vec3 rotationVector;
GLfloat rotateDirection;
std::vector<GLfloat> vertices;

std::stack<glm::mat4> mvStack;
std::stack<glm::mat4> viewStack;

glm::vec3 camera = { 0, 0, -6 };
glm::vec3 lookAt = { 0, 1, 0 };
glm::vec3 up = { 0, 1, 0 };

GLfloat delta;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void divideTriangle(glm::vec3 a, glm::vec3 b, glm::vec3 c, int count);
void triangle(glm::vec3 a, glm::vec3 b, glm::vec3 c);
void tetrahedron(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d, int count);
void divideTriangleWithoutNormalizing(glm::vec3 a, glm::vec3 b, glm::vec3 c, int count);
void floor(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d, int count);
void moveForward();
void moveBackward();
void moveLeft();
void moveRight();
glm::vec3 bisector(glm::vec3 a, glm::vec3 b);


const char *vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"out vec4 assignedColor;\n"
"uniform mat4 mvp;\n"
"void main()\n"
"{\n"
"   gl_Position = mvp * vec4(aPos, 1.0);\n"
"	assignedColor = vec4(sin(aPos.x), cos(aPos.z) + 0.5, cos(aPos.x), 1.0);\n"
"}\0";

const char *floorVertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"uniform mat4 mvp;\n "
"varying vec2 texCoord;\n"
"void main(void)\n"
"{\n"
"gl_Position = mvp * vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
"texCoord = vec2( gl_Position.x , gl_Position.z ) ;\n"
"}\0";

const char *floorFragmentShaderSource = "#version 330 core \n #extension GL_EXT_gpu_shader4 : enable\n"
"out vec4 fragColor;\n"
"uniform sampler2D Texture0;\n"
"varying vec2 texCoord;\n"
"void main()\n"
"{\n"
"	ivec2 size = textureSize2D(Texture0, 0);\n"
"	bool isEvenA = (mod(floor((texCoord.x)), 2.0) == 0.0);\n"
"	bool isEvenB = (mod(floor((texCoord.y)), 2.0) == 0.0);\n"
"	bool isEven = isEvenA == isEvenB;"
"	vec4 black = vec4(0.0, 0.0, 0.7, 1.0);\n"
"	vec4 white = vec4(0.6, 0.3, 0.3, 1.0);\n"
"   fragColor = (isEven)? black : white;\n"
"}\n\0";

const char *fragmentShaderSource = "#version 330 core\n"
"out vec4 fragColor;\n"
"in vec4 assignedColor;\n"
"void main()\n"
"{\n"
"   fragColor = assignedColor;\n"
"}\n\0";

int main()
{
	delta = 0.1;
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);
	glfwMakeContextCurrent(window);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	GLint GlewInitResult = glewInit();
	if (GLEW_OK != GlewInitResult)
	{
		std::cout << "ERROR: %s\n" << glewGetErrorString(GlewInitResult) << std::endl;
		exit(EXIT_FAILURE);
	}
	// build and compile our shader program
	// ------------------------------------
	// vertex shader

	int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	// check for shader compile errors
	int success;
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	// fragment shader
	int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	// check for shader compile errors
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	// link shaders
	int shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	// check for linking errors
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}
	//glDeleteShader(vertexShader);
	//glDeleteShader(fragmentShader);

	int floorVertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(floorVertexShader, 1, &floorVertexShaderSource, NULL);
	glCompileShader(floorVertexShader);
	glGetShaderiv(floorVertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(floorVertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FLOOR_VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	int floorFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(floorFragmentShader, 1, &floorFragmentShaderSource, NULL);
	glCompileShader(floorFragmentShader);
	glGetShaderiv(floorFragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(floorFragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FLOOR_FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	int floorShaderProgram = glCreateProgram();
	glAttachShader(floorShaderProgram, floorVertexShader);
	glAttachShader(floorShaderProgram, floorFragmentShader);
	glLinkProgram(floorShaderProgram);

	//glDeleteShader(floorVertexShader);
	//glDeleteShader(floorFragmentShader);
	// set up vertex data (and buffer(s)) and configure vertex attributes
	// ------------------------------------------------------------------
	glm::vec3 a = { 0.0f , 0.0f, -1.0f };
	glm::vec3 b = { 0.0f , 0.94289f, 0.33333f };
	glm::vec3 c = { -0.816497f, -0.471405, 0.33333f };
	glm::vec3 d = { 0.816497f, -0.471405, 0.33333f };
	tetrahedron(a, b, c, d, 5);
	GLuint sphereVertexSize = vertices.size() / 3;
	GLuint sphereVertexIndex = 0;

	glm::vec3 floorVertexA = { -10.0 * 100.0, -1.0 , 10.0 * 100.0 }; // front left }
	glm::vec3 floorVertexB = { 10.0 * 100.0, -1.0, 10.0 * 100.0 }; // front right
	glm::vec3 floorVertexC = { -10.0 * 100.0, -1.0, -10.0 * 100.0 }; // back left
	glm::vec3 floorVertexD = { 10.0 * 100.0, -1.0, -10.0 * 100.0 }; // back right
	floor(floorVertexA, floorVertexB, floorVertexC, floorVertexD, 0);
	GLuint floorVertexSize = (vertices.size() - sphereVertexSize) / 3;
	GLuint floorVertexIndex = sphereVertexIndex + sphereVertexSize;
	
	GLuint VBO, VAO;
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), (const void *)&vertices[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);

	// bind the VAO (it was already bound, but just to demonstrate): seeing as we only have a single VAO we can 
	// just bind it beforehand before rendering the respective triangle; this is another approach.
	glBindVertexArray(VAO);

	mvStack.push(glm::mat4(1.0f));

	viewStack.push(glm::lookAt(
		camera,
		lookAt,
		up
	));
	while (!glfwWindowShouldClose(window))
	{
		// input
		// -----
		processInput(window);

		// render
		// ------
		glClearColor(1.0f, 0.3f, 0.7f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// be sure to activate the shader before any calls to glUniform
		// set the transform matrix

		// model matrix
		glm::mat4 model = glm::mat4(1.0f);
		//model = glm::rotate(model, , rotationVector);

		// view



		float angle = (GLfloat)glfwGetTime() * 360.f / 310.0f;
		glm::mat4 rotate = glm::rotate(glm::mat4(1.0f), angle / 2.0f, glm::vec3(0, 1, 0));

		// projection
		glm::mat4 projection = glm::perspective(45.0f, 4.0f /2.0f, 0.1f, 1000.0f);

		glm::mat4 mvp = projection * viewStack.top() * mvStack.top();



		glUseProgram(floorShaderProgram);
		GLuint mvpLocation = glGetUniformLocation(floorShaderProgram, "mvp");
		glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, glm::value_ptr(mvp));
		glDrawArrays(GL_TRIANGLES, floorVertexIndex, floorVertexSize);

		mvp = mvp * rotate;
		glUseProgram(shaderProgram);
		mvpLocation = glGetUniformLocation(shaderProgram, "mvp");
		glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, glm::value_ptr(mvp));
		glLineWidth(4.0f);
		glDrawArrays(GL_LINES, sphereVertexIndex, sphereVertexSize);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
	{
		moveForward();
	}
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
	{
		moveBackward();
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
	{
		moveLeft();
	}
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
	{
		moveRight();
	}
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

void divideTriangle(glm::vec3 a, glm::vec3 b, glm::vec3 c, int count) 
{
	if (count > 0)
	{
		glm::vec3 ab = glm::normalize(bisector(a, b));
		glm::vec3 ac = glm::normalize(bisector(a, c));
		glm::vec3 bc = glm::normalize(bisector(b, c));
		
		divideTriangle(a, ab, ac, count - 1);
		divideTriangle(ab, b, bc, count - 1);
		divideTriangle(bc, c, ac, count - 1);
		divideTriangle(ab, bc, ac, count - 1);
	}
	else {
		triangle(a, b, c);
	}

	
}

void triangle(glm::vec3 a, glm::vec3 b, glm::vec3 c)
{

	vertices.push_back(a[0]);
	vertices.push_back(a[1]);
	vertices.push_back(a[2]);
	vertices.push_back(b[0]);
	vertices.push_back(b[1]);
	vertices.push_back(b[2]);
	vertices.push_back(c[0]);
	vertices.push_back(c[1]);
	vertices.push_back(c[2]);
}

glm::vec3 bisector(glm::vec3 a, glm::vec3 b)
{
	glm::vec3 result = glm::normalize(a) + glm::normalize(b);
	return result / 2.0f;
}

void tetrahedron(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d, int count)
{
	divideTriangle(b, c, d, count);
	divideTriangle(d, a, c, count);
	divideTriangle(c, b, a, count);
	divideTriangle(a, d, b, count);
}

void floor(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d, int count)
{
	divideTriangleWithoutNormalizing(a, b, c, count);
	divideTriangleWithoutNormalizing(b, d, c, count);
}

void divideTriangleWithoutNormalizing(glm::vec3 a, glm::vec3 b, glm::vec3 c, int count) {
	if (count > 0)
	{
		glm::vec3 ab = (a + b) / 2.0f;
		glm::vec3 ac = (a + c) / 2.0f;
		glm::vec3 bc = (b + c) / 2.0f;

		divideTriangleWithoutNormalizing(a, ab, ac, count - 1);
		divideTriangleWithoutNormalizing(ab, b, bc, count - 1);
		divideTriangleWithoutNormalizing(bc, c, ac, count - 1);
		divideTriangleWithoutNormalizing(ab, bc, ac, count - 1);
	}
	else {
		triangle(a, b, c);
	}
}

void moveForward() {
	glm::mat4 current;
	current = mvStack.top();
	current = current * glm::translate(glm::mat4(1.0), glm::vec3(0.0, 0.0, delta));
	mvStack.pop();
	mvStack.push(current);

	//viewStack.pop();
	/*
		camera = camera + glm::vec3(0.0, 0.0, delta );
	lookAt = camera + glm::vec3(0.0, 0.0, 1);
	std::cout << camera.z << std::endl;
	viewStack.push(glm::lookAt(camera, lookAt, up));
	*/

	//currentView = currentView * glm::translate(glm::mat4(1.0), glm::vec3(0.0, 0.0, delta));
}

void moveBackward() {
	glm::mat4 currentModel;
	currentModel = mvStack.top();
	currentModel = currentModel * glm::translate(glm::mat4(1.0), glm::vec3(0.0, 0.0, -delta));
	mvStack.pop();
	mvStack.push(currentModel);



}

void moveLeft() {
	glm::mat4 current;
	current = mvStack.top();
	current = current * glm::translate(glm::mat4(1.0), glm::vec3(delta, 0.0, 0.0));
	mvStack.pop();
	mvStack.push(current);
}

void moveRight() {
	glm::mat4 current;
	current = mvStack.top();
	current = current * glm::translate(glm::mat4(1.0), glm::vec3(-delta, 0.0, 0.0));
	mvStack.pop();
	mvStack.push(current);
}

