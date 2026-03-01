#ifndef CORAL_SHADERLANGUAGE_COMPILER_HPP
#define CORAL_SHADERLANGUAGE_COMPILER_HPP

#include <Coral/ShaderLanguage/ShaderGraph.hpp>

#include <expected>
#include <map>
#include <string_view>

namespace Coral::ShaderLanguage
{

enum class ShaderStage
{
	VERTEX,
	FRAGMENT
};

class Compiler
{
public:

	struct Result
	{
		std::string shaderCode;
	};

	struct Error
	{
		std::string message;
	};

	/// Compile the shader program
	virtual std::expected<Result, Error> Compile(const ShaderGraph& shaderGraph, ShaderStage stage) = 0;
};

} // namespace Coral

#endif // !CORAL_SHADERLANGUAGE_COMPILER_HPP
