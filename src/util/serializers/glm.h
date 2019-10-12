#pragma once
#include <glm/glm.hpp>
#include "../SerializeUtil.h"

template<typename T>
class GenericSerializer<glm::tvec2<T>>
{
public:

	static void serialize(const glm::tvec2<T>& what, cpptoml::table& target)
	{
		target.insert("x", what.x);
		target.insert("y", what.y);
	}

	static void deserialize(glm::tvec2<T>& target, const cpptoml::table& from)
	{
		SAFE_TOML_GET(target.x, "x", T);
		SAFE_TOML_GET(target.y, "y", T);
	}
};


template<typename T>
class GenericSerializer<glm::tvec3<T>>
{
public:

	static void serialize(const glm::tvec3<T>& what, cpptoml::table& target)
	{
		target.insert("x", what.x);
		target.insert("y", what.y);
		target.insert("z", what.z);
	}

	static void deserialize(glm::tvec3<T>& to, const cpptoml::table& from)
	{
		SAFE_TOML_GET(to.x, "x", T);
		SAFE_TOML_GET(to.y, "y", T);
		SAFE_TOML_GET(to.z, "z", T);
	}
};
