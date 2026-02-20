#ifndef CORAL_SHADERLANGUAGE_COMPILER_HPP
#define CORAL_SHADERLANGUAGE_COMPILER_HPP

#include <Coral/ShaderLanguage/ShaderModule.hpp>

#include <expected>
#include <map>
#include <string_view>

namespace Coral::ShaderLanguage
{

class Compiler
{
public:

	struct Result
	{
		std::string vertexShader;
		std::string fragmentShader;
	};

	struct Error
	{
		std::string message;
	};

	/// Compile the shader program
	virtual std::expected<Result, Error> Compile(const ShaderModule& vertexShader, const ShaderModule& fragmentShader) = 0;
};

} // namespace Coral

#endif // !CORAL_SHADERLANGUAGE_COMPILER_HPP
