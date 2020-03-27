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
		if constexpr (std::is_same<T, float>::value)
		{
			double x, y;

			SAFE_TOML_GET(x, "x", double);
			SAFE_TOML_GET(y, "y", double);

			target.x = (float)x;
			target.y = (float)y;
		}
		else
		{
			SAFE_TOML_GET(target.x, "x", double);
			SAFE_TOML_GET(target.y, "y", double);
		}
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
		if constexpr (std::is_same<T, float>::value)
		{
			double x, y, z;
			SAFE_TOML_GET(x, "x", double);
			SAFE_TOML_GET(y, "y", double);
			SAFE_TOML_GET(z, "z", double);

			to.x = (float)x;
			to.y = (float)y;
			to.z = (float)z;
		}
		else
		{
			SAFE_TOML_GET(to.x, "x", double);
			SAFE_TOML_GET(to.y, "y", double);
			SAFE_TOML_GET(to.z, "z", double);
		}

	}
};

template<typename T>
class GenericSerializer<glm::tvec4<T>>
{
public:

	static void serialize(const glm::tvec4<T>& what, cpptoml::table& target)
	{
		target.insert("x", what.x);
		target.insert("y", what.y);
		target.insert("z", what.z);
		target.insert("w", what.w);
	}

	static void deserialize(glm::tvec4<T>& to, const cpptoml::table& from)
	{
		if constexpr (std::is_same<T, float>::value)
		{
			double x, y, z, w;
			SAFE_TOML_GET(x, "x", double);
			SAFE_TOML_GET(y, "y", double);
			SAFE_TOML_GET(z, "z", double);
			SAFE_TOML_GET(w, "w", double);

			to.x = (float)x;
			to.y = (float)y;
			to.z = (float)z;
			to.w = (float)w;
		}
		else
		{
			SAFE_TOML_GET(to.x, "x", double);
			SAFE_TOML_GET(to.y, "y", double);
			SAFE_TOML_GET(to.z, "z", double);
			SAFE_TOML_GET(to.w, "w", double);
		}
	}
};
