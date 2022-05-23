// Quick note: GLAD needs to be included first before GLFW.
// Otherwise, GLAD will complain about gl.h being already included.
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <cstddef>
#include <fstream>
#include <iostream>
#include <string>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// ---------------
// Function declarations
// ---------------

/// <summary>
/// Creates a shader program based on the provided file paths for the vertex and fragment shaders.
/// </summary>
/// <param name="vertexShaderFilePath">Vertex shader file path</param>
/// <param name="fragmentShaderFilePath">Fragment shader file path</param>
/// <returns>OpenGL handle to the created shader program</returns>
GLuint CreateShaderProgram(const std::string& vertexShaderFilePath, const std::string& fragmentShaderFilePath);

/// <summary>
/// Creates a shader based on the provided shader type and the path to the file containing the shader source.
/// </summary>
/// <param name="shaderType">Shader type</param>
/// <param name="shaderFilePath">Path to the file containing the shader source</param>
/// <returns>OpenGL handle to the created shader</returns>
GLuint CreateShaderFromFile(const GLuint& shaderType, const std::string& shaderFilePath);

/// <summary>
/// Creates a shader based on the provided shader type and the string containing the shader source.
/// </summary>
/// <param name="shaderType">Shader type</param>
/// <param name="shaderSource">Shader source string</param>
/// <returns>OpenGL handle to the created shader</returns>
GLuint CreateShaderFromSource(const GLuint& shaderType, const std::string& shaderSource);

/// <summary>
/// Function for handling the event when the size of the framebuffer changed.
/// </summary>
/// <param name="window">Reference to the window</param>
/// <param name="width">New width</param>
/// <param name="height">New height</param>
void FramebufferSizeChangedCallback(GLFWwindow* window, int width, int height);

void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
/// <summary>
/// Struct containing data about a vertex
/// </summary>
struct Vertex
{
	GLfloat x, y, z;	// Position
	GLubyte r, g, b;	// Color
	GLfloat u, v;		// UV coordinates
	GLfloat nx, ny, nz; // Normal Vectors
	
};

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

float deltaTime = 0.0f;
float lastFrame = 0.0f;

// mouse state
bool firstMouse = true;
float lastX = 800.0f / 2.0;
float lastY = 600.0f / 2.0;

float yaw = -90.0f;
float pitch = 0.0f;
float fov = 45.0f;

float skyboxVertices[] =
{
	//   Coordinates
	-1.0f, -1.0f,  1.0f,//        7--------6
	 1.0f, -1.0f,  1.0f,//       /|       /|
	 1.0f, -1.0f, -1.0f,//      4--------5 |
	-1.0f, -1.0f, -1.0f,//      | |      | |
	-1.0f,  1.0f,  1.0f,//      | 3------|-2
	 1.0f,  1.0f,  1.0f,//      |/       |/
	 1.0f,  1.0f, -1.0f,//      0--------1
	-1.0f,  1.0f, -1.0f
};

unsigned int skyboxIndices[] =
{
	// Right
	1, 2, 6,
	6, 5, 1,
	// Left
	0, 4, 7,
	7, 3, 0,
	// Top
	4, 5, 6,
	6, 7, 4,
	// Bottom
	0, 3, 2,
	2, 1, 0,
	// Back
	0, 1, 5,
	5, 4, 0,
	// Front
	3, 7, 6,
	6, 2, 3
};

