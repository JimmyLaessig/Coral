#include <Coral/HeaderGenerator.h>

#include <regex>
#include <sstream>
#include <iostream>

namespace
{

void
insertMultiLineString(const std::string& input, std::stringstream& output, uint32_t indentation)
{
	std::stringstream ss(input);
	output << std::endl;
	std::string whitespaces((indentation + 1) * 4, ' ');

	for (std::string line; std::getline(ss, line);)
	{
		// Convert back slashes to forward slashes
		std::replace(line.begin(), line.end(), '\\', '/');

		// Properly escape double quotes
		line = std::regex_replace(line, std::regex("\""), "\\\"");

		output << whitespaces << "\"" << line << "\\n\"" << std::endl;
	}
	
	output.seekp(-1, output.cur);
	output << ";" << std::endl;
}


void
insertMultiLineUInt32Array(const std::vector<uint32_t>& input, std::stringstream& output, uint32_t indentation)
{
	output << " { " << std::endl;
	std::string whitespace((indentation + 1) * 4, ' ');

	std::string line = whitespace;
	for (uint32_t v : input)
	{
		std::string element = std::to_string(v) + "u, ";
		if (line.length() + element.length() > 100)
		{
			output << line << std::endl;
			line = whitespace;
		}
		line += element;
	}
	output << line;
	output.seekp(-2, output.cur);
	output << " };" << std::endl;
}

} // namespace

namespace Coral
{

std::string
buildShaderHeader(std::string_view className,
				  const std::optional<std::string_view>& classNamespace,
				  const Coral::ShaderModuleCompileInfo& compileInfo,
				  const Coral::CompiledShaderModule& shaderModule)
{
	std::stringstream ss;

	ss << "#include <array>" << std::endl;
	ss << "#include <cstdint>" << std::endl;
	ss << std::endl;

	if (classNamespace)
	{
		ss << "namespace " << *classNamespace << std::endl << "{" << std::endl << std::endl;
	}
	ss << "class " << className << std::endl << "{" << std::endl;
	ss << "public:" << std::endl;
	ss << std::endl;
	ss << "    constexpr static auto entryPoint = \"" << compileInfo.entryPoint << "\";" << std::endl;;
	ss << std::endl;
	ss << "    constexpr static auto shaderSource = ";
	insertMultiLineString(shaderModule.sourceCodePreprocessed, ss, 1);
	ss << std::endl;
	ss << "    constexpr static std::array<uint32_t, " << shaderModule.spirVCode.size() << "> shaderSourceSpirV =";
	insertMultiLineUInt32Array(shaderModule.spirVCode, ss, 1);
	ss << std::endl;
	ss << "    constexpr static auto shaderSourceSpirVAssembly =";
	insertMultiLineString(shaderModule.spirVCodeAssembly, ss, 1);
	ss << std::endl;
	ss << "};" << std::endl << std::endl;

	if (classNamespace)
	{
		ss << "} // namespace " << *classNamespace;
	}

	return ss.str();
}

} // namespace ShaderCompiler
