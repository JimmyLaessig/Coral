#ifndef CORAL_SHADERLANGUAGE_BINDING_HPP
#define CORAL_SHADERLANGUAGE_BINDING_HPP

#include <string_view>

namespace Coral::ShaderLanguage
{

template <size_t N>
struct StringLiteral
{ 
	constexpr StringLiteral(const char(&str)[N])
	{ 
		for (size_t i = 0; i < N; ++i)
		{
			value[i] = str[i];
		}
	}

	constexpr operator std::string_view() const
	{ 
		return { value, N - 1 };
	}

	char value[N];
};

enum class Location : uint32_t {};

} // namespace Coral::ShaderLanguage

#endif // !CORAL_SHADERLANGUAGE_BINDING_HPP
