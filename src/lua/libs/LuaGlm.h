#pragma once
#include "../LuaLib.h"
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

/*
	It should implement a interface very similar to glm, except for:

	- Constructors are called with '.new', ex. ('vec2.new')
	- Many functions are not implemented (rarely used in C++)
	- Some functions only offer certain overloads (rarely used in C++)
	- We offer utility functions 'vec4:to_vec3' '[vec4/vec3]:to_vec2' for easier typecasting
	- We offer utility functions ':unpack()' that return the coordinates to satisfy multi argument functions

	Note that everything is in doubles, but the 'd' suffix is not added!
	Lua uses doubles natively so this makes sense.

	This has a drawback, the coder must be careful when giving values to, and 
	taking them from, lua, as they must be glm::d[x], not glm::[x]. Otherwise
	the lua user will not be able to handle the type!

	As we use a lot of doubles in OSPGL, this will rarely be a problem.
	
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

