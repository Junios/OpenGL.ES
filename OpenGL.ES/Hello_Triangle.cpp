#include <GLES3/gl31.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <numeric>

#include <esUtil.h>
#include <esUtil_win.h>

struct user_data {
	GLuint program_object;
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
		"uniform vec4 rgb;                       \n"
		"uniform float alpha[4];                 \n"
		"void main()                             \n"
		"{                                       \n"
		"    gl_Position = vPosition;            \n"
		"}                                       \n"
	};

	std::string fragment_shader_src{
		"#version 300 es                           \n"
		"precision mediump float;                  \n"
		"uniform vec4 rgb;                         \n"
		"uniform float alpha[4];                   \n"
		"out vec4 fragColor;                       \n"
		"void main()                               \n"
		"{                                         \n"
		"    vec4 color = vec4(rgb.rgb, alpha[3]);\n"
		"    fragColor = color;                    \n"
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
	else {
		std::unordered_map<EGLint, std::string> program_status{
			{ GL_ACTIVE_ATTRIBUTES, "GL_ACTIVE_ATTRIBUTES" },
			{ GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, "GL_ACTIVE_ATTRIBUTE_MAX_LENGTH" },
			{ GL_ACTIVE_UNIFORM_BLOCKS, "GL_ACTIVE_UNIFORM_BLOCKS" },
			{ GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH, "GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH" },
			{ GL_ACTIVE_UNIFORMS, "GL_ACTIVE_UNIFORMS" },
			{ GL_ACTIVE_UNIFORM_MAX_LENGTH, "GL_ACTIVE_UNIFORM_MAX_LENGTH" },
			{ GL_ATTACHED_SHADERS, "GL_ATTACHED_SHADERS" },
			{ GL_DELETE_STATUS, "GL_DELETE_STATUS" },
			{ GL_INFO_LOG_LENGTH, "GL_INFO_LOG_LENGTH" },
			{ GL_LINK_STATUS, "GL_LINK_STATUS" },
			{ GL_PROGRAM_BINARY_RETRIEVABLE_HINT, "GL_PROGRAM_BINARY_RETRIEVABLE_HINT" },
			{ GL_TRANSFORM_FEEDBACK_BUFFER_MODE, "GL_TRANSFORM_FEEDBACK_BUFFER_MODE" },
			{ GL_TRANSFORM_FEEDBACK_VARYINGS, "GL_TRANSFORM_FEEDBACK_VARYINGS" },
			{ GL_TRANSFORM_FEEDBACK_VARYING_MAX_LENGTH, "GL_TRANSFORM_FEEDBACK_VARYING_MAX_LENGTH" },
			{ GL_VALIDATE_STATUS, "GL_VALIDATE_STATUS" }
		};
		std::cerr << "Program status: " << program_object << '\n';
		std::for_each(program_status.begin(), program_status.end(), [=](decltype(program_status)::reference kv) {
			EGLint status;
			glGetProgramiv(program_object, kv.first, &status);
			std::cerr << kv.second << ": " << status << '\n';
		});
	}

	GLint num_active_uniforms = 0;
	GLint max_length_active_uniform = 0;
	glGetProgramiv(program_object, GL_ACTIVE_UNIFORMS, &num_active_uniforms);
	if (num_active_uniforms > 0) {
		glGetProgramiv(program_object, GL_ACTIVE_UNIFORM_MAX_LENGTH, &max_length_active_uniform);
		std::vector<GLchar> uniform_name(max_length_active_uniform);

		for (int i = 0; i < num_active_uniforms; ++i) {
			GLsizei length = 0;
			GLint size = 0;
			GLenum type;
			glGetActiveUniform(program_object, i, static_cast<GLsizei>(uniform_name.capacity()),
				&length, &size, &type, uniform_name.data());
			if (size == 1) {
				std::cerr << "uniform[" << i << "] = " << uniform_name.data() << '\n';
				std::cerr << "length: " << length << ", size: " << size << ", type: 0x" << std::hex << type << '\n';
			}
			else if (size > 1) {
				std::unordered_map<uint32_t, std::string> uniform_details{
					{ GL_UNIFORM_TYPE, "GL_UNIFORM_TYPE" },
					{ GL_UNIFORM_SIZE, "GL_UNIFORM_SIZE" },
					{ GL_UNIFORM_NAME_LENGTH, "GL_UNIFORM_NAME_LENGTH" },
					{ GL_UNIFORM_BLOCK_INDEX, "GL_UNIFORM_BLOCK_INDEX" },
					{ GL_UNIFORM_OFFSET, "GL_UNIFORM_OFFSET" },
					{ GL_UNIFORM_ARRAY_STRIDE, "GL_UNIFORM_ARRAY_STRIDE" },
					{ GL_UNIFORM_MATRIX_STRIDE, "GL_UNIFORM_MATRIX_STRIDE" },
					{ GL_UNIFORM_IS_ROW_MAJOR, "GL_UNIFORM_IS_ROW_MAJOR" }
				};
				std::cerr << "uniform[" << i << "] = " << uniform_name.data() << '\n';
				std::cerr << "length: " << length << ", size: " << size << ", type: 0x" << std::hex << type << '\n';
				std::for_each(uniform_details.cbegin(), uniform_details.cend(), [=](decltype(uniform_details)::const_reference kv) {
					std::vector<GLuint> indices(size);
					std::vector<GLint> params(size);
					std::vector<const char*> names{
						"alpha[0]", "alpha[1]", "alpha[2]", "alpha[3]"
					};
					glGetUniformIndices(program_object, size, names.data(), indices.data());
					glGetActiveUniformsiv(program_object, size, indices.data(), kv.first, params.data());
					std::cerr << kv.second << ":\n";
					for (int x = 0; x < size; ++x) {
						std::cerr << "[" << x << "]: " << params[x] << '\n';
					}
				});
			}
		}
	}

	//These calls does not delete immediately, but marked them to be deleted
	//when no programs used them
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	userdata->program_object = program_object;

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
	glUseProgram(userdata->program_object);

	//Load the vertex data
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vVertices.data());
	glEnableVertexAttribArray(0);

	glDrawArrays(GL_TRIANGLES, 0, 3);
}

