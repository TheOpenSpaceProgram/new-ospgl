#pragma once
#include "../LuaLib.h"
#include <glm/glm.hpp>

/*
	It should implement a interface very similar to glm, except for:

	- Constructors are called with '.new', ex. ('vec2.new')
	- Many functions are not implemented (rarely used in C++)
	- Some functions only offer certain overloads (rarely used in C++)

	Note that everything is in doubles, but the 'd' suffix is not added!
	Lua uses doubles natively so this makes sense.

	
*/
class LuaGlm : public LuaLib
{
public:

	static std::string glm_vec2_to_string(const glm::dvec2& v);
	static std::string glm_vec3_to_string(const glm::dvec3& v);
	static std::string glm_vec4_to_string(const glm::dvec4& v);
	static std::string glm_mat3_to_string(const glm::dmat3& v);
	static std::string glm_mat4_to_string(const glm::dmat4& v);

	virtual void load_to(sol::table& table) override;

	LuaGlm();
	~LuaGlm();
};

