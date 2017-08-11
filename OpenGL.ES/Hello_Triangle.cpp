#include <GLES3/gl31.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <iostream>
#include <vector>
#include <string>

#include <esUtil.h>
#include <esUtil_win.h>

struct user_data {
	GLuint programObject;
};

//Create a shader object, load the shader source, and compile the shader
GLuint load_shader(GLenum type, const char* shader_src)
{
	GLuint shader;
	GLint compiled;

	//Create the shader object
	if ((shader = glCreateShader(type)) == 0) {
		return 0;
	}

	//Load the shader source
	glShaderSource(shader, 1, &shader_src, NULL);

	//Compile the shader
	glCompileShader(shader);

	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

	if (!compiled) {
		GLint info_length = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_length);
		if (info_length > 0) {
			std::vector<GLchar> info_log(info_length);
			glGetShaderInfoLog(shader, info_length, NULL, info_log.data());
			std::cerr << "Error compiling shader:\n" << info_log.data() << '\n';
		}

		glDeleteShader(shader);
		return 0;
	}

	return shader;
}

//Initialize the shader and program object
bool init(ESContext* esContext)
{
	user_data* userdata = static_cast<user_data*>(esContext->userData);
	std::string vertex_shader_src{
		"#version 300 es                         \n"
		"layout(location = 0) in vec4 vPosition; \n"
		"void main()                             \n"
		"{                                       \n"
		"    gl_Position = vPosition;            \n"
		"}                                       \n"
	};

	std::string fragment_shader_src{
		"#version 300 es                           \n"
		"precision mediump float;                  \n"
		"out vec4 fragColor;                       \n"
		"void main()                               \n"
		"{                                         \n"
		"    fragColor = vec4(1.0, 0.0, 0.0, 1.0); \n"
		"}                                         \n"
	};

	GLuint vertex_shader;
	GLuint fragment_shader;
	GLuint program_object;
	GLint linked;

	//Load the vertex/fragment shaders
	vertex_shader = load_shader(GL_VERTEX_SHADER, vertex_shader_src.data());
	fragment_shader = load_shader(GL_FRAGMENT_SHADER, fragment_shader_src.data());

	//Create the program object
	if ((program_object = glCreateProgram()) == 0) {
		return 0;
	}

	glAttachShader(program_object, vertex_shader);
	glAttachShader(program_object, fragment_shader);

	//Link the program
	glLinkProgram(program_object);

	//Check the link status
	glGetProgramiv(program_object, GL_LINK_STATUS, &linked);

	if (!linked) {
		GLint info_length = 0;
		glGetProgramiv(program_object, GL_INFO_LOG_LENGTH, &info_length);
		if (info_length > 0) {
			std::vector<char> info_log(info_length);
			glGetProgramInfoLog(program_object, info_length, NULL, info_log.data());
			std::cerr << "Error linking program:\n" << info_log.data() << '\n';
		}

		glDeleteProgram(program_object);
		return false;
	}

	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	userdata->programObject = program_object;

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

	return true;
}

//Draw a triangle using the shader pair created in Init()
void draw(ESContext* esContext)
{
	user_data* userdata = static_cast<user_data*>(esContext->userData);
	std::vector<GLfloat> vVertices{ 0.0f, 0.5f, 0.0f,
									-0.5f, -0.5f, 0.0f,
									0.5f, -0.5f, 0.0f };

	//Set the viewport
	glViewport(0, 0, esContext->width, esContext->height);

	//Clear the color buffer
	glClear(GL_COLOR_BUFFER_BIT);

	//Use the program object
	glUseProgram(userdata->programObject);

	//Load the vertex data
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vVertices.data());
	glEnableVertexAttribArray(0);

	glDrawArrays(GL_TRIANGLES, 0, 3);
}

void shutdown(ESContext* esContext)
{
	user_data* userdata = static_cast<user_data*>(esContext->userData);
	glDeleteProgram(userdata->programObject);
}

extern "C"
int esMain(ESContext *esContext)
{
	esContext->userData = malloc(sizeof user_data);
	esCreateWindow(esContext, "Hello Triangle", 640, 480, ES_WINDOW_RGB);

	if (!init(esContext)) {
		return GL_FALSE;
	}

	esRegisterShutdownFunc(esContext, shutdown);
	esRegisterDrawFunc(esContext, draw);

	return GL_TRUE;
}
