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
struct fmt::formatter<glm::dmat4>
{
	// Parses format specifications of the form ['f' | 'e'].
	constexpr auto parse(format_parse_context& ctx)
	{
		return ctx.begin();
	}

	template <typename FormatContext>
	auto format(const glm::dmat4& p, FormatContext& ctx)
	{
		// ctx.out() is an output iterator to write to.
		return format_to(
				ctx.out(),
				"({}, {}, {}, {})\n({}, {}, {}, {})\n({}, {}, {}, {})\n({}, {}, {}, {})",
				p[0].x, p[0].y, p[0].z, p[0].w,
				p[1].x, p[1].y, p[1].z, p[1].w,
				p[2].x, p[2].y, p[2].z, p[2].w,
				p[3].x, p[3].y, p[3].z, p[3].w);
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
