#pragma once
#include <glad/glad.h>
#include <string>
#include "../util/Logger.h"

#include <glm/gtc/type_ptr.hpp>

// Not only do we load the shader, and make it easily usable
// we also run a preprocessor to allow includes
class Shader
{
private:

	// Run basic preprocessing on shader
	std::string preprocessor(const std::string& file);



public:

	GLuint id;

	void use();

	inline void setBool(const std::string &name, bool value) const
	{
		glUniform1i(glGetUniformLocation(id, name.c_str()), (int)value);
	}
	inline void setInt(const std::string &name, int value) const
	{
		glUniform1i(glGetUniformLocation(id, name.c_str()), value);
	}

	inline void setFloat(const std::string &name, float value) const
	{
		glUniform1f(glGetUniformLocation(id, name.c_str()), value);
	}

	inline void setVec3(const std::string& name, glm::vec3 value) const
	{
		glUniform3f(glGetUniformLocation(id, name.c_str()), value.x, value.y, value.z);
	}

	inline void setVec4(const std::string& name, glm::vec4 value) const
	{
		glUniform4f(glGetUniformLocation(id, name.c_str()), value.x, value.y, value.z, value.w);
	}

	inline void setMat4(const std::string& name, glm::mat4 value) const
	{
		glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
	}

	Shader(const std::string& vertexData, const std::string& fragmentData);
	~Shader();
};

Shader* loadShader(const std::string& path, const std::string& pkg);


