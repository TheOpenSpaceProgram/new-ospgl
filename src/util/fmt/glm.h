#include <fmt/format.h>
#include <glm/glm.hpp>

template <>
struct fmt::formatter<glm::dvec2>
{
	// Parses format specifications of the form ['f' | 'e'].
	constexpr auto parse(format_parse_context& ctx) 
	{
		return ctx.begin(); 
	}

	template <typename FormatContext>
	auto format(const glm::dvec4& p, FormatContext& ctx)
	{
		// ctx.out() is an output iterator to write to.
		return format_to(
			ctx.out(),
			"({}, {})",
			p.x, p.y);
	}
};

template <>
struct fmt::formatter<glm::dvec3> 
{
	// Parses format specifications of the form ['f' | 'e'].
	constexpr auto parse(format_parse_context& ctx)
	{
		return ctx.begin(); 
	}

	template <typename FormatContext>
	auto format(const glm::dvec3& p, FormatContext& ctx)
	{
		// ctx.out() is an output iterator to write to.
		return format_to(
			ctx.out(),
			"({}, {}, {})",
			p.x, p.y, p.z);
	}
};

template <>
struct fmt::formatter<glm::dvec4>
{
	// Parses format specifications of the form ['f' | 'e'].
	constexpr auto parse(format_parse_context& ctx)
	{
		return ctx.begin();
	}

	template <typename FormatContext>
	auto format(const glm::dvec4& p, FormatContext& ctx)
	{
		// ctx.out() is an output iterator to write to.
		return format_to(
			ctx.out(),
			"({}, {}, {}, {})",
			p.x, p.y, p.z, p.w);
	}
};

template <>
struct fmt::formatter<glm::dquat>
{
	// Parses format specifications of the form ['f' | 'e'].
	constexpr auto parse(format_parse_context& ctx)
	{
		return ctx.begin();
	}

	template <typename FormatContext>
	auto format(const glm::dquat& p, FormatContext& ctx)
	{
		// ctx.out() is an output iterator to write to.
		return format_to(
			ctx.out(),
			"q({}, {}, {}, {})",
			p.x, p.y, p.z, p.w);
	}
};
