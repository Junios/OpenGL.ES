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

void print_display_config(EGLDisplay const& dpy, EGLConfig const& config)
{
	std::unordered_map<EGLenum, std::string> display_attributes{
		{ EGL_RED_SIZE, "EGL_RED_SIZE" },
		{ EGL_GREEN_SIZE, "EGL_GREEN_SIZE" },
		{ EGL_BLUE_SIZE, "EGL_BLUE_SIZE" },
		{ EGL_ALPHA_SIZE, "EGL_ALPHA_SIZE" },
		{ EGL_DEPTH_SIZE, "EGL_DEPTH_SIZE" },
		{ EGL_STENCIL_SIZE, "EGL_STENCIL_SIZE" },
		{ EGL_RENDERABLE_TYPE, "EGL_RENDERABLE_TYPE" },
		{ EGL_SAMPLE_BUFFERS, "EGL_SAMPLE_BUFFERS" }
	};

	std::cout << "* Display config:\n";
	std::cout << std::hex << "> Display: 0x" << static_cast<void*>(dpy)
		<< ", Config: 0x" << static_cast<void*>(config) << '\n';
	std::for_each(display_attributes.cbegin(), display_attributes.cend(), [&](auto const& kv) {
		EGLint attrib;
		eglGetConfigAttrib(dpy, config, kv.first, &attrib);
		std::cout << "> " << kv.second << ":\t0x" << std::hex << attrib << "(" << std::dec << attrib << ")\n";
	});
}

void print_egl_error(EGLint error)
{
	std::cerr << "* EGL error: 0x" << std::hex << error << "(" << std::dec << error << ")\n";
	switch (error) {
	case EGL_BAD_MATCH:
		std::cerr << "> EGL_BAD_MATCH:\n";
		std::cerr << "> Check window and EGLConfig attributes to determine compatibility,\n"
			"> or verify that the EGLConfig supports rendering to a window\n";
		break;
	case EGL_BAD_CONFIG:
		std::cerr << "> EGL_BAD_CONFIG:\n";
		std::cerr << "> Verify that provided EGLConfig is valid\n";
		break;
	case EGL_BAD_NATIVE_WINDOW:
		std::cerr << "> EGL_BAD_NATIVE_WINDOW:\n";
		std::cerr << "> Verify that provided EGLNativeWindow is valid\n";
		break;
	case EGL_BAD_ALLOC:
		std::cerr << "> EGL_BAD_ALLOC:\n";
		std::cerr << "> Not enough resources available; handle and recover\n";
		break;
	}
}

void print_program_status(GLuint program)
{
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
		{ GL_VALIDATE_STATUS, "GL_VALIDATE_STATUS" },
		{ GL_PROGRAM_BINARY_LENGTH, "GL_PROGRAM_BINARY_LENGTH" }
	};

	std::cout << "* Program status: 0x" << std::hex << program
		<< "(" << std::dec << program << ")\n";

	GLint num_program_binary_formats = 0;
	glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS, &num_program_binary_formats);
	std::cout << "> GL_NUM_PROGRAM_BINARY_FORMATS: " << num_program_binary_formats << '\n';
	if (num_program_binary_formats > 0) {
		std::vector<GLint> binary_formats(num_program_binary_formats);
		glGetIntegerv(GL_PROGRAM_BINARY_FORMATS, binary_formats.data());
		std::cout << "> GL_PROGRAM_BINARY_FORMATS: ";
		for (auto format : binary_formats) {
			std::cout << format << ", ";
		}
		std::cout << '\n';
	}

	std::for_each(program_status.begin(), program_status.end(), [=](auto const& kv) {
		EGLint status;
		glGetProgramiv(program, kv.first, &status);
		std::cout << "> " << kv.second << ": 0x" << std::hex << status
			<< "(" << std::dec << status << ")\n";
	});
}

