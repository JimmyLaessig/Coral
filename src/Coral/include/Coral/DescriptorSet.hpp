#ifndef CORAL_DESCRIPTORSET_HPP
#define CORAL_DESCRIPTORSET_HPP

#include <Coral/System.hpp>

#include <Coral/Buffer.hpp>
#include <Coral/Image.hpp>
#include <Coral/Sampler.hpp>

#include <variant>
#include <vector>


namespace Coral
{

struct CombinedTextureSampler
{
	Image* texture{ nullptr };
	Sampler* sampler{ nullptr };
};

using Descriptor = std::variant<Buffer*, Sampler*, Image*, CombinedTextureSampler>;

struct DescriptorBinding
{
	uint32_t binding{ 0 };

	Descriptor descriptor;
};


/// Configuration to create a Buffer

struct DescriptorSetCreateConfig
{
	std::vector<DescriptorBinding> bindings;
};

enum class DescriptorSetCreationError
{
	INTERNAL_ERROR
};

class CORAL_API DescriptorSet
{
public:

	virtual ~DescriptorSet() = default;

};

} // namespace Coral

#endif // !CORAL_DESCRIPTORSET_HPP