#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Macro for indexing vertex buffer
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

using namespace std;

// Vertex Shader (for convenience, it is defined in the main here, but we will be using text files for shaders in future)
// Note: Input to this shader is the vertex positions that we specified for the triangle. 
// Note: gl_Position is a special built-in variable that is supposed to contain the vertex position (in X, Y, Z, W)
// Since our triangle vertices were specified as vec3, we just set W to 1.0.

static const char* pVS = "                                                    \n\
#version 330                                                                  \n\
                                                                              \n\
in vec3 vPosition;															  \n\
in vec4 vColor;																  \n\
out vec4 color;																 \n\
uniform mat4 mat;														\n\
                                                                              \n\
                                                                               \n\
void main()                                                                     \n\
{                                                                                \n\
    gl_Position = mat * vec4(vPosition.x/2, vPosition.y/2, vPosition.z/2, 1.0);  \n\
	color = vColor;							\n\
}";

// Fragment Shader
// Note: no input in this shader, it just outputs the colour of all fragments, in this case set to red (format: R, G, B, A).
static const char* pFS = "                                              \n\
#version 330                                                            \n\
                                                                        \n\
in vec4 color;															\n\
out vec4 FragColor;                                                      \n\
                                                                          \n\
void main()                                                               \n\
{                                                                          \n\
	FragColor = color;									 \n\
}";

GLuint POS_ID = 0;
float matrix[] = { 1, 0, 0, 0,
0, 1, 0, 0,
0, 0, 1, 0,
0, 0, 0, 1 };
glm::mat4 rotMat = glm::make_mat4(matrix);
glm::mat4 transMat = glm::make_mat4(matrix);
glm::mat4 scalMat = glm::make_mat4(matrix);
glm::mat4 mat;
glm::mat4 rotMat2 = glm::make_mat4(matrix);
glm::mat4 transMat2 = glm::make_mat4(matrix);
glm::mat4 scalMat2 = glm::make_mat4(matrix);
glm::mat4 mat2;

// Shader Functions- click on + to expand
#pragma region SHADER_FUNCTIONS
static void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
{
	// create a shader object
    GLuint ShaderObj = glCreateShader(ShaderType);

    if (ShaderObj == 0) {
        fprintf(stderr, "Error creating shader type %d\n", ShaderType);
        exit(0);
    }
	// Bind the source code to the shader, this happens before compilation
	glShaderSource(ShaderObj, 1, (const GLchar**)&pShaderText, NULL);
	// compile the shader and check for errors
    glCompileShader(ShaderObj);
    GLint success;
	// check for shader related errors using glGetShaderiv
    glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar InfoLog[1024];
        glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
        fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
        exit(1);
    }
	// Attach the compiled shader object to the program object
    glAttachShader(ShaderProgram, ShaderObj);
}

GLuint CompileShaders()
{
	//Start the process of setting up our shaders by creating a program ID
	//Note: we will link all the shaders together into this ID
    GLuint shaderProgramID = glCreateProgram();
    if (shaderProgramID == 0) {
        fprintf(stderr, "Error creating shader program\n");
        exit(1);
    }

	// Create two shader objects, one for the vertex, and one for the fragment shader
    AddShader(shaderProgramID, pVS, GL_VERTEX_SHADER);
    AddShader(shaderProgramID, pFS, GL_FRAGMENT_SHADER);

    GLint Success = 0;
    GLchar ErrorLog[1024] = { 0 };


	// After compiling all shader objects and attaching them to the program, we can finally link it
    glLinkProgram(shaderProgramID);
	// check for program related errors using glGetProgramiv
    glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &Success);
	if (Success == 0) {
		glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
        exit(1);
	}

	// program has been successfully linked but needs to be validated to check whether the program can execute given the current pipeline state
    glValidateProgram(shaderProgramID);
	// check for program related errors using glGetProgramiv
    glGetProgramiv(shaderProgramID, GL_VALIDATE_STATUS, &Success);
    if (!Success) {
        glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
        fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
        exit(1);
    }
	// Finally, use the linked shader program
	// Note: this program will stay in effect for all draw calls until you replace it with another or explicitly disable its use
    glUseProgram(shaderProgramID);
	return shaderProgramID;
}
#pragma endregion SHADER_FUNCTIONS

// VBO Functions - click on + to expand
#pragma region VBO_FUNCTIONS
GLuint generateObjectBuffer(GLfloat vertices[], GLfloat colors[]) {
	GLuint numVertices = 6;
	// Genderate 1 generic buffer object, called VBO
	GLuint VBO;
 	glGenBuffers(1, &VBO);
	// In OpenGL, we bind (make active) the handle to a target name and then execute commands on that target
	// Buffer will contain an array of vertices 
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	// After binding, we now fill our object with data, everything in "Vertices" goes to the GPU
	glBufferData(GL_ARRAY_BUFFER, numVertices*7*sizeof(GLfloat), NULL, GL_STATIC_DRAW);
	// if you have more data besides vertices (e.g., vertex colours or normals), use glBufferSubData to tell the buffer when the vertices array ends and when the colors start
	glBufferSubData (GL_ARRAY_BUFFER, 0, numVertices*3*sizeof(GLfloat), vertices);
	glBufferSubData (GL_ARRAY_BUFFER, numVertices*3*sizeof(GLfloat), numVertices*4*sizeof(GLfloat), colors);
return VBO;
}

