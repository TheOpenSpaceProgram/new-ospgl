#include "Shader.h"
#include "AssetManager.h"
#include <renderer/Renderer.h>
#include <string>

std::string Shader::preprocessor(const std::string& file)
{
	std::unordered_map<std::string, std::string> defines;
	
	std::stringstream ss(file);
	
	std::string line;

	std::string final_file;

	size_t line_num = 1;
	while(std::getline(ss, line, '\n'))
	{
		// Remove whitespace, the shader compiler does not care about it
		int first_non_white = line.find_first_not_of(" \t");
		if(first_non_white != std::string::npos)
		{
			line = line.substr(line.find_first_not_of(" \t"));
		}

		std::string include_str = "#include ";

		if(line.size() > include_str.size() && line.substr(0, include_str.size()) == include_str)
		{
			// #include "A" or #include <A>, we don't allow substitutions here
			std::string token = line.substr(include_str.size());	
			bool relative = false;

			if(token[0] == '"')
			{
				relative = true;
				token = token.substr(1, token.find_last_of('"') - 1);
			}
			else if(token[0] == '<')
			{
				relative = false;
				token = token.substr(1, token.find_last_of('>') - 1);
			}
			else
			{
				logger->warn("Unknown #include enclosing character ({})", token[0]);
				continue;
			}

			std::string path;

			if(relative)
			{
				// We add the package later for simplicity
				path = get_asset_name().substr(0, get_asset_name().find_last_of('/') + 1);

				// Starting path is the path of the shader, any ../ goes backwards one
				// subdirectory. It's forbidden to go lower than the package root directory,
				// so loading from another package using relative paths is NOT POSSIBLE
				std::string spath = "";
				for(int j = 0; j < token.size(); j++)
				{
					if(token[j] == '/' || j == token.size() - 1)
					{
						spath += token[j];

						if(spath == "../")
						{
							int pos = path.find_last_of('/');
							if(pos == std::string::npos)
							{
								logger->error("Error while preprocessing shader relative include '{}', invalid path", token);
							}
							else
							{
								path = path.substr(0, pos);
								path += '/';
							}	
						}
						else
						{
							path += spath;
						}

						spath = "";
					}
					else
					{
						spath += token[j];
					}

				}		

				path = get_asset_pkg() + ":" + path;
			}
			else
			{
				if(token == "quality_defines")
				{
					// Add the config defines from RenderConfig
					final_file += osp->renderer->quality.get_shader_defines();
					final_file += "\n";
					final_file += "\n#line " + std::to_string(line_num - 1);
					final_file += "\n";
					continue;
				}
				else
				{
					auto[pk, nm] = osp->assets->get_package_and_name(token, get_asset_pkg());
					path = pk + ":" + nm;
				}
			}

			if(!AssetManager::file_exists(osp->assets->resolve_path(path)))
			{
				logger->error("Could not include: {}", path);
			}
			else
			{
				std::string sfile = osp->assets->load_string(path);
				// Postprocess the file too
				sfile = preprocessor(sfile);
				final_file += sfile;
				// Add line directive
				final_file += "\n#line " + std::to_string(line_num - 1);
				final_file += "\n";

			}
		}
		else
		{
			// Parse substitutions
			final_file += line;
			final_file += "\n";
		}


		line_num++;
	}

	return final_file;
}

void Shader::use() const
{
	logger->check(id != 0, "Shader must be created before use");
	glUseProgram(id);
}

Shader::Shader(const std::string& v, const std::string& f, ASSET_INFO) : Asset(ASSET_INFO_P)
{
	std::string vproc = preprocessor(v);
	std::string fproc = preprocessor(f);

	int success = true;
	char infoLog[1024];

	const char* vproc_cstr = vproc.c_str();
	const char* fproc_cstr = fproc.c_str();

	GLuint vs = glCreateShader(GL_VERTEX_SHADER);

	glShaderSource(vs, 1, &vproc_cstr, nullptr);
	glCompileShader(vs);

	glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vs, 1024, nullptr, infoLog);
		logger->error("Error compiling vertex shader:\n{}", std::string(infoLog));
	}

	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(fs, 1, &fproc_cstr, nullptr);
	glCompileShader(fs);

	glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fs, 1024, nullptr, infoLog);
		logger->error("Error compiling fragment shader:\n{}", std::string(infoLog));
	}

	id = glCreateProgram();
	glAttachShader(id, vs);
	glAttachShader(id, fs);
	glLinkProgram(id);

	glGetProgramiv(id, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(id, 1024, nullptr, infoLog);
		logger->error("Error linking shaders:\n{}", std::string(infoLog));
	}

	glDeleteShader(vs);
	glDeleteShader(fs);

}


Shader::~Shader() = default;

Shader* load_shader(const std::string& path, const std::string& name, const std::string& pkg, const cpptoml::table& cfg)
{
	
	// We get the path to the vertex shader
	std::string fspath = path.substr(0, path.find_last_of('.')) + ".fs";
	std::string vs = AssetManager::load_string_raw(path);
	std::string fs = AssetManager::load_string_raw(fspath);

	return new Shader(vs, fs, ASSET_INFO_P);
	
}