/// <summary>
/// Main function.
/// </summary>
/// <returns>An integer indicating whether the program ended successfully or not.
/// A value of 0 indicates the program ended succesfully, while a non-zero value indicates
/// something wrong happened during execution.</returns>
int main()
{
	// Initialize GLFW
	int glfwInitStatus = glfwInit();
	if (glfwInitStatus == GLFW_FALSE)
	{
		std::cerr << "Failed to initialize GLFW!" << std::endl;
		return 1;
	}

	// Tell GLFW that we prefer to use OpenGL 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	// Tell GLFW that we prefer to use the modern OpenGL
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Tell GLFW to create a window
	float windowWidth = 1920;
	float windowHeight = 1080;
	GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "Final Project", nullptr, nullptr);
	if (window == nullptr)
	{
		std::cerr << "Failed to create GLFW window!" << std::endl;
		glfwTerminate();
		return 1;
	}

	// Tell GLFW to use the OpenGL context that was assigned to the window that we just created
	glfwMakeContextCurrent(window);

	// Register the callback function that handles when the framebuffer size has changed
	glfwSetFramebufferSizeCallback(window, FramebufferSizeChangedCallback);

	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	// Tell GLAD to load the OpenGL function pointers
	if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
	{
		std::cerr << "Failed to initialize GLAD!" << std::endl;
		return 1;
	}

	// --- Vertex specification ---

	Vertex vertices[240];
	// Right Wall
	vertices[0] = { 1.0f, -1.0f, -1.0f,		255, 255, 255,		0.0f, 0.5f,		-1.0f, 0.0f, 0.0f };
	vertices[1] = { 1.0f, 1.0f, -1.0f,		255, 255, 255,		0.0f, 1.0f,		-1.0f, 0.0f, 0.0f };
	vertices[2] = { 1.0f, 1.0f, 1.0f,		255, 255, 255,		0.5f, 1.0f,		-1.0f, 0.0f, 0.0f };
	vertices[3] = { 1.0f, -1.0f, -1.0f,		255, 255, 255,		0.0f, 0.5f,		-1.0f, 0.0f, 0.0f };
	vertices[4] = { 1.0f, 1.0f, 1.0f,		255, 255, 255,		0.5f, 1.0f,		-1.0f, 0.0f, 0.0f };
	vertices[5] = { 1.0f, -1.0f, 1.0f,		255, 255, 255,		0.5f, 0.5f,		-1.0f, 0.0f, 0.0f };

	// Ceiling
	vertices[6] = { 1.0f, 1.0f, -1.0f,		255, 255, 255,		1.0f, 0.5f,		0.0f, -1.0f, 0.0f };
	vertices[7] = { -1.0f, 1.0f, -1.0f,		255, 255, 255,		1.0f, 1.0f,		0.0f, -1.0f, 0.0f };
	vertices[8] = { -1.0f, 1.0f, 1.0f,		255, 255, 255,		0.5f, 1.0f,		0.0f, -1.0f, 0.0f };
	vertices[9] = { 1.0f, 1.0f, -1.0f,		255, 255, 255,		1.0f, 0.5f,		0.0f, -1.0f, 0.0f };
	vertices[10] = { -1.0f, 1.0f, 1.0f,		255, 255, 255,		0.5f, 1.0f,		0.0f, -1.0f, 0.0f };
	vertices[11] = { 1.0f, 1.0f, 1.0f,		255, 255, 255,		0.5f, 0.5f,		0.0f, -1.0f, 0.0f };

	// Left Wall
	vertices[12] = { -1.0f, -1.0f, -1.0f,	255, 255, 255,		0.0f, 0.5f,		1.0f, 0.0f, 0.0f };
	vertices[13] = { -1.0f, 1.0f, -1.0f,	255, 255, 255,		0.0f, 1.0f,		1.0f, 0.0f, 0.0f };
	vertices[14] = { -1.0f, 1.0f, 1.0f,		255, 255, 255,		0.5f, 1.0f,		1.0f, 0.0f, 0.0f };
	vertices[15] = { -1.0f, -1.0f, -1.0f,	255, 255, 255,		0.0f, 0.5f,		1.0f, 0.0f, 0.0f };
	vertices[16] = { -1.0f, 1.0f, 1.0f,		255, 255, 255,		0.5f, 1.0f,		1.0f, 0.0f, 0.0f };
	vertices[17] = { -1.0f, -1.0f, 1.0f,	255, 255, 255,		0.5f, 0.5f,		1.0f, 0.0f, 0.0f };

	// Floor
	vertices[18] = { -1.0f, -1.0f, -1.0f,	255, 255, 255,		1.0f, 0.5f,		0.0f, 1.0f, 0.0f };
	vertices[19] = { 1.0f, -1.0f, -1.0f,	255, 255, 255,		1.0f, 1.0f,		0.0f, 1.0f, 0.0f };
	vertices[20] = { 1.0f, -1.0f, 1.0f,		255, 255, 255,		0.5f, 1.0f,		0.0f, 1.0f, 0.0f };
	vertices[21] = { -1.0f, -1.0f, -1.0f,	255, 255, 255,		1.0f, 0.5f,		0.0f, 1.0f, 0.0f };
	vertices[22] = { 1.0f, -1.0f, 1.0f,		255, 255, 255,		0.5f, 1.0f,		0.0f, 1.0f, 0.0f };
	vertices[23] = { -1.0f, -1.0f, 1.0f,	255, 255, 255,		0.5f, 0.5f,		0.0f, 1.0f, 0.0f };

	// Front Wall
	vertices[24] = { 1.0f, -1.0f, 1.0f,		255, 255, 255,		0.0f, 0.5f,		0.0f, 0.0f, -1.0f };
	vertices[25] = { 1.0f, 1.0f, 1.0f,		255, 255, 255,		0.0f, 1.0f,		0.0f, 0.0f, -1.0f };
	vertices[26] = { -1.0f, 1.0f, 1.0f,		255, 255, 255,		0.5f, 1.0f,		0.0f, 0.0f, -1.0f };
	vertices[27] = { 1.0f, -1.0f, 1.0f,		255, 255, 255,		0.0f, 0.5f,		0.0f, 0.0f, -1.0f };
	vertices[28] = { -1.0f, 1.0f, 1.0f,		255, 255, 255,		0.5f, 1.0f,		0.0f, 0.0f, -1.0f };
	vertices[29] = { -1.0f, -1.0f, 1.0f,	255, 255, 255,		0.5f, 0.5f,		0.0f, 0.0f, -1.0f };

	// Back Wall
	vertices[30] = { 1.0f, -1.0f, -1.0f,	255, 255, 255,		0.0f, 0.5f,		0.0f, 0.0f, 1.0f };
	vertices[31] = { 1.0f, 1.0f, -1.0f,		255, 255, 255,		0.0f, 1.0f,		0.0f, 0.0f, 1.0f };
	vertices[32] = { -1.0f, 1.0f, -1.0f,	255, 255, 255,		0.5f, 1.0f,		0.0f, 0.0f, 1.0f };
	vertices[33] = { 1.0f, -1.0f, -1.0f,	255, 255, 255,		0.0f, 0.5f,		0.0f, 0.0f, 1.0f };
	vertices[34] = { -1.0f, 1.0f, -1.0f,	255, 255, 255,		0.5f, 1.0f,		0.0f, 0.0f, 1.0f };
	vertices[35] = { -1.0f, -1.0f, -1.0f,	255, 255, 255,		0.5f, 0.5f,		0.0f, 0.0f, 1.0f };

	//Door
	vertices[36] = { 0.25f, -1.0f, 1.01f,	255, 255, 255,		0.25f, 0.0f,	0.0f, 0.0f, 1.0f };
	vertices[37] = { 0.25f, 0.0f, 1.01f,	255, 255, 255,		0.25f, 0.5f,	0.0f, 0.0f, 1.0f };
	vertices[38] = { -0.25f, 0.0f, 1.01f,	255, 255, 255,		0.0f, 0.5f,		0.0f, 0.0f, 1.0f };
	vertices[39] = { 0.25f, -1.0f, 1.01f,	255, 255, 255,		0.25f, 0.0f,	0.0f, 0.0f, 1.0f };
	vertices[40] = { -0.25f, 0.0f, 1.01f,	255, 255, 255,		0.0f, 0.5f,		0.0f, 0.0f, 1.0f };
	vertices[41] = { -0.25f, -1.0f, 1.01f,	255, 255, 255,		0.0f, 0.0f,		0.0f, 0.0f, 1.0f };

	// Right Wall Big Crate
	vertices[42] = { 1.0f, -1.0f, -1.0f,	255, 255, 255,		0.6f, 0.15f,	1.0f, 0.0f, 0.0f };
	vertices[43] = { 1.0f, 1.0f, -1.0f,		255, 255, 255,		0.6f, 0.5f,		1.0f, 0.0f, 0.0f };
	vertices[44] = { 1.0f, 1.0f, 1.0f,		255, 255, 255,		0.25f, 0.5f,	1.0f, 0.0f, 0.0f };
	vertices[45] = { 1.0f, -1.0f, -1.0f,	255, 255, 255,		0.6f, 0.15f,	1.0f, 0.0f, 0.0f };
	vertices[46] = { 1.0f, 1.0f, 1.0f,		255, 255, 255,		0.25f, 0.5f,	1.0f, 0.0f, 0.0f };
	vertices[47] = { 1.0f, -1.0f, 1.0f,		255, 255, 255,		0.25f, 0.15f,	1.0f, 0.0f, 0.0f };

	// Ceiling Big Crate
	vertices[48] = { 1.0f, 1.0f, -1.0f,		255, 255, 255,		0.6f, 0.15f,	0.0f, 1.0f, 0.0f };
	vertices[49] = { -1.0f, 1.0f, -1.0f,	255, 255, 255,		0.6f, 0.5f,		0.0f, 1.0f, 0.0f };
	vertices[50] = { -1.0f, 1.0f, 1.0f,		255, 255, 255,		0.25f, 0.5f,	0.0f, 1.0f, 0.0f };
	vertices[51] = { 1.0f, 1.0f, -1.0f,		255, 255, 255,		0.6f, 0.15f,	0.0f, 1.0f, 0.0f };
	vertices[52] = { -1.0f, 1.0f, 1.0f,		255, 255, 255,		0.25f, 0.5f,	0.0f, 1.0f, 0.0f };
	vertices[53] = { 1.0f, 1.0f, 1.0f,		255, 255, 255,		0.25f, 0.15f,	0.0f, 1.0f, 0.0f };

	// Left Wall Big Crate
	vertices[54] = { -1.0f, -1.0f, -1.0f,	255, 255, 255,		0.6f, 0.15f,	-1.0f, 0.0f, 0.0f };
	vertices[55] = { -1.0f, 1.0f, -1.0f,	255, 255, 255,		0.6f, 0.5f,		-1.0f, 0.0f, 0.0f };
	vertices[56] = { -1.0f, 1.0f, 1.0f,		255, 255, 255,		0.25f, 0.5f,	-1.0f, 0.0f, 0.0f };
	vertices[57] = { -1.0f, -1.0f, -1.0f,	255, 255, 255,		0.6f, 0.15f,	-1.0f, 0.0f, 0.0f };
	vertices[58] = { -1.0f, 1.0f, 1.0f,		255, 255, 255,		0.25f, 0.5f,	-1.0f, 0.0f, 0.0f };
	vertices[59] = { -1.0f, -1.0f, 1.0f,	255, 255, 255,		0.25f, 0.15f,	-1.0f, 0.0f, 0.0f };

	// Floor Big Crate
	vertices[60] = { -1.0f, -1.0f, -1.0f,	255, 255, 255,		0.6f, 0.15f,	0.0f, -1.0f, 0.0f };
	vertices[61] = { 1.0f, -1.0f, -1.0f,	255, 255, 255,		0.6f, 0.5f,		0.0f, -1.0f, 0.0f };
	vertices[62] = { 1.0f, -1.0f, 1.0f,		255, 255, 255,		0.25f, 0.5f,	0.0f, -1.0f, 0.0f };
	vertices[63] = { -1.0f, -1.0f, -1.0f,	255, 255, 255,		0.6f, 0.15f,	0.0f, -1.0f, 0.0f };
	vertices[64] = { 1.0f, -1.0f, 1.0f,		255, 255, 255,		0.25f, 0.5f,	0.0f, -1.0f, 0.0f };
	vertices[65] = { -1.0f, -1.0f, 1.0f,	255, 255, 255,		0.25f, 0.15f,	0.0f, -1.0f, 0.0f };

	// Front Wall Big Crate
	vertices[66] = { 1.0f, -1.0f, 1.0f,		255, 255, 255,		0.6f, 0.15f,	0.0f, 0.0f, 1.0f };
	vertices[67] = { 1.0f, 1.0f, 1.0f,		255, 255, 255,		0.6f, 0.5f,		0.0f, 0.0f, 1.0f };
	vertices[68] = { -1.0f, 1.0f, 1.0f,		255, 255, 255,		0.25f, 0.5f,	0.0f, 0.0f, 1.0f };
	vertices[69] = { 1.0f, -1.0f, 1.0f,		255, 255, 255,		0.6f, 0.15f,	0.0f, 0.0f, 1.0f };
	vertices[70] = { -1.0f, 1.0f, 1.0f,		255, 255, 255,		0.25f, 0.5f,	0.0f, 0.0f, 1.0f };
	vertices[71] = { -1.0f, -1.0f, 1.0f,	255, 255, 255,		0.25f, 0.15f,	0.0f, 0.0f, 1.0f };

	// Back Wall Big Crate
	vertices[72] = { 1.0f, -1.0f, -1.0f,	255, 255, 255,		0.6f, 0.15f,	0.0f, 0.0f, -1.0f };
	vertices[73] = { 1.0f, 1.0f, -1.0f,		255, 255, 255,		0.6f, 0.5f,		0.0f, 0.0f, -1.0f };
	vertices[74] = { -1.0f, 1.0f, -1.0f,	255, 255, 255,		0.25f, 0.5f,	0.0f, 0.0f, -1.0f };
	vertices[75] = { 1.0f, -1.0f, -1.0f,	255, 255, 255,		0.6f, 0.15f,	0.0f, 0.0f, -1.0f };
	vertices[76] = { -1.0f, 1.0f, -1.0f,	255, 255, 255,		0.25f, 0.5f,	0.0f, 0.0f, -1.0f };
	vertices[77] = { -1.0f, -1.0f, -1.0f,	255, 255, 255,		0.25f, 0.15f,	0.0f, 0.0f, -1.0f };

	// Right Wall Big Crate 2
	vertices[78] = { 1.0f, -1.0f, -1.0f,	255, 255, 255,		0.6f, 0.15f,	1.0f, 0.0f, 0.0f };
	vertices[79] = { 1.0f, 1.0f, -1.0f,		255, 255, 255,		0.6f, 0.5f,		1.0f, 0.0f, 0.0f };
	vertices[80] = { 1.0f, 1.0f, 1.0f,		255, 255, 255,		0.25f, 0.5f,	1.0f, 0.0f, 0.0f };
	vertices[81] = { 1.0f, -1.0f, -1.0f,	255, 255, 255,		0.6f, 0.15f,	1.0f, 0.0f, 0.0f };
	vertices[82] = { 1.0f, 1.0f, 1.0f,		255, 255, 255,		0.25f, 0.5f,	1.0f, 0.0f, 0.0f };
	vertices[83] = { 1.0f, -1.0f, 1.0f,		255, 255, 255,		0.25f, 0.15f,	1.0f, 0.0f, 0.0f };

	// Ceiling Big Crate 2
	vertices[84] = { 1.0f, 1.0f, -1.0f,		255, 255, 255,		0.6f, 0.15f,	0.0f, 1.0f, 0.0f };
	vertices[85] = { -1.0f, 1.0f, -1.0f,	255, 255, 255,		0.6f, 0.5f,		0.0f, 1.0f, 0.0f };
	vertices[86] = { -1.0f, 1.0f, 1.0f,		255, 255, 255,		0.25f, 0.5f,	0.0f, 1.0f, 0.0f };
	vertices[87] = { 1.0f, 1.0f, -1.0f,		255, 255, 255,		0.6f, 0.15f,	0.0f, 1.0f, 0.0f };
	vertices[88] = { -1.0f, 1.0f, 1.0f,		255, 255, 255,		0.25f, 0.5f,	0.0f, 1.0f, 0.0f };
	vertices[89] = { 1.0f, 1.0f, 1.0f,		255, 255, 255,		0.25f, 0.15f,	0.0f, 1.0f, 0.0f };

	// Left Wall Big Crate 2
	vertices[90] = { -1.0f, -1.0f, -1.0f,	255, 255, 255,		0.6f, 0.15f,	-1.0f, 0.0f, 0.0f };
	vertices[91] = { -1.0f, 1.0f, -1.0f,	255, 255, 255,		0.6f, 0.5f,		-1.0f, 0.0f, 0.0f };
	vertices[92] = { -1.0f, 1.0f, 1.0f,		255, 255, 255,		0.25f, 0.5f,	-1.0f, 0.0f, 0.0f };
	vertices[93] = { -1.0f, -1.0f, -1.0f,	255, 255, 255,		0.6f, 0.15f,	-1.0f, 0.0f, 0.0f };
	vertices[94] = { -1.0f, 1.0f, 1.0f,		255, 255, 255,		0.25f, 0.5f,	-1.0f, 0.0f, 0.0f };
	vertices[95] = { -1.0f, -1.0f, 1.0f,	255, 255, 255,		0.25f, 0.15f,	-1.0f, 0.0f, 0.0f };

	// Floor Big Crate 2
	vertices[96] = { -1.0f, -1.0f, -1.0f,	255, 255, 255,		0.6f, 0.15f,	0.0f, -1.0f, 0.0f };
	vertices[97] = { 1.0f, -1.0f, -1.0f,	255, 255, 255,		0.6f, 0.5f,		0.0f, -1.0f, 0.0f };
	vertices[98] = { 1.0f, -1.0f, 1.0f,		255, 255, 255,		0.25f, 0.5f,	0.0f, -1.0f, 0.0f };
	vertices[99] = { -1.0f, -1.0f, -1.0f,	255, 255, 255,		0.6f, 0.15f,	0.0f, -1.0f, 0.0f };
	vertices[100] = { 1.0f, -1.0f, 1.0f,	255, 255, 255,		0.25f, 0.5f,	0.0f, -1.0f, 0.0f };
	vertices[101] = { -1.0f, -1.0f, 1.0f,	255, 255, 255,		0.25f, 0.15f,	0.0f, -1.0f, 0.0f };

	// Front Wall Big Crate 2
	vertices[102] = { 1.0f, -1.0f, 1.0f,	255, 255, 255,		0.6f, 0.15f,	0.0f, 0.0f, -1.0f };
	vertices[103] = { 1.0f, 1.0f, 1.0f,		255, 255, 255,		0.6f, 0.5f,		0.0f, 0.0f, -1.0f };
	vertices[104] = { -1.0f, 1.0f, 1.0f,	255, 255, 255,		0.25f, 0.5f,	0.0f, 0.0f, -1.0f };
	vertices[105] = { 1.0f, -1.0f, 1.0f,	255, 255, 255,		0.6f, 0.15f,	0.0f, 0.0f, -1.0f };
	vertices[106] = { -1.0f, 1.0f, 1.0f,	255, 255, 255,		0.25f, 0.5f,	0.0f, 0.0f, -1.0f };
	vertices[107] = { -1.0f, -1.0f, 1.0f,	255, 255, 255,		0.25f, 0.15f,	0.0f, 0.0f, -1.0f };

	// Back Wall Big Crate 2
	vertices[108] = { 1.0f, -1.0f, -1.0f,	255, 255, 255,		0.6f, 0.15f,	0.0f, 0.0f, 1.0f };
	vertices[109] = { 1.0f, 1.0f, -1.0f,	255, 255, 255,		0.6f, 0.5f,		0.0f, 0.0f, 1.0f };
	vertices[110] = { -1.0f, 1.0f, -1.0f,	255, 255, 255,		0.25f, 0.5f,	0.0f, 0.0f, 1.0f };
	vertices[111] = { 1.0f, -1.0f, -1.0f,	255, 255, 255,		0.6f, 0.15f,	0.0f, 0.0f, 1.0f };
	vertices[112] = { -1.0f, 1.0f, -1.0f,	255, 255, 255,		0.25f, 0.5f,	0.0f, 0.0f, 1.0f };
	vertices[113] = { -1.0f, -1.0f, -1.0f,	255, 255, 255,		0.25f, 0.15f,	0.0f, 0.0f, 1.0f };

	// Right Wall Big Crate 3
	vertices[114] = { 1.0f, -1.0f, -1.0f,	255, 255, 255,		0.6f, 0.15f,	1.0f, 0.0f, 0.0f };
	vertices[115] = { 1.0f, 1.0f, -1.0f,	255, 255, 255,		0.6f, 0.5f,		1.0f, 0.0f, 0.0f };
	vertices[116] = { 1.0f, 1.0f, 1.0f,		255, 255, 255,		0.25f, 0.5f,	1.0f, 0.0f, 0.0f };
	vertices[117] = { 1.0f, -1.0f, -1.0f,	255, 255, 255,		0.6f, 0.15f,	1.0f, 0.0f, 0.0f };
	vertices[118] = { 1.0f, 1.0f, 1.0f,		255, 255, 255,		0.25f, 0.5f,	1.0f, 0.0f, 0.0f };
	vertices[119] = { 1.0f, -1.0f, 1.0f,	255, 255, 255,		0.25f, 0.15f,	1.0f, 0.0f, 0.0f };

	// Ceiling Big Crate 3
	vertices[120] = { 1.0f, 1.0f, -1.0f,	255, 255, 255,		0.6f, 0.15f,	0.0f, 1.0f, 0.0f };
	vertices[121] = { -1.0f, 1.0f, -1.0f,	255, 255, 255,		0.6f, 0.5f,		0.0f, 1.0f, 0.0f };
	vertices[122] = { -1.0f, 1.0f, 1.0f,	255, 255, 255,		0.25f, 0.5f,	0.0f, 1.0f, 0.0f };
	vertices[123] = { 1.0f, 1.0f, -1.0f,	255, 255, 255,		0.6f, 0.15f,	0.0f, 1.0f, 0.0f };
	vertices[124] = { -1.0f, 1.0f, 1.0f,	255, 255, 255,		0.25f, 0.5f,	0.0f, 1.0f, 0.0f };
	vertices[125] = { 1.0f, 1.0f, 1.0f,		255, 255, 255,		0.25f, 0.15f,	0.0f, 1.0f, 0.0f };

	// Left Wall Big Crate 3
	vertices[126] = { -1.0f, -1.0f, -1.0f,	255, 255, 255,		0.6f, 0.15f,	-1.0f, 0.0f, 0.0f };
	vertices[127] = { -1.0f, 1.0f, -1.0f,	255, 255, 255,		0.6f, 0.5f,		-1.0f, 0.0f, 0.0f };
	vertices[128] = { -1.0f, 1.0f, 1.0f,	255, 255, 255,		0.25f, 0.5f,	-1.0f, 0.0f, 0.0f };
	vertices[129] = { -1.0f, -1.0f, -1.0f,	255, 255, 255,		0.6f, 0.15f,	-1.0f, 0.0f, 0.0f };
	vertices[130] = { -1.0f, 1.0f, 1.0f,	255, 255, 255,		0.25f, 0.5f,	-1.0f, 0.0f, 0.0f };
	vertices[131] = { -1.0f, -1.0f, 1.0f,	255, 255, 255,		0.25f, 0.15f,	-1.0f, 0.0f, 0.0f };

	// Floor Big Crate 3
	vertices[132] = { -1.0f, -1.0f, -1.0f,	255, 255, 255,		0.6f, 0.15f,	0.0f, -1.0f, 0.0f };
	vertices[133] = { 1.0f, -1.0f, -1.0f,	255, 255, 255,		0.6f, 0.5f,		0.0f, -1.0f, 0.0f };
	vertices[134] = { 1.0f, -1.0f, 1.0f,	255, 255, 255,		0.25f, 0.5f,	0.0f, -1.0f, 0.0f };
	vertices[135] = { -1.0f, -1.0f, -1.0f,	255, 255, 255,		0.6f, 0.15f,	0.0f, -1.0f, 0.0f };
	vertices[136] = { 1.0f, -1.0f, 1.0f,	255, 255, 255,		0.25f, 0.5f,	0.0f, -1.0f, 0.0f };
	vertices[137] = { -1.0f, -1.0f, 1.0f,	255, 255, 255,		0.25f, 0.15f,	0.0f, -1.0f, 0.0f };

	// Front Wall Big Crate 3
	vertices[138] = { 1.0f, -1.0f, 1.0f,	255, 255, 255,		0.6f, 0.15f,	0.0f, 0.0f, -1.0f };
	vertices[139] = { 1.0f, 1.0f, 1.0f,		255, 255, 255,		0.6f, 0.5f,		0.0f, 0.0f, -1.0f };
	vertices[140] = { -1.0f, 1.0f, 1.0f,	255, 255, 255,		0.25f, 0.5f,	0.0f, 0.0f, -1.0f };
	vertices[141] = { 1.0f, -1.0f, 1.0f,	255, 255, 255,		0.6f, 0.15f,	0.0f, 0.0f, -1.0f };
	vertices[142] = { -1.0f, 1.0f, 1.0f,	255, 255, 255,		0.25f, 0.5f,	0.0f, 0.0f, -1.0f };
	vertices[143] = { -1.0f, -1.0f, 1.0f,	255, 255, 255,		0.25f, 0.15f,	0.0f, 0.0f, -1.0f };

	// Back Wall Big Crate 3
	vertices[144] = { 1.0f, -1.0f, -1.0f,	255, 255, 255,		0.6f, 0.15f,	0.0f, 0.0f, 1.0f };
	vertices[145] = { 1.0f, 1.0f, -1.0f,	255, 255, 255,		0.6f, 0.5f,		0.0f, 0.0f, 1.0f };
	vertices[146] = { -1.0f, 1.0f, -1.0f,	255, 255, 255,		0.25f, 0.5f,	0.0f, 0.0f, 1.0f };
	vertices[147] = { 1.0f, -1.0f, -1.0f,	255, 255, 255,		0.6f, 0.15f,	0.0f, 0.0f, 1.0f };
	vertices[148] = { -1.0f, 1.0f, -1.0f,	255, 255, 255,		0.25f, 0.5f,	0.0f, 0.0f, 1.0f };
	vertices[149] = { -1.0f, -1.0f, -1.0f,	255, 255, 255,		0.25f, 0.15f,	0.0f, 0.0f, 1.0f };

	//Door2
	vertices[150] = { 0.25f, -1.0f, 0.99f,	255, 255, 255,		0.25f, 0.0f,	0.0f, 0.0f, -1.0f };
	vertices[151] = { 0.25f, 0.0f, 0.99f,	255, 255, 255,		0.25f, 0.5f,	0.0f, 0.0f, -1.0f };
	vertices[152] = { -0.25f, 0.0f, 0.99f,	255, 255, 255,		0.0f, 0.5f,		0.0f, 0.0f, -1.0f };
	vertices[153] = { 0.25f, -1.0f, 0.99f,	255, 255, 255,		0.25f, 0.0f,	0.0f, 0.0f, -1.0f };
	vertices[154] = { -0.25f, 0.0f, 0.99f,	255, 255, 255,		0.0f, 0.5f,		0.0f, 0.0f, -1.0f };
	vertices[155] = { -0.25f, -1.0f, 0.99f,	255, 255, 255,		0.0f, 0.0f,		0.0f, 0.0f, -1.0f };

	// Window
	vertices[156] = { 0.50f, -1.0f, 0.99f,	255, 255, 255,		1.0f, 0.0f,		0.0f, 0.0f, 1.0f };
	vertices[157] = { 0.50f, 0.0f, 0.99f,	255, 255, 255,		1.0f, 0.5f,		0.0f, 0.0f, 1.0f };
	vertices[158] = { -0.25f, 0.0f, 0.99f,	255, 255, 255,		0.6f, 0.5f,		0.0f, 0.0f, 1.0f };
	vertices[159] = { 0.50f, -1.0f, 0.99f,	255, 255, 255,		1.0f, 0.0f,		0.0f, 0.0f, 1.0f };
	vertices[160] = { -0.25f, 0.0f, 0.99f,	255, 255, 255,		0.6f, 0.5f,		0.0f, 0.0f, 1.0f };
	vertices[161] = { -0.25f, -1.0f, 0.99f,	255, 255, 255,		0.6f, 0.0f,		0.0f, 0.0f, 1.0f };

	// Roof Base
	vertices[162] = { 1.0f, 0.0f, 1.0f,		255, 255, 255,		0.6f, 0.15f };
	vertices[163] = { 1.0f, 0.0f, -1.0f,	255, 255, 255,		0.6f, 0.5f };
	vertices[164] = { -1.0f, 0.0f, -1.0f,	255, 255, 255,		0.25f, 0.5f };
	vertices[165] = { 1.0f, 0.0f, 1.0f,		255, 255, 255,		0.6f, 0.15f };
	vertices[166] = { -1.0f, 0.0f, -1.0f,	255, 255, 255,		0.25f, 0.5f };
	vertices[167] = { -1.0f, 0.0f, 1.0f,	255, 255, 255,		0.25f, 0.15f };

	// Roof Sides
	vertices[168] = { 1.0f, 0.0f, 1.0f,		255, 255, 255,		0.6f, 0.15f };
	vertices[169] = { 0.0f, 1.0f, 0.0f,		255, 255, 255,		0.425f, 0.325f };
	vertices[170] = { -1.0f, 0.0f, 1.0f,	255, 255, 255,		0.25f, 0.15f };
	
	vertices[171] = { 1.0f, 0.0f, -1.0f,	255, 255, 255,		0.6f, 0.15f };
	vertices[172] = { 0.0f, 1.0f, 0.0f,		255, 255, 255,		0.425f, 0.325f };
	vertices[173] = { 1.0f, 0.0f, 1.0f,		255, 255, 255,		0.25f, 0.15f };

	vertices[174] = { -1.0f, 0.0f, -1.0f,	255, 255, 255,		0.6f, 0.15f };
	vertices[175] = { 0.0f, 1.0f, 0.0f,		255, 255, 255,		0.425f, 0.325f };
	vertices[176] = { 1.0f, 0.0f, -1.0f,	255, 255, 255,		0.25f, 0.15f };

	vertices[177] = { -1.0f, 0.0f, -1.0f,	255, 255, 255,		0.6f, 0.15f };
	vertices[178] = { 0.0f, 1.0f, 0.0f,		255, 255, 255,		0.425f, 0.325f };
	vertices[179] = { -1.0f, 0.0f, 1.0f,	255, 255, 255,		0.25f, 0.15f };

	// Chair back
	vertices[180] = { 0.5f, -1.0f, 1.0f,	255, 255, 255,		0.6f, 0.0f,		0.0f, 0.0f, 1.0f };
	vertices[181] = { 0.5f, 1.0f, 1.0f,		255, 255, 255,		0.6f, 0.15f,	0.0f, 0.0f, 1.0f };
	vertices[182] = { -1.0f, 1.0f, 1.0f,	255, 255, 255,		0.25f, 0.15f,	0.0f, 0.0f, 1.0f };
	vertices[183] = { 0.5f, -1.0f, 1.0f,	255, 255, 255,		0.6f, 0.0f,		0.0f, 0.0f, 1.0f };
	vertices[184] = { -1.0f, 1.0f, 1.0f,	255, 255, 255,		0.25f, 0.15f,	0.0f, 0.0f, 1.0f };
	vertices[185] = { -1.0f, -1.0f, 1.0f,	255, 255, 255,		0.25f, 0.0f,	0.0f, 0.0f, 1.0f };

	vertices[186] = { 0.5f, -1.0f, 0.5f,	255, 255, 255,		0.6f, 0.0f,		1.0f, 0.0f, 0.0f };
	vertices[187] = { 0.5f, 1.0f, 0.5f,		255, 255, 255,		0.6f, 0.15f,	1.0f, 0.0f, 0.0f };
	vertices[188] = { 0.5f, 1.0f, 1.0f,		255, 255, 255,		0.4f, 0.15f,	1.0f, 0.0f, 0.0f };
	vertices[189] = { 0.5f, -1.0f, 0.5f,	255, 255, 255,		0.6f, 0.0f,		1.0f, 0.0f, 0.0f };
	vertices[190] = { 0.5f, 1.0f, 1.0f,		255, 255, 255,		0.4f, 0.15f,	1.0f, 0.0f, 0.0f };
	vertices[191] = { 0.5f, -1.0f, 1.0f,	255, 255, 255,		0.4f, 0.0f,		1.0f, 0.0f, 0.0f };

	vertices[192] = { 0.5f, -1.0f, 0.5f,	255, 255, 255,		0.6f, 0.0f,		0.0f, 0.0f, -1.0f };
	vertices[193] = { 0.5f, 1.0f, 0.5f,		255, 255, 255,		0.6f, 0.15f,	0.0f, 0.0f, -1.0f };
	vertices[194] = { -1.0f, 1.0f, 0.5f,	255, 255, 255,		0.25f, 0.15f,	0.0f, 0.0f, -1.0f };
	vertices[195] = { 0.5f, -1.0f, 0.5f,	255, 255, 255,		0.6f, 0.0f,		0.0f, 0.0f, -1.0f };
	vertices[196] = { -1.0f, 1.0f, 0.5f,	255, 255, 255,		0.25f, 0.15f,	0.0f, 0.0f, -1.0f };
	vertices[197] = { -1.0f, -1.0f, 0.5f,	255, 255, 255,		0.25f, 0.0f,	0.0f, 0.0f, -1.0f };

	vertices[198] = { -1.0f, -1.0f, 1.0f,	255, 255, 255,		0.6f, 0.0f,		-1.0f, 0.0f, 0.0f };
	vertices[199] = { -1.0f, 1.0f, 1.0f,	255, 255, 255,		0.6f, 0.15f,	-1.0f, 0.0f, 0.0f };
	vertices[200] = { -1.0f, 1.0f, 0.5f,	255, 255, 255,		0.4f, 0.15f,	-1.0f, 0.0f, 0.0f };
	vertices[201] = { -1.0f, -1.0f, 1.0f,	255, 255, 255,		0.6f, 0.0f,		-1.0f, 0.0f, 0.0f };
	vertices[202] = { -1.0f, 1.0f, 0.5f,	255, 255, 255,		0.4f, 0.15f,	-1.0f, 0.0f, 0.0f };
	vertices[203] = { -1.0f, -1.0f, 0.5f,	255, 255, 255,		0.4f, 0.0f,		-1.0f, 0.0f, 0.0f };
	
	vertices[204] = { 0.5f, 1.0f, 1.0f,		255, 255, 255,		0.6f, 0.0f,		0.0f, 1.0f, 0.0f };
	vertices[205] = { 0.5f, 1.0f, 0.5f,		255, 255, 255,		0.6f, 0.15f,	0.0f, 1.0f, 0.0f };
	vertices[206] = { -1.0f, 1.0f, 0.5f,	255, 255, 255,		0.4f, 0.15f,	0.0f, 1.0f, 0.0f };
	vertices[207] = { 0.5f, 1.0f, 1.0f,		255, 255, 255,		0.6f, 0.0f,		0.0f, 1.0f, 0.0f };
	vertices[208] = { -1.0f, 1.0f, 0.5f,	255, 255, 255,		0.4f, 0.15f,	0.0f, 1.0f, 0.0f };
	vertices[209] = { -1.0f, 1.0f, 1.0f,	255, 255, 255,		0.4f, 0.0f,		0.0f, 1.0f, 0.0f };

	vertices[210] = { 0.5f, -1.0f, 1.0f,	255, 255, 255,		0.6f, 0.0f,		0.0f, -1.0f, 0.0f };
	vertices[211] = { 0.5f, -1.0f, 0.5f,	255, 255, 255,		0.6f, 0.15f,	0.0f, -1.0f, 0.0f };
	vertices[212] = { -1.0f, -1.0f, 0.5f,	255, 255, 255,		0.4f, 0.15f,	0.0f, -1.0f, 0.0f };
	vertices[213] = { 0.5f, -1.0f, 1.0f,	255, 255, 255,		0.6f, 0.0f,		0.0f, -1.0f, 0.0f };
	vertices[214] = { -1.0f, -1.0f, 0.5f,	255, 255, 255,		0.4f, 0.15f,	0.0f, -1.0f, 0.0f };
	vertices[215] = { -1.0f, -1.0f, 1.0f,	255, 255, 255,		0.4f, 0.0f,		0.0f, -1.0f, 0.0f };

	// Chair legs
	vertices[216] = { 0.5f, -1.0f, 0.75f,	255, 255, 255,		0.6f, 0.0f,		1.0f, 0.0f, 0.0f };
	vertices[217] = { 0.5f, 1.0f, 0.75f,	255, 255, 255,		0.6f, 0.15f,	1.0f, 0.0f, 0.0f };
	vertices[218] = { 0.5f, 1.0f, 1.0f,		255, 255, 255,		0.4f, 0.15f,	1.0f, 0.0f, 0.0f };
	vertices[219] = { 0.5f, -1.0f, 0.75f,	255, 255, 255,		0.6f, 0.0f,		1.0f, 0.0f, 0.0f };
	vertices[220] = { 0.5f, 1.0f, 1.0f,		255, 255, 255,		0.4f, 0.15f,	1.0f, 0.0f, 0.0f };
	vertices[221] = { 0.5f, -1.0f, 1.0f,	255, 255, 255,		0.4f, 0.0f,		1.0f, 0.0f, 0.0f };

	vertices[222] = { 0.5f, -1.0f, 1.0f,	255, 255, 255,		0.6f, 0.0f,		0.0f, 0.0f, 1.0f };
	vertices[223] = { 0.5f, 1.0f, 1.0f,		255, 255, 255,		0.6f, 0.15f,	0.0f, 0.0f, 1.0f };
	vertices[224] = { 0.25f, 1.0f, 1.0f,	255, 255, 255,		0.4f, 0.15f,	0.0f, 0.0f, 1.0f };
	vertices[225] = { 0.5f, -1.0f, 1.0f,	255, 255, 255,		0.6f, 0.0f,		0.0f, 0.0f, 1.0f };
	vertices[226] = { 0.25f, 1.0f, 1.0f,	255, 255, 255,		0.4f, 0.15f,	0.0f, 0.0f, 1.0f };
	vertices[227] = { 0.25f, -1.0f, 1.0f,	255, 255, 255,		0.4f, 0.0f,		0.0f, 0.0f, 1.0f };

	vertices[228] = { 0.25f, -1.0f, 1.0f,	255, 255, 255,		0.6f, 0.0f,		-1.0f, 0.0f, 0.0f };
	vertices[229] = { 0.25f, 1.0f, 1.0f,	255, 255, 255,		0.6f, 0.15f,	-1.0f, 0.0f, 0.0f };
	vertices[230] = { 0.25f, 1.0f, 0.75f,	255, 255, 255,		0.4f, 0.15f,	-1.0f, 0.0f, 0.0f };
	vertices[231] = { 0.25f, -1.0f, 1.0f,	255, 255, 255,		0.6f, 0.0f,		-1.0f, 0.0f, 0.0f };
	vertices[232] = { 0.25f, 1.0f, 0.75f,	255, 255, 255,		0.4f, 0.15f,	-1.0f, 0.0f, 0.0f };
	vertices[233] = { 0.25f, -1.0f, 0.75f,	255, 255, 255,		0.4f, 0.0f,		-1.0f, 0.0f, 0.0f };

	vertices[234] = { 0.5f, -1.0f, 0.75f,	255, 255, 255,		0.6f, 0.0f,		0.0f, 0.0f, -1.0f };
	vertices[235] = { 0.5f, 1.0f, 0.75f,	255, 255, 255,		0.6f, 0.15f,	0.0f, 0.0f, -1.0f };
	vertices[236] = { 0.25f, 1.0f, 0.75f,	255, 255, 255,		0.4f, 0.15f,	0.0f, 0.0f, -1.0f };
	vertices[237] = { 0.5f, -1.0f, 0.75f,	255, 255, 255,		0.6f, 0.0f,		0.0f, 0.0f, -1.0f };
	vertices[238] = { 0.25f, 1.0f, 0.75f,	255, 255, 255,		0.4f, 0.15f,	0.0f, 0.0f, -1.0f };
	vertices[239] = { 0.25f, -1.0f, 0.75f,	255, 255, 255,		0.4f, 0.0f,		0.0f, 0.0f, -1.0f };

	// Create a vertex buffer object (VBO), and upload our vertices data to the VBO
	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	// Create a vertex array object that contains data on how to map vertex attributes
	// (e.g., position, color) to vertex shader properties.
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	// Vertex attribute 0 - Position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, x));

	// Vertex attribute 1 - Color
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (void*)(offsetof(Vertex, r)));

	// Vertex attribute 2 - UV coordinate
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, u)));

	//Vertex attribute 3 - Normal Vectors
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, nx)));

	glBindVertexArray(0);

	// Create a shader program
	GLuint program = CreateShaderProgram("main.vsh", "main.fsh");

	// shader program for sadown mapping
	GLuint program_mapping = CreateShaderProgram("map_shader.vsh", "map_shader.fsh");

	GLuint skyboxShader = CreateShaderProgram("skybox.vsh", "skybox.fsh");

	
	// Tell OpenGL the dimensions of the region where stuff will be drawn.
	// For now, tell OpenGL to use the whole screen
	glViewport(0, 0, windowWidth, windowHeight);

	// Create a variable that will contain the ID for our texture,
	// and use glGenTextures() to generate the texture itself
	GLuint tex;
	glGenTextures(1, &tex);

	// --- Load our image using stb_image ---

	// Im image-space (pixels), (0, 0) is the upper-left corner of the image
	// However, in u-v coordinates, (0, 0) is the lower-left corner of the image
	// This means that the image will appear upside-down when we use the image data as is
	// This function tells stbi to flip the image vertically so that it is not upside-down when we use it
	stbi_set_flip_vertically_on_load(true);

	// 'imageWidth' and imageHeight will contain the width and height of the loaded image respectively
	int imageWidth, imageHeight, numChannels;

	// Read the image data and store it in an unsigned char array
	unsigned char* imageData = stbi_load("final project texture.jpg", &imageWidth, &imageHeight, &numChannels, 0);

	// Make sure that we actually loaded the image before uploading the data to the GPU
	if (imageData != nullptr)
	{
		// Our texture is 2D, so we bind our texture to the GL_TEXTURE_2D target
		glBindTexture(GL_TEXTURE_2D, tex);

		// Set the filtering methods for magnification and minification
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		// Set the wrapping method for the s-axis (x-axis) and t-axis (y-axis)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		// Upload the image data to GPU memory
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imageWidth, imageHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, imageData);

		// If we set minification to use mipmaps, we can tell OpenGL to generate the mipmaps for us
		//glGenerateMipmap(GL_TEXTURE_2D);

		// Once we have copied the data over to the GPU, we can delete
		// the data on the CPU side, since we won't be using it anymore
		stbi_image_free(imageData);
		imageData = nullptr;
	}
	else
	{
		std::cerr << "Failed to load image" << std::endl;
	}

	// Create VAO, VBO, and EBO for the skybox
	unsigned int skyboxVAO, skyboxVBO, skyboxEBO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glGenBuffers(1, &skyboxEBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skyboxEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(skyboxIndices), &skyboxIndices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	
	// All the faces of the cubemap (make sure they are in this exact order)
	std::string facesCubemap[6] =
	{
		"Skybox/posx.jpg",
		"Skybox/negx.jpg",
		"Skybox/posy.jpg",
		"Skybox/negy.jpg",
		"Skybox/posz.jpg",
		"Skybox/negz.jpg"
	};

	// Creates the cubemap texture object
	unsigned int cubemapTexture;
	glGenTextures(1, &cubemapTexture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	// These are very important to prevent seams
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	// Cycles through all the textures and attaches them to the cubemap object
	for (unsigned int i = 0; i < 6; i++)
	{
		int width, height, nrChannels;
		unsigned char* data = stbi_load(facesCubemap[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			stbi_set_flip_vertically_on_load(false);
			glTexImage2D
			(
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0,
				GL_RGB,
				width,
				height,
				0,
				GL_RGB,
				GL_UNSIGNED_BYTE,
				data
			);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Failed to load texture: " << facesCubemap[i] << std::endl;
			stbi_image_free(data);
		}
	}

	GLuint framebuffer;
	glGenFramebuffers(1, &framebuffer);


	GLuint framebufferTex;
	glGenTextures(1, &framebufferTex);
	glBindTexture(GL_TEXTURE_2D, framebufferTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 2048, 2048, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBindTexture(GL_TEXTURE_2D, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, framebufferTex, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "Error! Framebuffer not complete!" << std::endl;
	}




	glEnable(GL_DEPTH_TEST);

	// Render loop
	while (!glfwWindowShouldClose(window))
	{
		processInput(window);
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Clear the color and depth buffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use the shader program that we created
		


		glUseProgram(program);

		// Use the vertex array object that we created
		glBindVertexArray(vao);

		// Bind our texture to texture unit 0
		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, framebufferTex);

		// Make our sampler in the fragment shader use texture unit 0
		GLint texUniformLocation = glGetUniformLocation(program, "tex");
		glUniform1i(texUniformLocation, 0);

		float time = glfwGetTime();
		// model matrices
		glm::mat4 roomModelMatrix = glm::mat4(1.0f);
		roomModelMatrix = glm::scale(roomModelMatrix, glm::vec3(5.0f, 5.0f, 5.0f));

		glm::mat4 Crate1ModelMatrix = glm::mat4(1.0f);
		Crate1ModelMatrix = glm::translate(Crate1ModelMatrix, glm::vec3(-4.0f, -4.0f, -4.0f));

		glm::mat4 projectionMatrixLight = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 10.0f, 20.0f);
		glm::mat4 viewMatrixLight = glm::lookAt(glm::vec3(0.0f, 10.0f, -10.0f), glm::vec3(0.0f, 0.0f, 0.0f), cameraUp);

		glm::mat4 Crate2ModelMatrix = glm::mat4(1.0f);
		Crate2ModelMatrix = glm::translate(Crate2ModelMatrix, glm::vec3(-4.5f, -2.6f, -3.5f));
		Crate2ModelMatrix = glm::scale(Crate2ModelMatrix, glm::vec3(0.3f, 0.3f, 0.3f));
		Crate2ModelMatrix = glm::rotate(Crate2ModelMatrix, glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));

		glm::mat4 Crate3ModelMatrix = glm::mat4(1.0f);
		Crate3ModelMatrix = glm::translate(Crate3ModelMatrix, glm::vec3(-3.5f, -2.6f, -4.0f));
		Crate3ModelMatrix = glm::scale(Crate3ModelMatrix, glm::vec3(0.3f, 0.3f, 0.3f));
		Crate3ModelMatrix = glm::rotate(Crate3ModelMatrix, glm::radians(250.0f), glm::vec3(0.0f, 1.0f, 0.0f));

		glm::mat4 WindowModelMatrix = glm::mat4(1.0f);
		WindowModelMatrix = glm::translate(WindowModelMatrix, glm::vec3(0.0f, 3.0f, 0.0f));
		WindowModelMatrix = glm::scale(WindowModelMatrix, glm::vec3(5.0f, 5.0f, 5.0f));
		WindowModelMatrix = glm::rotate(WindowModelMatrix, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

		glm::mat4 RoofModelMatrix = glm::mat4(1.0f);
		RoofModelMatrix = glm::translate(RoofModelMatrix, glm::vec3(0.0f, 5.01f, 0.0f));
		RoofModelMatrix = glm::scale(RoofModelMatrix, glm::vec3(6.0f, 6.0f, 6.0f));

		glm::mat4 ChairBackModelMatrix = glm::mat4(1.0f);
		ChairBackModelMatrix = glm::translate(ChairBackModelMatrix, glm::vec3(3.75f, -1.0f, -4.8f));
		ChairBackModelMatrix = glm::scale(ChairBackModelMatrix, glm::vec3(1.2f, 1.2f, 1.2f));
		ChairBackModelMatrix = glm::rotate(ChairBackModelMatrix, glm::radians(-25.0f), glm::vec3(0.0f, 1.0f, 0.0f));

		glm::mat4 ChairBaseModelMatrix = glm::mat4(1.0f);
		ChairBaseModelMatrix = glm::translate(ChairBaseModelMatrix, glm::vec3(3.0f, -1.6f, -3.2f));
		ChairBaseModelMatrix = glm::scale(ChairBaseModelMatrix, glm::vec3(1.2f, 1.2f, 1.2f));
		ChairBaseModelMatrix = glm::rotate(ChairBaseModelMatrix, glm::radians(-25.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		ChairBaseModelMatrix = glm::rotate(ChairBaseModelMatrix, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

		glm::mat4 ChairLeg1ModelMatrix = glm::mat4(1.0f);
		ChairLeg1ModelMatrix = glm::translate(ChairLeg1ModelMatrix, glm::vec3(2.98f, -3.7f, -3.22f));
		ChairLeg1ModelMatrix = glm::scale(ChairLeg1ModelMatrix, glm::vec3(1.2f, 1.2f, 1.2f));
		ChairLeg1ModelMatrix = glm::rotate(ChairLeg1ModelMatrix, glm::radians(-25.0f), glm::vec3(0.0f, 1.0f, 0.0f));

		glm::mat4 ChairLeg2ModelMatrix = glm::mat4(1.0f);
		ChairLeg2ModelMatrix = glm::translate(ChairLeg2ModelMatrix, glm::vec3(1.68f, -3.7f, -3.83f));
		ChairLeg2ModelMatrix = glm::scale(ChairLeg2ModelMatrix, glm::vec3(1.2f, 1.2f, 1.2f));
		ChairLeg2ModelMatrix = glm::rotate(ChairLeg2ModelMatrix, glm::radians(-25.0f), glm::vec3(0.0f, 1.0f, 0.0f));

		glm::mat4 ChairLeg3ModelMatrix = glm::mat4(1.0f);
		ChairLeg3ModelMatrix = glm::translate(ChairLeg3ModelMatrix, glm::vec3(2.5f, -3.7f, -5.6f));
		ChairLeg3ModelMatrix = glm::scale(ChairLeg3ModelMatrix, glm::vec3(1.2f, 1.2f, 1.2f));
		ChairLeg3ModelMatrix = glm::rotate(ChairLeg3ModelMatrix, glm::radians(-25.0f), glm::vec3(0.0f, 1.0f, 0.0f));

		glm::mat4 ChairLeg4ModelMatrix = glm::mat4(1.0f);
		ChairLeg4ModelMatrix = glm::translate(ChairLeg4ModelMatrix, glm::vec3(3.8f, -3.7f, -5.0f));
		ChairLeg4ModelMatrix = glm::scale(ChairLeg4ModelMatrix, glm::vec3(1.2f, 1.2f, 1.2f));
		ChairLeg4ModelMatrix = glm::rotate(ChairLeg4ModelMatrix, glm::radians(-25.0f), glm::vec3(0.0f, 1.0f, 0.0f));

		//first pass
		
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, 2048, 2048);
		glUseProgram(program_mapping);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex);

		GLint projectionlUniformLocationMapping = glGetUniformLocation(program_mapping, "projection");
		glUniformMatrix4fv(projectionlUniformLocationMapping, 1, GL_FALSE, glm::value_ptr(projectionMatrixLight));
		GLint viewUniformLocationMapping = glGetUniformLocation(program_mapping, "view");
		glUniformMatrix4fv(viewUniformLocationMapping, 1, GL_FALSE, glm::value_ptr(viewMatrixLight));

		GLint modelUniformLocationMapping = glGetUniformLocation(program_mapping, "model");
		glUniformMatrix4fv(modelUniformLocationMapping, 1, GL_FALSE, glm::value_ptr(roomModelMatrix));
		glDrawArrays(GL_TRIANGLES, 0, 30);
		//glDrawArrays(GL_TRIANGLES, 36, 6);
		//glDrawArrays(GL_TRIANGLES, 150, 6);

		modelUniformLocationMapping = glGetUniformLocation(program_mapping, "model");
		glUniformMatrix4fv(modelUniformLocationMapping, 1, GL_FALSE, glm::value_ptr(Crate1ModelMatrix));
		glDrawArrays(GL_TRIANGLES, 42, 36);

		modelUniformLocationMapping = glGetUniformLocation(program_mapping, "model");
		glUniformMatrix4fv(modelUniformLocationMapping, 1, GL_FALSE, glm::value_ptr(Crate2ModelMatrix));
		glDrawArrays(GL_TRIANGLES, 42, 36);

		modelUniformLocationMapping = glGetUniformLocation(program_mapping, "model");
		glUniformMatrix4fv(modelUniformLocationMapping, 1, GL_FALSE, glm::value_ptr(Crate3ModelMatrix));
		glDrawArrays(GL_TRIANGLES, 42, 36);

		modelUniformLocationMapping = glGetUniformLocation(program_mapping, "model");
		glUniformMatrix4fv(modelUniformLocationMapping, 1, GL_FALSE, glm::value_ptr(WindowModelMatrix));
		glDrawArrays(GL_TRIANGLES, 156, 6);

		/*modelUniformLocationMapping = glGetUniformLocation(program_mapping, "model");
		glUniformMatrix4fv(modelUniformLocationMapping, 1, GL_FALSE, glm::value_ptr(RoofModelMatrix));
		glDrawArrays(GL_TRIANGLES, 162, 18);*/

		modelUniformLocationMapping = glGetUniformLocation(program_mapping, "model");
		glUniformMatrix4fv(modelUniformLocationMapping, 1, GL_FALSE, glm::value_ptr(ChairBackModelMatrix));
		glDrawArrays(GL_TRIANGLES, 180, 36);

		modelUniformLocationMapping = glGetUniformLocation(program_mapping, "model");
		glUniformMatrix4fv(modelUniformLocationMapping, 1, GL_FALSE, glm::value_ptr(ChairBaseModelMatrix));
		glDrawArrays(GL_TRIANGLES, 180, 36);

		modelUniformLocationMapping = glGetUniformLocation(program_mapping, "model");
		glUniformMatrix4fv(modelUniformLocationMapping, 1, GL_FALSE, glm::value_ptr(ChairLeg1ModelMatrix));
		glDrawArrays(GL_TRIANGLES, 216, 24);

		modelUniformLocationMapping = glGetUniformLocation(program_mapping, "model");
		glUniformMatrix4fv(modelUniformLocationMapping, 1, GL_FALSE, glm::value_ptr(ChairLeg2ModelMatrix));
		glDrawArrays(GL_TRIANGLES, 216, 24);

		modelUniformLocationMapping = glGetUniformLocation(program_mapping, "model");
		glUniformMatrix4fv(modelUniformLocationMapping, 1, GL_FALSE, glm::value_ptr(ChairLeg3ModelMatrix));
		glDrawArrays(GL_TRIANGLES, 216, 24);

		modelUniformLocationMapping = glGetUniformLocation(program_mapping, "model");
		glUniformMatrix4fv(modelUniformLocationMapping, 1, GL_FALSE, glm::value_ptr(ChairLeg4ModelMatrix));
		glDrawArrays(GL_TRIANGLES, 216, 24);

		//second pass
		glUseProgram(program);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, windowWidth, windowHeight);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, tex);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, framebufferTex);

		glm::mat4 viewMatrix = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
		glm::mat4 projectionMatrix = glm::perspective(glm::radians(fov), windowWidth / windowHeight, 0.1f, 100.0f);
		glm::mat4 finalMatrix = projectionMatrix * viewMatrix * roomModelMatrix;

		GLint projectionlUniformLocationMappingSecond = glGetUniformLocation(program, "projectionLight");
		glUniformMatrix4fv(projectionlUniformLocationMappingSecond, 1, GL_FALSE, glm::value_ptr(projectionMatrixLight));
		GLint viewUniformLocationMappingSecond = glGetUniformLocation(program, "viewLight");
		glUniformMatrix4fv(viewUniformLocationMappingSecond, 1, GL_FALSE, glm::value_ptr(viewMatrixLight));
		
		GLint fboUniformLocation = glGetUniformLocation(program, "shadowMap");
		glUniform1i(fboUniformLocation, 1);

		GLint eyePositionUniformLocation = glGetUniformLocation(program, "eyePosition");
		glUniform3f(eyePositionUniformLocation, cameraPos.x, cameraPos.y, cameraPos.z);

		GLint lightAmbientUniformLocation = glGetUniformLocation(program, "point_ambient_intensity");
		glUniform3f(lightAmbientUniformLocation, 0.4f, 0.4f, 0.4f);

		GLint lightDiffuseUniformLocation = glGetUniformLocation(program, "point_diffuse_intensity");
		glUniform3f(lightDiffuseUniformLocation, 0.8f, 0.8f, 0.8f);

		GLint lightSpecularUniformLocation = glGetUniformLocation(program, "point_specular_intensity");
		glUniform3f(lightSpecularUniformLocation, 0.2f, 0.2f, 0.2f);

		GLint directionalLightUniformLocation = glGetUniformLocation(program, "directional_light");
		glUniform3f(directionalLightUniformLocation, 0.0f, -1.0f, 1.0f);

		GLint shininessUniformLocation = glGetUniformLocation(program, "u_shininess");
		glUniform1f(shininessUniformLocation, 1.0f);


		GLint matUniformLocation = glGetUniformLocation(program, "mat");
		glUniformMatrix4fv(matUniformLocation, 1, GL_FALSE, glm::value_ptr(finalMatrix));
		GLint modelUniformLocation = glGetUniformLocation(program, "model");
		glUniformMatrix4fv(modelUniformLocation, 1, GL_FALSE, glm::value_ptr(roomModelMatrix));

		glDrawArrays(GL_TRIANGLES, 0, 30);
		//glDrawArrays(GL_TRIANGLES, 36, 6);
		//glDrawArrays(GL_TRIANGLES, 150, 6);

		
		finalMatrix = projectionMatrix * viewMatrix * Crate1ModelMatrix;
		
		matUniformLocation = glGetUniformLocation(program, "mat");
		glUniformMatrix4fv(matUniformLocation, 1, GL_FALSE, glm::value_ptr(finalMatrix));
		modelUniformLocation = glGetUniformLocation(program, "model");
		glUniformMatrix4fv(modelUniformLocation, 1, GL_FALSE, glm::value_ptr(Crate1ModelMatrix));
		glDrawArrays(GL_TRIANGLES, 42, 36);

		
		finalMatrix = projectionMatrix * viewMatrix * Crate2ModelMatrix;

		matUniformLocation = glGetUniformLocation(program, "mat");
		glUniformMatrix4fv(matUniformLocation, 1, GL_FALSE, glm::value_ptr(finalMatrix));
		modelUniformLocation = glGetUniformLocation(program, "model");
		glUniformMatrix4fv(modelUniformLocation, 1, GL_FALSE, glm::value_ptr(Crate2ModelMatrix));
		glDrawArrays(GL_TRIANGLES, 42, 36);

		
		finalMatrix = projectionMatrix * viewMatrix * Crate3ModelMatrix;

		matUniformLocation = glGetUniformLocation(program, "mat");
		glUniformMatrix4fv(matUniformLocation, 1, GL_FALSE, glm::value_ptr(finalMatrix));
		modelUniformLocation = glGetUniformLocation(program, "model");
		glUniformMatrix4fv(modelUniformLocation, 1, GL_FALSE, glm::value_ptr(Crate3ModelMatrix));
		glDrawArrays(GL_TRIANGLES, 42, 36);

		
		finalMatrix = projectionMatrix * viewMatrix * WindowModelMatrix;

		matUniformLocation = glGetUniformLocation(program, "mat");
		glUniformMatrix4fv(matUniformLocation, 1, GL_FALSE, glm::value_ptr(finalMatrix));
		modelUniformLocation = glGetUniformLocation(program, "model");
		glUniformMatrix4fv(modelUniformLocation, 1, GL_FALSE, glm::value_ptr(WindowModelMatrix));
		glDrawArrays(GL_TRIANGLES, 156, 6);

		/*glm::mat4 Window2ModelMatrix = glm::mat4(1.0f);
		Window2ModelMatrix = glm::translate(Window2ModelMatrix, glm::vec3(0.1f, 3.0f, 0.0f));
		Window2ModelMatrix = glm::scale(Window2ModelMatrix, glm::vec3(5.0f, 5.0f, 5.0f));
		Window2ModelMatrix = glm::rotate(Window2ModelMatrix, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		finalMatrix = projectionMatrix * viewMatrix * Window2ModelMatrix;

		matUniformLocation = glGetUniformLocation(program, "mat");
		glUniformMatrix4fv(matUniformLocation, 1, GL_FALSE, glm::value_ptr(finalMatrix));
		glDrawArrays(GL_TRIANGLES, 156, 6);

		glm::mat4 Window3ModelMatrix = glm::mat4(1.0f);
		Window3ModelMatrix = glm::translate(Window3ModelMatrix, glm::vec3(-9.9f, 3.0f, 0.0f));
		Window3ModelMatrix = glm::scale(Window3ModelMatrix, glm::vec3(5.0f, 5.0f, 5.0f));
		Window3ModelMatrix = glm::rotate(Window3ModelMatrix, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		finalMatrix = projectionMatrix * viewMatrix * Window3ModelMatrix;

		matUniformLocation = glGetUniformLocation(program, "mat");
		glUniformMatrix4fv(matUniformLocation, 1, GL_FALSE, glm::value_ptr(finalMatrix));
		glDrawArrays(GL_TRIANGLES, 156, 6);

		glm::mat4 Window4ModelMatrix = glm::mat4(1.0f);
		Window4ModelMatrix = glm::translate(Window4ModelMatrix, glm::vec3(-10.0f, 3.0f, 0.0f));
		Window4ModelMatrix = glm::scale(Window4ModelMatrix, glm::vec3(5.0f, 5.0f, 5.0f));
		Window4ModelMatrix = glm::rotate(Window4ModelMatrix, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		finalMatrix = projectionMatrix * viewMatrix * Window4ModelMatrix;

		matUniformLocation = glGetUniformLocation(program, "mat");
		glUniformMatrix4fv(matUniformLocation, 1, GL_FALSE, glm::value_ptr(finalMatrix));
		glDrawArrays(GL_TRIANGLES, 156, 6);*/

		
		finalMatrix = projectionMatrix * viewMatrix * RoofModelMatrix;

		matUniformLocation = glGetUniformLocation(program, "mat");

		/*glUniformMatrix4fv(matUniformLocation, 1, GL_FALSE, glm::value_ptr(finalMatrix));
		modelUniformLocation = glGetUniformLocation(program, "model");
		glUniformMatrix4fv(modelUniformLocation, 1, GL_FALSE, glm::value_ptr(RoofModelMatrix));
		glDrawArrays(GL_TRIANGLES, 162, 18);*/

		
		finalMatrix = projectionMatrix * viewMatrix * ChairBackModelMatrix;

		matUniformLocation = glGetUniformLocation(program, "mat");
		glUniformMatrix4fv(matUniformLocation, 1, GL_FALSE, glm::value_ptr(finalMatrix));
		modelUniformLocation = glGetUniformLocation(program, "model");
		glUniformMatrix4fv(modelUniformLocation, 1, GL_FALSE, glm::value_ptr(ChairBackModelMatrix));
		glDrawArrays(GL_TRIANGLES, 180, 36);

		
		finalMatrix = projectionMatrix * viewMatrix * ChairBaseModelMatrix;

		matUniformLocation = glGetUniformLocation(program, "mat");
		glUniformMatrix4fv(matUniformLocation, 1, GL_FALSE, glm::value_ptr(finalMatrix));
		modelUniformLocation = glGetUniformLocation(program, "model");
		glUniformMatrix4fv(modelUniformLocation, 1, GL_FALSE, glm::value_ptr(ChairBaseModelMatrix));
		glDrawArrays(GL_TRIANGLES, 180, 36);

		
		finalMatrix = projectionMatrix * viewMatrix * ChairLeg1ModelMatrix;

		matUniformLocation = glGetUniformLocation(program, "mat");
		glUniformMatrix4fv(matUniformLocation, 1, GL_FALSE, glm::value_ptr(finalMatrix));
		modelUniformLocation = glGetUniformLocation(program, "model");
		glUniformMatrix4fv(modelUniformLocation, 1, GL_FALSE, glm::value_ptr(ChairLeg1ModelMatrix));
		glDrawArrays(GL_TRIANGLES, 216, 24);

		
		finalMatrix = projectionMatrix * viewMatrix * ChairLeg2ModelMatrix;

		matUniformLocation = glGetUniformLocation(program, "mat");
		glUniformMatrix4fv(matUniformLocation, 1, GL_FALSE, glm::value_ptr(finalMatrix));
		modelUniformLocation = glGetUniformLocation(program, "model");
		glUniformMatrix4fv(modelUniformLocation, 1, GL_FALSE, glm::value_ptr(ChairLeg2ModelMatrix));
		glDrawArrays(GL_TRIANGLES, 216, 24);

		
		finalMatrix = projectionMatrix * viewMatrix * ChairLeg3ModelMatrix;

		matUniformLocation = glGetUniformLocation(program, "mat");
		glUniformMatrix4fv(matUniformLocation, 1, GL_FALSE, glm::value_ptr(finalMatrix));
		modelUniformLocation = glGetUniformLocation(program, "model");
		glUniformMatrix4fv(modelUniformLocation, 1, GL_FALSE, glm::value_ptr(ChairLeg3ModelMatrix));
		glDrawArrays(GL_TRIANGLES, 216, 24);

		
		finalMatrix = projectionMatrix * viewMatrix * ChairLeg4ModelMatrix;

		matUniformLocation = glGetUniformLocation(program, "mat");
		glUniformMatrix4fv(matUniformLocation, 1, GL_FALSE, glm::value_ptr(finalMatrix));
		modelUniformLocation = glGetUniformLocation(program, "model");
		glUniformMatrix4fv(modelUniformLocation, 1, GL_FALSE, glm::value_ptr(ChairLeg4ModelMatrix));
		glDrawArrays(GL_TRIANGLES, 216, 24);

		
		
		
		glDepthFunc(GL_LEQUAL);
		glUseProgram(skyboxShader);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		GLint skyboxUniformLocationMapping = glGetUniformLocation(skyboxShader, "skybox");
		glUniform1i(skyboxUniformLocationMapping, 2);

		glm::mat4 viewsky = glm::mat4(1.0f);
		glm::mat4 projectionsky = glm::mat4(1.0f);
		// We make the mat4 into a mat3 and then a mat4 again in order to get rid of the last row and column
		// The last row and column affect the translation of the skybox (which we don't want to affect)
		viewsky = glm::mat4(glm::mat3(glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp)));
		projectionsky = glm::perspective(glm::radians(fov), windowWidth / windowHeight, 0.1f, 100.0f);
		GLint projectionSkyUniformLocationMapping = glGetUniformLocation(skyboxShader, "projection");
		glUniformMatrix4fv(projectionSkyUniformLocationMapping, 1, GL_FALSE, glm::value_ptr(projectionsky));
		GLint viewSkyUniformLocationMapping = glGetUniformLocation(skyboxShader, "view");
		glUniformMatrix4fv(viewSkyUniformLocationMapping, 1, GL_FALSE, glm::value_ptr(viewsky));

		// Draws the cubemap as the last object so we can save a bit of performance by discarding all fragments
		// where an object is present (a depth of 1.0f will always fail against any object's depth value)
		glBindVertexArray(skyboxVAO);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		// Switch back to the normal depth function
		glDepthFunc(GL_LESS);

		// "Unuse" the vertex array object
		glBindVertexArray(0);

		// Tell GLFW to swap the screen buffer with the offscreen buffer
		glfwSwapBuffers(window);

		// Tell GLFW to process window events (e.g., input events, window closed events, etc.)
		glfwPollEvents();
	}

	// --- Cleanup ---

	// Make sure to delete the shader program
	glDeleteProgram(program);

	// Delete the VBO that contains our vertices
	glDeleteBuffers(1, &vbo);

	// Delete the vertex array object
	glDeleteVertexArrays(1, &vao);

	// Remember to tell GLFW to clean itself up before exiting the application
	glfwTerminate();

	return 0;
}

