
#include <Coral/HeaderGenerator.h>
#include <Coral/ShaderCompiler.h>

#include <CLI/CLI.hpp>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <optional>
#include <span>
#include <sstream>
#include <string>
#include <string_view>


std::optional<std::string>
loadFile(const std::filesystem::path& path)
{
	std::ifstream file(path, std::ios::in | std::ios::binary);
	if (!file.good())
	{
		return {};
	}

	std::stringstream buffer;
	buffer << file.rdbuf();

	return { buffer.str() };
}


bool
writeFile(std::string_view content, const std::filesystem::path& filePath)
{
	auto parentDirectory = filePath.parent_path();

	if (!std::filesystem::exists(parentDirectory) && !parentDirectory.empty())
	{
		std::filesystem::create_directories(parentDirectory);
	}

	std::ofstream f(filePath);

	if (!f.is_open())
	{
		return false;
	}

	f.write(content.data(), content.size());

	return true;
}


int main(int argc, char** argv)
{
	std::string shaderName;
	std::string shaderNamespace;
	std::filesystem::path inputPath;
	std::filesystem::path outputPath;
	std::string entryPoint;
	Coral::ShaderStage type;
	Coral::ShaderLanguage language;

	CLI::App app{ "Coral Shader Compiler" };

	std::map<std::string, Coral::ShaderStage> shaderStageMap{ { "vertex", Coral::ShaderStage::VERTEX }, { "fragment", Coral::ShaderStage::FRAGMENT } };

	std::map<std::string, Coral::ShaderLanguage> languageMap{ { "glsl", Coral::ShaderLanguage::GLSL }, { "hlsl", Coral::ShaderLanguage::HLSL } };

	app.add_option("-n,--name",		  shaderName,	   "The name of the compiled shader")->required(true);
	app.add_option("--namespace",     shaderNamespace, "The optional namespace of the compiled shader header")->required(true);
	app.add_option("-o,--output",	  outputPath,	   "The path to the compiled shader header")->required(true);
	app.add_option("-i,--input",	  inputPath,	   "The path to the HLSL shader file")->required(true);
	app.add_option("-e,--entrypoint", entryPoint,	   "The name of the shader entry point function")->required(true);
	app.add_option("-s,--stage",	  type,			   "The stage of the shader")->transform(CLI::CheckedTransformer(shaderStageMap, CLI::ignore_case))->required(true);
	app.add_option("-l,--language",   language,		   "The source language of the shader code")->transform(CLI::CheckedTransformer(languageMap, CLI::ignore_case))->required(true);

	try
	{
		app.parse(argc, argv);
	}
	catch (const CLI::ParseError& e)
	{
		return app.exit(e);
	}

	std::cout << "Executing Coral Shader Compiler for shader '" << shaderName << "'..." << std::endl;

	auto content = loadFile(inputPath);

	if (!content)
	{
		std::cerr << "Failed to open input file '" << inputPath << "'." << std::endl;
		return EXIT_FAILURE;
	}

	Coral::ShaderModuleCompileInfo compileInfo;
	compileInfo.filePath	= inputPath;
	compileInfo.entryPoint  = entryPoint;
	compileInfo.sourceCode  = std::move(*content);
	compileInfo.type		= type;
	compileInfo.language    = language;
	compileInfo.additionalIncludeDirectories = { std::filesystem::current_path() };

	auto compiledShader = Coral::compileHLSLToSpirV(compileInfo);

	if (!compiledShader)
	{
		return EXIT_FAILURE;
	}
	
	std::string fileContent = Coral::buildShaderHeader(shaderName, shaderNamespace, compileInfo, *compiledShader);

	if (!writeFile(fileContent, outputPath))
	{
		std::cerr << "Failed to write output file " << outputPath << "." << std::endl;
		return EXIT_FAILURE;
	}

	std::cout << "Successfully written shader '"<< shaderName << "' to " << outputPath << "." << std::endl;
	return EXIT_SUCCESS;
}