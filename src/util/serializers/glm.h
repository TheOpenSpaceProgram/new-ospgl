#pragma once
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

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


template<typename T>
class GenericSerializer<glm::tquat<T>>
{
public:

	static void serialize(const glm::tquat<T>& what, cpptoml::table& target)
	{
		target.insert("x", what.x);
		target.insert("y", what.y);
		target.insert("z", what.z);
		target.insert("w", what.w);
	}

	static void deserialize(glm::tquat<T>& to, const cpptoml::table& from)
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

// This is a useful function as matrices are simply a toml array
static std::shared_ptr<cpptoml::array> serialize_matrix(glm::dmat4 mat)
{
	auto out = cpptoml::make_array();
	
	for(int i = 0; i < 4 * 4; i++)
	{
		out->push_back(glm::value_ptr(mat)[i]);
	}
	
	return out;
}

static glm::dmat4 deserialize_matrix(cpptoml::array& array)
{
	glm::dmat4 out;
	// We assume it's valid
	auto array_of_double = *array.get_array_of<double>();

	for(int i = 0; i < 4 * 4; i++)
	{
		glm::value_ptr(out)[i] = array_of_double[i];
	}

	return out;
}