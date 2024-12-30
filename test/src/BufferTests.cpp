#include "Catch.hpp"

#include <Coral/Coral.hpp>

void
checkBuffer(const Coral::Buffer& buffer, const Coral::BufferCreateConfig& expected)
{
	CHECK(buffer.size() == expected.size);
	CHECK(buffer.type() == expected.type);
}

TEST_CASE_PERSISTENT_FIXTURE(UnitTests::CoralTestFixture, "Buffer creation tests")
{
	auto& ctx = context();

	auto bufferType = GENERATE(Coral::BufferType::INDEX_BUFFER, Coral::BufferType::STORAGE_BUFFER, Coral::BufferType::UNIFORM_BUFFER, Coral::BufferType::VERTEX_BUFFER);
	auto cpuVisible = GENERATE(true, false);

	SECTION("Zero-sized buffer - creation should fail")
	{
		Coral::BufferCreateConfig config{};
		config.size		  = 0;
		config.type		  = bufferType;
		config.cpuVisible = cpuVisible;
		auto buffer       = ctx.createBuffer(config);

		REQUIRE_FALSE(buffer);
		CHECK(buffer.error() == Coral::BufferCreationError::INVALID_SIZE);
	}

	SECTION("Non-zero buffer - creation should succeed")
	{
		Coral::BufferCreateConfig config{};
		config.size		  = GENERATE(1, 100, 10000, 500000);
		config.type		  = bufferType;
		config.cpuVisible = cpuVisible;
		auto buffer		  = ctx.createBuffer(config);

		REQUIRE(buffer);
		REQUIRE(buffer.value());
		checkBuffer(**buffer, config);
	}
}


TEST_CASE_PERSISTENT_FIXTURE(UnitTests::CoralTestFixture, "Buffer memory mapping tests")
{
	auto& ctx = context();

	auto bufferType = GENERATE(Coral::BufferType::INDEX_BUFFER, Coral::BufferType::STORAGE_BUFFER, 
							   Coral::BufferType::UNIFORM_BUFFER, Coral::BufferType::VERTEX_BUFFER);

	SECTION("Map non cpuVisible buffer - should fail")
	{
		Coral::BufferCreateConfig config{};
		config.size		  = 1024;
		config.type		  = bufferType;
		config.cpuVisible = false;
		auto buffer		  = ctx.createBuffer(config);

		REQUIRE(buffer);
		REQUIRE(buffer.value());
		CHECK(buffer.value()->map() == nullptr);
	}

	SECTION("Map cpuVisible buffer - should succeed")
	{
		auto size = GENERATE(4, 64, 1024, 512, 100000);

		Coral::BufferCreateConfig config{};
		config.size = size;
		config.type = bufferType;
		config.cpuVisible = true;
		auto buffer = ctx.createBuffer(config);

		REQUIRE(buffer);
		REQUIRE(buffer.value() != nullptr);
		CHECK(buffer.value()->size() == size);

		// Calling unmap without map should fail
		REQUIRE_FALSE(buffer.value()->unmap());

		// Map the buffer
		auto mapped = buffer.value()->map();
		CHECK(mapped != nullptr);

		// Calling map again should fail
		CHECK(buffer.value()->map() == nullptr);

		// Create some randomized data and copy it to the buffer
		std::string data = UnitTests::createRandomString(size);

		std::memcpy(mapped, data.data(), size);
		
		// Unmap the buffer to synchronize the changes
		REQUIRE(buffer.value()->unmap());
		mapped = nullptr;

		// Mapping the buffer again should download the same data that we uploaded before
		mapped = buffer.value()->map();
		REQUIRE(mapped);

		std::string downloaded(size, '0');
		std::memcpy(downloaded.data(), mapped, size);

		CHECK(data == downloaded);
		
		REQUIRE(buffer.value()->unmap());
	}
}