#include "Shader.h"
#include "AssetManager.h"

std::string Shader::preprocessor(const std::string & file)
{
	return file;
}

void Shader::use()
{
	logger->check(id != 0, "Shader must be created before use");
	glUseProgram(id);
}

Shader::Shader(const std::string& v, const std::string& f)
{
	std::string vproc = preprocessor(v);
	std::string fproc = preprocessor(f);

	int success = true;
	char infoLog[1024];

	const char* vproc_cstr = vproc.c_str();
	const char* fproc_cstr = fproc.c_str();

	GLuint vs = glCreateShader(GL_VERTEX_SHADER);

	glShaderSource(vs, 1, &vproc_cstr, NULL);
	glCompileShader(vs);

	glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vs, 1024, NULL, infoLog);
		logger->error("Error compiling vertex shader:\n{}", std::string(infoLog));
	};

	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(fs, 1, &fproc_cstr, NULL);
	glCompileShader(fs);

	glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fs, 1024, NULL, infoLog);
		logger->error("Error compiling fragment shader:\n{}", std::string(infoLog));
	};

	id = glCreateProgram();
	glAttachShader(id, vs);
	glAttachShader(id, fs);
	glLinkProgram(id);

	glGetProgramiv(id, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(id, 1024, NULL, infoLog);
		logger->error("Error linking shaders:\n{}", std::string(infoLog));
	};

	glDeleteShader(vs);
	glDeleteShader(fs);

}


Shader::~Shader()
{
}

Shader* loadShader(const std::string& path, const std::string& name, const std::string& pkg, const cpptoml::table& cfg)
{
	
	// We get the path to the vertex shader
	std::string fspath = path.substr(0, path.find_last_of('.')) + ".fs";
	std::string vs = AssetManager::load_string_raw(path);
	std::string fs = AssetManager::load_string_raw(fspath);

	return new Shader(vs, fs);
}
