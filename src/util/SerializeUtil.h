#pragma once
#include <cpptoml.h>
#include <glm/glm.hpp>
#include "Logger.h"

#define SAFE_TOML_GET(target, name, type) \
	logger->check(from.get_qualified_as<type>(name).operator bool(), "Data " name " was malformed"); \
	target = *from.get_qualified_as<type>(name);

template<typename T>
class GenericSerializer
{
public:

	static void serialize(const T& what, cpptoml::table& target)
	{

	}

	static void deserialize(T& to, cpptoml::table& from)
	{
		
	}
};

template<typename T> 
void serialize(const T& what, cpptoml::table& target)
{
	GenericSerializer<T>::serialize(what, target);
}

template<typename T>
void deserialize(T& to, const cpptoml::table& from)
{
	GenericSerializer<T>::deserialize(to, from);
}

class SerializeUtil
{
public:

	static void write_to_file(const cpptoml::table& table, const std::string& path)
	{
		std::ofstream file(path);
		file << table;
	}

	static const std::shared_ptr<cpptoml::table> load_file(const std::string& path)
	{
		try
		{
			return cpptoml::parse_file(path);
		}
		catch (const cpptoml::parse_exception& ex)
		{
			logger->fatal("TOML parse error:\n{}", ex.what());
			return nullptr;
		}
	}

	static const std::shared_ptr<cpptoml::table> load_or_write_default(const cpptoml::table& def, const std::string& path)
	{
		std::ifstream infile(path);
		bool read = infile.good();
		if (read)
		{
			return load_file(path);

		}
		else
		{
			write_to_file(def, path);
			// Now it will be able to read it
			return load_file(path);
		}
	}

	// Leave subpath empty for root
	template<typename T>
	static void read_file_to(const std::string& path, T& target, const std::string& sub_path = "")
	{
		auto root = load_file(path);
		
		std::shared_ptr<cpptoml::table> from;

		if (sub_path != "")
		{
			from = root->get_table(sub_path);
		}
		else
		{
			from = root;
		}

		deserialize(target, *from);
	}

	// Optionally allows only overwriting a part of a file if sub_path != ""
	template<typename T>
	static void write_to(const std::string path, const T& source, const std::string& sub_path = "")
	{
		auto serialized = cpptoml::make_table();
		serialize(source, *serialized);

		if (sub_path == "")
		{
			write_to_file(*serialized, path);
		}
		else
		{
			// We need to load the file and change only what's needed
			auto root = load_file(path);

			if (root->get_table(sub_path))
			{
				root->erase(sub_path);
			}

			root->insert(sub_path, serialized);
			
			write_to_file(*root, path);
		}
	}
};