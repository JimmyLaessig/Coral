#ifndef CORAL_SAMPLER_HPP
#define CORAL_SAMPLER_HPP

#include <Coral/Sampler.h>

#include <memory>

namespace Coral
{

/// 
class CORAL_API Sampler
{
public:

    using CreateConfig = CoSamplerCreateConfig;

    enum class CreateError
    {
        INTERNAL_ERROR
    };

    virtual ~Sampler() = default;

    ///
    virtual CoFilter magFilter() const = 0;

    ///
    virtual CoFilter minFilter() const = 0;

    ///
    virtual CoFilter mipMapFilter() const = 0;

    ///
    virtual CoWrapMode wrapMode() const = 0;
};

} // namespace Coral

struct CoSampler_T
{
    std::shared_ptr<Coral::Sampler> impl;
};

#endif // !CORAL_SAMPLER_HPP