void processInput(GLFWwindow* window)
{
	float cameraSpeed = 3.5 * deltaTime;

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		cameraPos += cameraSpeed * cameraFront;
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		cameraPos -= cameraSpeed * cameraFront;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	}
}
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.1f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(front);
}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (fov >= 1.0f && fov <= 45.0f)
		fov -= yoffset;
	if (fov <= 1.0f)
		fov = 1.0f;
	if (fov >= 45.0f)
		fov = 45.0f;
}
/// <summary>
/// Creates a shader program based on the provided file paths for the vertex and fragment shaders.
/// </summary>
/// <param name="vertexShaderFilePath">Vertex shader file path</param>
/// <param name="fragmentShaderFilePath">Fragment shader file path</param>
/// <returns>OpenGL handle to the created shader program</returns>
GLuint CreateShaderProgram(const std::string& vertexShaderFilePath, const std::string& fragmentShaderFilePath)
{
	GLuint vertexShader = CreateShaderFromFile(GL_VERTEX_SHADER, vertexShaderFilePath);
	GLuint fragmentShader = CreateShaderFromFile(GL_FRAGMENT_SHADER, fragmentShaderFilePath);

	GLuint program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);

	glLinkProgram(program);

	glDetachShader(program, vertexShader);
	glDeleteShader(vertexShader);
	glDetachShader(program, fragmentShader);
	glDeleteShader(fragmentShader);

	// Check shader program link status
	GLint linkStatus;
	glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
	if (linkStatus != GL_TRUE) {
		char infoLog[512];
		GLsizei infoLogLen = sizeof(infoLog);
		glGetProgramInfoLog(program, infoLogLen, &infoLogLen, infoLog);
		std::cerr << "program link error: " << infoLog << std::endl;
	}

	return program;
}

