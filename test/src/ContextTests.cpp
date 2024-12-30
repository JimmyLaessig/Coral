#include "Catch.hpp"

#include <Coral/Context.hpp>

TEST_CASE("Context creation test")
{
	Coral::ContextCreateConfig config{};
	config.graphicsAPI = Coral::GraphicsAPI::VULKAN;

	// We should always be able to create a context. Context creation should only ever fail if we don't have a
	// Vulkan-cabable device.
	auto ctx = Coral::createContext(config);

	REQUIRE(ctx.has_value());
	CHECK(*ctx != nullptr);
}