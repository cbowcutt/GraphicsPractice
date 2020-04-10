/*
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <gl/GL.h>
#include <gl/GLU.h>


#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include <iostream>

glm::vec3 rotationVector;
GLfloat rotateDirection;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

const char *vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec3 color;\n"
"out vec4 assignedColor;\n"
"uniform mat4 mvp;\n"
"void main()\n"
"{\n"
"   gl_Position = mvp * vec4(aPos, 1.0);\n"
"	assignedColor = vec4(color, 1.0);\n"
"}\0";

const char *fragmentShaderSource = "#version 330 core\n"
"out vec4 fragColor;\n"
"in vec4 assignedColor;\n"
"void main()\n"
"{\n"
"   fragColor = assignedColor;\n"
"}\n\0";

int main()
{
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
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	// set up vertex data (and buffer(s)) and configure vertex attributes
	// ------------------------------------------------------------------
	GLfloat vertices[] = {
		0.0f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f,  // top, red
		0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,  // right, green
		0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 1.0f, // front, blue
		-0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, // left, purple
		0.0f, 0.0f, -0.5f, 1.0f, 1.0f, 0.0f, // back, yellow
		0.0f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f // bottom, turquoise
	};

	rotationVector = glm::vec3(0.0f, 1.0f, 0.0f);
	rotateDirection = 1.0;
	GLint indices[] = {
		0, 2, 1, // top front right
		0, 4, 3, // top back left,
		0, 1, 4,// top right back
		0, 3, 2,// top left front
		0, 1, 4, // right top back
		0, 3, 4,
		5, 3, 4,// bottom left back
		5, 4, 1,// right bottom back
		5, 2, 1,// bottom right front
		5, 2, 3// left bottom front
	};
	glCullFace(GL_FRONT_AND_BACK);
	GLuint VBO, VAO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	// You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
	// VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
	// glBindVertexArray(0);

	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// bind the VAO (it was already bound, but just to demonstrate): seeing as we only have a single VAO we can 
	// just bind it beforehand before rendering the respective triangle; this is another approach.
	glBindVertexArray(VAO);


	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		// input
		// -----
		processInput(window);

		// render
		// ------
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// be sure to activate the shader before any calls to glUniform
		glUseProgram(shaderProgram);
		// set the transform matrix
		GLuint transformLoc = glGetUniformLocation(shaderProgram, "transform");

		// model matrix
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::rotate(model,(GLfloat) glfwGetTime() * 360.f  / 200.0f, rotationVector);

		// view
	
		glm::mat4 view = glm::lookAt(
			glm::vec3(0, 0, 2), // Camera is at (4,3,3), in World Space
			glm::vec3(0, 0, 0),
			glm::vec3(0, 1, 0)
		);


		// projection
		glm::mat4 projection = glm::perspective(45.0f, 1.0f, 0.1f, 10.0f);
		
		glm::mat4 mvp = projection * view * model;

		GLuint mvpLocation = glGetUniformLocation(shaderProgram, "mvp");
		glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, glm::value_ptr(mvp));
		glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);


		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
	return 0;
}
/*


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
	{
		rotationVector = glm::vec3(0.0f, 1.0f, 0.0f);
		rotateDirection = 1.0f;
	}
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
	{
		rotationVector = glm::vec3(0.0f, -1.0f, 0.0f);
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
*/