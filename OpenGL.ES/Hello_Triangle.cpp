#include <GLES3/gl31.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <iostream>
#include <vector>

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
int init(ESContext* esContext)
{
	user_data* userdata = static_cast<user_data*>(esContext->userData);
	std::vector<GLchar> vertex_shader_src{
		"#version 300 es\n",
		"layout(location = 0) in vec4 vPosition;\n",
		"void main()\n",
	};
}
