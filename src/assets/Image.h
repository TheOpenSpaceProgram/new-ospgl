#pragma once
#include <glad/glad.h>
#include <string>
#include "../util/Logger.h"
#include "../util/SerializeUtil.h"

struct ImageConfig
{
	// 8 for u8 pixels
	// 16 for u16 pixels
	// 32 for float pixels (Warning, HDR conversion!)
	int bitdepth;
	int channels;
	bool upload;
	bool in_memory;
};


// Not only do we load the shader, and make it easily usable
// we also run a preprocessor to allow includes
class Image
{
private:

	uint8_t* u8data;
	uint16_t* u16data;
	float* fdata;

	ImageConfig config;

	int width, height;

public:

	ImageConfig get_config() { return config; }
	inline int get_width() { return width; }
	inline int get_height() { return height; }

	int get_index(int x, int y);

	// All get functions are slightly slow as they
	// have to check correct file format, use
	// get_unsafe if you want that extra speed
	// but no automatic casting!
	template<typename T>
	std::tuple<T, T, T, T> get_rgba(int i);

	template<typename T>
	std::tuple<T, T, T> get_rgb(int i);

	template<typename T>
	std::tuple<T, T> get_ga(int i);

	template<typename T>
	std::tuple<T> get_g(int i);

	template<typename T>
	std::tuple<T, T, T, T> get_rgba_unsafe(int i);

	template<typename T>
	std::tuple<T, T, T> get_rgb_unsafe(int i);

	template<typename T>
	std::tuple<T, T> get_ga_unsafe(int i);

	template<typename T>
	T get_g_unsafe(int i);

	

	GLuint id;

	Image(ImageConfig config, const std::string& path);
	~Image();
};

Image* loadImage(const std::string& path, const std::string& pkg, const cpptoml::table& cfg);



template<>
class GenericSerializer<ImageConfig>
{
public:

	static void serialize(const ImageConfig& what, cpptoml::table& target)
	{
		target.insert("bitdepth", what.bitdepth);
		target.insert("channels", what.channels);
		target.insert("upload", what.upload);
		target.insert("in_memory", what.in_memory);
	}

	static void deserialize(ImageConfig& to, const cpptoml::table& from)
	{
		SAFE_TOML_GET(to.bitdepth, "bitdepth", int);
		SAFE_TOML_GET(to.channels, "channels", int);
		SAFE_TOML_GET(to.upload, "upload", bool);
		SAFE_TOML_GET(to.in_memory, "in_memory", bool);
	}
};

template<typename T>
inline std::tuple<T, T, T, T> Image::get_rgba_unsafe(int i)
{
	T r, g, b, a;
	int j = i * 4;
	if constexpr (std::is_same<T, uint8_t>::value)
	{
		r = u8data[j + 0];
		g = u8data[j + 1];
		b = u8data[j + 2];
		a = u8data[j + 3];
	}
	else if constexpr (std::is_same<T, uint16_t>::value)
	{
		r = u16data[j + 0];
		g = u16data[j + 1];
		b = u16data[j + 2];
		a = u16data[j + 3];
	}
	else if constexpr (std::is_same<T, float>::value)
	{
		r = fdata[j + 0];
		g = fdata[j + 1];
		b = fdata[j + 2];
		a = fdata[j + 3];
	}

	return std::make_tuple(r, g, b, a);
}

template<typename T>
inline std::tuple<T, T, T> Image::get_rgb_unsafe(int i)
{
	T r, g, b;
	int j = i * 3;
	if constexpr (std::is_same<T, uint8_t>::value)
	{
		r = u8data[j + 0];
		g = u8data[j + 1];
		b = u8data[j + 2];
	}
	else if constexpr (std::is_same<T, uint16_t>::value)
	{
		r = u16data[j + 0];
		g = u16data[j + 1];
		b = u16data[j + 2];
	}
	else if constexpr (std::is_same<T, float>::value)
	{
		r = fdata[j + 0];
		g = fdata[j + 1];
		b = fdata[j + 2];
	}

	return std::make_tuple(r, g, b);
}


template<typename T>
inline std::tuple<T, T> Image::get_ga_unsafe(int i)
{
	T r, g;
	int j = i * 2;
	if constexpr (std::is_same<T, uint8_t>::value)
	{
		r = u8data[j + 0];
		g = u8data[j + 1];
	}
	else if constexpr (std::is_same<T, uint16_t>::value)
	{
		r = u16data[j + 0];
		g = u16data[j + 1];
	}
	else if constexpr (std::is_same<T, float>::value)
	{
		r = fdata[j + 0];
		g = fdata[j + 1];
	}

	return std::make_tuple(r, g);
}


template<typename T>
inline T Image::get_g_unsafe(int i)
{
	T r;
	if constexpr (std::is_same<T, uint8_t>::value)
	{
		r = u8data[i];
	}
	else if constexpr (std::is_same<T, uint16_t>::value)
	{
		r = u16data[i];
	}
	else if constexpr (std::is_same<T, float>::value)
	{
		r = fdata[i];
	}

	return r;
}
