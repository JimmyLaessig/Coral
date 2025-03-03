
#include "Catch.hpp"

#include <Coral/Coral.hpp>

namespace
{

void 
checkBufferView(const Coral::BufferView& view, const Coral::BufferViewCreateConfig& expected)
{
	CHECK(view.attributeFormat() == expected.attribute);
	CHECK(view.buffer() == expected.buffer);
	CHECK(view.count() == expected.count);
	CHECK(view.offset() == expected.offset);

	auto expectedStride = expected.stride == 0 ? Coral::sizeInBytes(expected.attribute) : expected.stride;
	CHECK(view.stride() == expectedStride);
}

} // namespace

TEST_CASE_PERSISTENT_FIXTURE(UnitTests::CoralTestFixture, "Buffer View creation tests")
{
	auto& ctx = context();
	SECTION("Create BufferView for storage, uniform and vertex buffers")
	{
		auto bufferType = GENERATE(Coral::BufferType::STORAGE_BUFFER, Coral::BufferType::UNIFORM_BUFFER, Coral::BufferType::VERTEX_BUFFER);

		// Create a buffer with 10 3-component vectors
		Coral::BufferCreateConfig config{};
		config.size = 10 * Coral::sizeInBytes(Coral::AttributeFormat::VEC3F);
		config.type = bufferType;
		config.cpuVisible = false;
		auto buf = ctx.createBuffer(config);
		REQUIRE(buf.has_value());
		auto buffer = buf->get();

		SECTION("Buffer is nullptr - creation should fail")
		{
			Coral::BufferViewCreateConfig config{};
			config.attribute = Coral::AttributeFormat::VEC3F;
			config.count = 10;
			config.offset = 0;
			config.buffer = nullptr;
			auto bufferView = ctx.createBufferView(config);

			REQUIRE_FALSE(bufferView);
			CHECK(bufferView.error() == Coral::BufferViewCreationError::INVALID_BUFFER);
		}

		SECTION("Empty buffer view - creation should fail")
		{
			Coral::BufferViewCreateConfig config{};
			config.attribute = Coral::AttributeFormat::VEC3F;
			config.count = 0;
			config.offset = 0;
			config.buffer = buffer;
			auto bufferView = ctx.createBufferView(config);

			REQUIRE_FALSE(bufferView);
			CHECK(bufferView.error() == Coral::BufferViewCreationError::EMPTY_VIEW);
		}

		SECTION("Offset too large - creation should fail")
		{
			Coral::BufferViewCreateConfig config{};
			config.attribute = Coral::AttributeFormat::VEC3F;
			config.count = 10;
			config.offset = 1;
			config.buffer = buffer;
			auto bufferView = ctx.createBufferView(config);

			REQUIRE_FALSE(bufferView);
			CHECK(bufferView.error() == Coral::BufferViewCreationError::INVALID_SIZE);
		}

		SECTION("Create tightly packed buffer view")
		{
			Coral::BufferViewCreateConfig config{};
			config.attribute = Coral::AttributeFormat::VEC3F;
			config.count = 10;
			config.offset = 0;
			config.buffer = buffer;
			auto bufferView = ctx.createBufferView(config);

			REQUIRE(bufferView);
			REQUIRE(bufferView->get());
			checkBufferView(**bufferView, config);
		}

		SECTION("Create tightly packed buffer view with offset")
		{
			Coral::BufferViewCreateConfig config{};
			config.attribute = Coral::AttributeFormat::VEC3F;
			config.count     = 5;
			config.offset    = 5 * Coral::sizeInBytes(Coral::AttributeFormat::VEC3F);
			config.buffer    = buffer;
			auto bufferView  = ctx.createBufferView(config);

			REQUIRE(bufferView);
			REQUIRE(bufferView->get());
			checkBufferView(**bufferView, config);
		}

		SECTION("Create interleaved buffer views")
		{
			// Instead of treating the buffer as 10 Vec3 vertices, interpret the data has 5 vertices with interleaved
			// position and normal attributes.
			constexpr uint32_t vertexSize = Coral::sizeInBytes(Coral::AttributeFormat::VEC3F) * 2;

			Coral::BufferViewCreateConfig config{};
			config.attribute = Coral::AttributeFormat::VEC3F;
			config.count = 5;
			config.offset = 0;
			config.stride = vertexSize;
			config.buffer = buffer;
			auto view1 = ctx.createBufferView(config);

			REQUIRE(view1);
			REQUIRE(view1->get());
			checkBufferView(**view1, config);

			config.attribute = Coral::AttributeFormat::VEC3F;
			config.count = 5;
			config.offset = Coral::sizeInBytes(Coral::AttributeFormat::VEC3F);
			config.stride = vertexSize;
			config.buffer = buffer;
			auto view2 = ctx.createBufferView(config);

			REQUIRE(view2);
			REQUIRE(view2->get());
			checkBufferView(**view2, config);
		}
	}

	SECTION("Create BufferView for IndexBuffers - elements must be tightly packed")
	{
		// Create a buffer with 10 unsigned integers
		Coral::BufferCreateConfig config{};
		config.size			= 10 * Coral::sizeInBytes(Coral::AttributeFormat::UINT32);
		config.type			= Coral::BufferType::INDEX_BUFFER;
		config.cpuVisible	= false;
		auto buf			= ctx.createBuffer(config);
		REQUIRE(buf.has_value());
		auto buffer = buf->get();

		SECTION("Create BufferView with invalid stride - creation should fail")
		{
			Coral::BufferViewCreateConfig config{};
			config.attribute = Coral::AttributeFormat::UINT32;
			config.count     = 10;
			config.offset    = 0;
			config.stride    = 2;
			config.buffer    = buffer;
			auto view       = ctx.createBufferView(config);

			REQUIRE_FALSE(view.has_value());
			CHECK(view.error() == Coral::BufferViewCreationError::INVALID_STRIDE);
		}

		SECTION("Create BufferView with valid stride")
		{
			Coral::BufferViewCreateConfig config{};
			config.attribute = Coral::AttributeFormat::UINT32;
			config.count     = 10;
			config.offset    = 0;
			config.stride    = GENERATE(0u, Coral::sizeInBytes(Coral::AttributeFormat::UINT32));
			config.buffer    = buffer;
			auto view		 = ctx.createBufferView(config);

			REQUIRE(view.has_value());
			CHECK(view->get());
			checkBufferView(**view, config);
		}

		SECTION("Create BufferView with valid stride and offset")
		{
			Coral::BufferViewCreateConfig config{};
			config.attribute = Coral::AttributeFormat::UINT32;
			config.count     = 5;
			config.offset    = 5 * Coral::sizeInBytes(Coral::AttributeFormat::UINT32);
			config.stride    = GENERATE(0u, Coral::sizeInBytes(Coral::AttributeFormat::UINT32));
			config.buffer    = buffer;
			auto view        = ctx.createBufferView(config);

			REQUIRE(view.has_value());
			CHECK(view->get());
			checkBufferView(**view, config);
		}
	}
}