const std::string& get_gl_type_string(GLenum type)
{
	static const std::string UNKNOWN{ "UNKNOWN" };
	static std::unordered_map<GLenum, std::string> gl_types{
		{ GL_BYTE, "GL_BYTE" },
		{ GL_UNSIGNED_BYTE, "GL_UNSIGNED_BYTE" },
		{ GL_SHORT, "GL_SHORT" },
		{ GL_UNSIGNED_SHORT, "GL_UNSIGNED_SHORT" },
		{ GL_INT, "GL_INT" },
		{ GL_UNSIGNED_INT, "GL_UNSIGNED_INT" },
		{ GL_FLOAT, "GL_FLOAT" },
		{ GL_FIXED, "GL_FIXED" },
		{ GL_FLOAT_VEC2, "GL_FLOAT_VEC2" },
		{ GL_FLOAT_VEC3, "GL_FLOAT_VEC3" },
		{ GL_FLOAT_VEC4, "GL_FLOAT_VEC4" },
		{ GL_INT_VEC2, "GL_INT_VEC2" },
		{ GL_INT_VEC3, "GL_INT_VEC3" },
		{ GL_INT_VEC4, "GL_INT_VEC4" },
		{ GL_BOOL, "GL_BOOL" },
		{ GL_BOOL_VEC2, "GL_BOOL_VEC2" },
		{ GL_BOOL_VEC3, "GL_BOOL_VEC3" },
		{ GL_BOOL_VEC4, "GL_BOOL_VEC4" },
		{ GL_FLOAT_MAT2, "GL_FLOAT_MAT2" },
		{ GL_FLOAT_MAT3, "GL_FLOAT_MAT3" },
		{ GL_FLOAT_MAT4, "GL_FLOAT_MAT4" },
		{ GL_SAMPLER_2D, "GL_SAMPLER_2D" },
		{ GL_SAMPLER_CUBE, "GL_SAMPLER_CUBE" },
	};

	auto search = gl_types.find(type);
	if (search == gl_types.end()) { return UNKNOWN; }

	return search->second;
}

void print_uniform_info(GLuint program)
{
	std::cout << "* Uniform information:\n";
	GLint num_active_uniforms = 0;
	GLint max_length_active_uniform = 0;
	glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &num_active_uniforms);
	if (num_active_uniforms > 0) {
		glGetProgramiv(program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &max_length_active_uniform);
		std::vector<GLchar> uniform_name(max_length_active_uniform);

		for (int i = 0; i < num_active_uniforms; ++i) {
			GLsizei length = 0;
			GLsizei size = 0;
			GLenum type;
			glGetActiveUniform(program, i, static_cast<GLsizei>(uniform_name.capacity()),
				&length, &size, &type, uniform_name.data());
			std::cout << "> uniform[" << i << "]\n";
			std::cout << "  .name = " << uniform_name.data() << " .length(of name) = " << length << '\n';
			std::cout << "  .type = " << get_gl_type_string(type) << "(0x" << std::hex << type
				<< ")  .size = " << std::dec << size << '\n';
		}
	}
}

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
		"layout (std140) uniform LightBlock {    \n"
		"    vec3 lightDirection;                \n"
		"    vec4 lightPosition;                 \n"
		"};                                      \n"
		"void main()                             \n"
		"{                                       \n"
		"    vec3 ld = lightDirection;           \n"
		"    vec4 lp = lightPosition;            \n"
		"    gl_Position = vPosition;            \n"
		"}                                       \n"
	};

	std::string fragment_shader_src{
		"#version 300 es                           \n"
		"precision highp float;                    \n"
		"uniform vec4 rgb;                         \n"
		"uniform float alpha[4];                   \n"
		"out vec4 fragColor;                       \n"
		"void main()                               \n"
		"{                                         \n"
		"    fragColor = vec4(rgb.xyz, alpha[3]);  \n"
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

	GLint loc = glGetUniformLocation(userdata->program_object, "rgb");
	glUniform4f(loc, 1.0f, 0.0f, 0.0f, 1.0f);
	loc = glGetUniformLocation(userdata->program_object, "alpha[3]");
	glUniform1f(loc, 0.1f);

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
	if (!esCreateWindow(esContext, "Hello Triangle", 640, 480,
		ES_WINDOW_RGB | ES_WINDOW_ALPHA | ES_WINDOW_DEPTH)) {
		print_egl_error(eglGetError());
	}

	if (!init(esContext)) {
		return GL_FALSE;
	}

	esRegisterShutdownFunc(esContext, shutdown);
	esRegisterDrawFunc(esContext, draw);

	print_display_config(esContext->eglDisplay, esContext->eglConfig);
	print_program_status(static_cast<user_data*>(esContext->userData)->program_object);
	print_uniform_info(static_cast<user_data*>(esContext->userData)->program_object);
	return GL_TRUE;
}

extern "C" void WinLoop(ESContext *esContext);
int debugMain()
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
		//EGL_SAMPLE_BUFFERS, 1,
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
		print_display_config(context.eglDisplay, config);
	}

	if ((context.eglSurface = eglCreateWindowSurface(context.eglDisplay, config,
		context.eglNativeWindow, NULL)) == EGL_NO_SURFACE) {
		print_egl_error(eglGetError());
		return 1;
	}

	std::vector<EGLint> context_attributes{ EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE };
	if ((context.eglContext = eglCreateContext(context.eglDisplay, config,
		EGL_NO_CONTEXT, context_attributes.data())) == EGL_NO_CONTEXT) {
		std::cerr << std::hex << eglGetError() << '\n';
		return 1;
	}

	//eglMakeCurrent may show some information about current hardware platform
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