void linkCurrentBuffertoShader(GLuint shaderProgramID){
	GLuint numVertices = 6;
	// find the location of the variables that we will be using in the shader program
	GLuint positionID = glGetAttribLocation(shaderProgramID, "vPosition");
	GLuint colorID = glGetAttribLocation(shaderProgramID, "vColor");
	POS_ID = glGetUniformLocation(shaderProgramID, "mat");
	// Have to enable this
	glEnableVertexAttribArray(positionID);
	// Tell it where to find the position data in the currently active buffer (at index positionID)
    glVertexAttribPointer(positionID, 3, GL_FLOAT, GL_FALSE, 0, 0);
	// Similarly, for the color data.
	glEnableVertexAttribArray(colorID);
	glVertexAttribPointer(colorID, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(numVertices*3*sizeof(GLfloat)));
}
#pragma endregion VBO_FUNCTIONS


void display(){

	glClear(GL_COLOR_BUFFER_BIT);

	mat = rotMat * transMat * scalMat;
	glUniformMatrix4fv(POS_ID, 1, GL_TRUE, &mat[0][0]);

	// NB: Make the call to draw the geometry in the currently activated vertex buffer. This is where the GPU starts to work!
	glDrawArrays(GL_TRIANGLES, 0, 3);

	mat2 = rotMat2 * transMat2 * scalMat2;
	glUniformMatrix4fv(POS_ID, 1, GL_TRUE, &mat2[0][0]);

	// NB: Make the call to draw the geometry in the currently activated vertex buffer. This is where the GPU starts to work!
	glDrawArrays(GL_TRIANGLES, 3, 3);
    glutSwapBuffers();
}


void init()
{
	// Create 24 vertices that make up a cube that fits on the viewport 
	GLfloat vertices[] = {-1.0f, -1.0f, 0.0f, //triangle 1
			0.0f, -1.0f, 0.0f,
			-0.5f, 1.0f, 0.0f, //triangle 1
			0.0f, -1.0f, 0.0f, //triangle 2
			1.0f, -1.0f, 0.0f,
			0.5f, 1.0f, 0.0f}; //triangle 2
	// Create a color array that identfies the colors of each vertex (format R, G, B, A)
	GLfloat colors[] = {1.0f, 0.0f, 0.0f, 1.0f,
			0.0f, 1.0f, 0.0f, 1.0f,
			0.0f, 0.0f, 1.0f, 1.0f,
			1.0f, 0.0f, 0.0f, 1.0f,
			0.0f, 1.0f, 0.0f, 1.0f,
			0.0f, 0.0f, 1.0f, 1.0f };
	// Set up the shaders
	GLuint shaderProgramID = CompileShaders();
	// Put the vertices and colors into a vertex buffer object
	generateObjectBuffer(vertices, colors);
	// Link the current buffer to the shader
	linkCurrentBuffertoShader(shaderProgramID);	
}

void processSpecialKeys(int key, int x, int y) {

	switch (key) {
		case GLUT_KEY_F1:
			//rotate x
			rotMat = glm::rotate(rotMat, 0.25f, glm::vec3(0.1f, 0.0f, 0.0f)); break;
		case GLUT_KEY_F2:
			//rotate y
			rotMat = glm::rotate(rotMat, 0.25f, glm::vec3(0.0f, 0.1f, 0.0f)); break;
		case GLUT_KEY_F3:
			//rotate z
			rotMat = glm::rotate(rotMat, 0.25f, glm::vec3(0.0f, 0.0f, 0.1f)); break;
		case GLUT_KEY_F4:
			//translate x
			transMat[0][3] = transMat[0][3] + 0.1f; break;
		case GLUT_KEY_F5:
			//translate y
			transMat[1][3] = transMat[1][3] + 0.1f; break;
		case GLUT_KEY_F6:
			//translate z
			transMat[2][3] = transMat[2][3] + 0.1f; break;
		case GLUT_KEY_F7:
			//uniform scaling
			scalMat = glm::scale(scalMat, glm::vec3(0.5f)); break;
		case GLUT_KEY_F8:
			//non-uniform scaling 
			scalMat = glm::scale(scalMat, glm::vec3(0.5f, 1.25f, 0.75f)); break;
		case GLUT_KEY_F9:
			//Combined
			rotMat = glm::rotate(rotMat, 0.9f, glm::vec3(0.0f, 0.0f, 0.1f));
			transMat[1][3] = transMat[1][3] - 0.1f;
			scalMat = glm::scale(scalMat, glm::vec3(0.9f)); break;
		case GLUT_KEY_F10:
			//Multi
			rotMat = glm::rotate(rotMat, 1.5f, glm::vec3(0.5f, 0.0f, 0.0f));
			transMat[0][3] = transMat[0][3] - 0.1f;
			scalMat = glm::scale(scalMat, glm::vec3(1.2f));
			rotMat2 = glm::rotate(rotMat2, 1.5f, glm::vec3(0.0f, 0.0f, 1.5f));
			transMat2[1][3] = transMat2[1][3] + 0.1f;
			scalMat2 = glm::scale(scalMat2, glm::vec3(0.75f)); break;
		case GLUT_KEY_F11:
			exit(0);
	}

	display();
}

int main(int argc, char** argv){

	// Set up the window
	glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB);
    glutInitWindowSize(800, 800);
	glutCreateWindow("Assignment #2");
	// Tell glut where the display function is
	glutDisplayFunc(display);

	glutSpecialFunc(processSpecialKeys);

	 // A call to glewInit() must be done after glut is initialized!
    GLenum res = glewInit();
	// Check for any errors
    if (res != GLEW_OK) {
      fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
      return 1;
    }
	// Set up your objects and shaders
	init();
	// Begin infinite event loop
	glutMainLoop();
    return 0;
}