/// <summary>
/// Creates a shader based on the provided shader type and the path to the file containing the shader source.
/// </summary>
/// <param name="shaderType">Shader type</param>
/// <param name="shaderFilePath">Path to the file containing the shader source</param>
/// <returns>OpenGL handle to the created shader</returns>
GLuint CreateShaderFromFile(const GLuint& shaderType, const std::string& shaderFilePath)
{
	std::ifstream shaderFile(shaderFilePath);
	if (shaderFile.fail())
	{
		std::cerr << "Unable to open shader file: " << shaderFilePath << std::endl;
		return 0;
	}

	std::string shaderSource;
	std::string temp;
	while (std::getline(shaderFile, temp))
	{
		shaderSource += temp + "\n";
	}
	shaderFile.close();

	return CreateShaderFromSource(shaderType, shaderSource);
}

/// <summary>
/// Creates a shader based on the provided shader type and the string containing the shader source.
/// </summary>
/// <param name="shaderType">Shader type</param>
/// <param name="shaderSource">Shader source string</param>
/// <returns>OpenGL handle to the created shader</returns>
GLuint CreateShaderFromSource(const GLuint& shaderType, const std::string& shaderSource)
{
	GLuint shader = glCreateShader(shaderType);

	const char* shaderSourceCStr = shaderSource.c_str();
	GLint shaderSourceLen = static_cast<GLint>(shaderSource.length());
	glShaderSource(shader, 1, &shaderSourceCStr, &shaderSourceLen);
	glCompileShader(shader);

	// Check compilation status
	GLint compileStatus;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
	if (compileStatus == GL_FALSE)
	{
		char infoLog[512];
		GLsizei infoLogLen = sizeof(infoLog);
		glGetShaderInfoLog(shader, infoLogLen, &infoLogLen, infoLog);
		std::cerr << "shader compilation error: " << infoLog << std::endl;
	}

	return shader;
}

/// <summary>
/// Function for handling the event when the size of the framebuffer changed.
/// </summary>
/// <param name="window">Reference to the window</param>
/// <param name="width">New width</param>
/// <param name="height">New height</param>
void FramebufferSizeChangedCallback(GLFWwindow* window, int width, int height)
{
	// Whenever the size of the framebuffer changed (due to window resizing, etc.),
	// update the dimensions of the region to the new size
	glViewport(0, 0, width, height);
}
