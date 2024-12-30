#include "Catch.hpp"


std::string
Catch::StringMaker<std::byte>::convert(std::byte value)
{
	return std::to_string((uint8_t)value);
}

namespace UnitTests
{

std::string
createRandomString(size_t size)
{
	return std::string(size, '0');
}


CoralTestFixture::CoralTestFixture()
{
	Coral::ContextCreateConfig config{};
	config.graphicsAPI	   = Coral::GraphicsAPI::VULKAN;
	config.applicationName = "CoralUnitTests";
	auto context           = Coral::createContext(config);

	REQUIRE(context.has_value());
	REQUIRE(context.value());

	mContext = std::move(context.value());
}


Coral::Context&
CoralTestFixture::context() const
{
	return *mContext;
}

} // namespace UnitTests