void shutdown(ESContext* esContext)
{
	user_data* userdata = static_cast<user_data*>(esContext->userData);
	glDeleteProgram(userdata->program_object);
}

extern "C"
int esMain(ESContext *esContext)
{
	esContext->userData = malloc(sizeof user_data);
	esCreateWindow(esContext, "Hello Triangle", 640, 480,
		ES_WINDOW_RGB|ES_WINDOW_ALPHA|ES_WINDOW_DEPTH);

	if (!init(esContext)) {
		return GL_FALSE;
	}

	esRegisterShutdownFunc(esContext, shutdown);
	esRegisterDrawFunc(esContext, draw);

	return GL_TRUE;
}

extern "C" void WinLoop(ESContext *esContext);

int main()
{
	ESContext context = { 0, };
	user_data userdata = { 0, };

	context.userData = &userdata;
	context.width = 640;
	context.height = 480;

	const char* title = "Hello Triangle";

	//Create eglNativeWindow
	//For Win32, CreateWindow is used.
	if (WinCreate(&context, title) == GL_FALSE) {
		std::cerr << std::hex << eglGetError() << '\n';
		return 1;
	}

	if ((context.eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY)) == EGL_NO_DISPLAY) {
		std::cerr << std::hex << eglGetError() << '\n';
		return 1;
	}
	EGLint major, minor;
	if (eglInitialize(context.eglDisplay, &major, &minor) == EGL_FALSE) {
		std::cerr << std::hex << eglGetError() << '\n';
		return 1;
	}
	else {
		std::cout << "EGL version:\n";
		std::cout << "Major: " << major << '\n';
		std::cout << "Minor: " << minor << '\n';
	}

	EGLConfig config;
	EGLint num_configs = 0;
	std::vector<EGLint> attributes{
		EGL_RED_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_BLUE_SIZE, 8,
		EGL_ALPHA_SIZE, 8,
		EGL_DEPTH_SIZE, 16,
		EGL_STENCIL_SIZE, EGL_DONT_CARE,
		EGL_SAMPLE_BUFFERS, 1,
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
		EGL_NONE
	};

	if (eglChooseConfig(context.eglDisplay, attributes.data(),
		&config, 1, &num_configs) == EGL_FALSE) {
		std::cerr << std::hex << eglGetError() << '\n';
		return 1;
	}

	if (num_configs < 1) {
		std::cerr << "There is no matched EGL config!\n";
		return 1;
	}
	else {
		EGLint attrib;
		eglGetConfigAttrib(context.eglDisplay, config, EGL_RED_SIZE, &attrib);
		std::cout << "EGL_RED_SIZE: " << attrib << '\n';
		eglGetConfigAttrib(context.eglDisplay, config, EGL_GREEN_SIZE, &attrib);
		std::cout << "EGL_GREEN_SIZE: " << attrib << '\n';
		eglGetConfigAttrib(context.eglDisplay, config, EGL_BLUE_SIZE, &attrib);
		std::cout << "EGL_BLUE_SIZE: " << attrib << '\n';
		eglGetConfigAttrib(context.eglDisplay, config, EGL_ALPHA_SIZE, &attrib);
		std::cout << "EGL_ALPHA_SIZE: " << attrib << '\n';
		eglGetConfigAttrib(context.eglDisplay, config, EGL_DEPTH_SIZE, &attrib);
		std::cout << "EGL_DEPTH_SIZE: " << attrib << '\n';
		eglGetConfigAttrib(context.eglDisplay, config, EGL_STENCIL_SIZE, &attrib);
		std::cout << "EGL_STENCIL_SIZE: " << attrib << '\n';
		eglGetConfigAttrib(context.eglDisplay, config, EGL_RENDERABLE_TYPE, &attrib);
		std::cout << "EGL_RENDERABLE_TYPE: 0x" << std::hex << attrib << '\n';
	}

	if ((context.eglSurface = eglCreateWindowSurface(context.eglDisplay, config,
		context.eglNativeWindow, NULL)) == EGL_NO_SURFACE) {
		EGLint error = eglGetError();
		std::cerr << std::hex << error << '\n';
		switch (error) {
		case EGL_BAD_MATCH:
			std::cerr << "EGL_BAD_MATCH:\n";
			std::cerr << "Check window and EGLConfig attributes to determine compatibility,\n"
				"or verify that the EGLConfig supports rendering to a window\n";
			break;
		case EGL_BAD_CONFIG:
			std::cerr << "EGL_BAD_CONFIG:\n";
			std::cerr << "Verify that provided EGLConfig is valid\n";
			break;
		case EGL_BAD_NATIVE_WINDOW:
			std::cerr << "EGL_BAD_NATIVE_WINDOW:\n";
			std::cerr << "Verify that provided EGLNativeWindow is valid\n";
			break;
		case EGL_BAD_ALLOC:
			std::cerr << "EGL_BAD_ALLOC:\n";
			std::cerr << "Not enough resources available; handle and recover\n";
			break;
		}
		return 1;
	}
	std::vector<EGLint> context_attributes{ EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE	};
	if ((context.eglContext = eglCreateContext(context.eglDisplay, config,
		EGL_NO_CONTEXT, context_attributes.data())) == EGL_NO_CONTEXT) {
		std::cerr << std::hex << eglGetError() << '\n';
		return 1;
	}

	if (eglMakeCurrent(context.eglDisplay, context.eglSurface, context.eglSurface,
		context.eglContext) == EGL_FALSE) {
		std::cerr << std::hex << eglGetError() << '\n';
		return 1;
	}

	if (!init(&context)) {
		std::cerr << "Fail to init!\n";
		return 1;
	}

	esRegisterDrawFunc(&context, draw);
	esRegisterShutdownFunc(&context, shutdown);

	WinLoop(&context);

	if (context.shutdownFunc != NULL)
	{
		context.shutdownFunc(&context);
	}

	return 0;
}
