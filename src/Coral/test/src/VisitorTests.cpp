#include <catch2/catch_all.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>

#include <Common/Visitor.hpp>



TEST_CASE("Test Vistor")
{
	std::variant<std::string, int, float> v;
	
	SECTION("void visitors")
	{
		bool called{ false };


		SECTION("float value")
		{
			v = 1.f;

			Common::visit(v, Common::Visitor
				{
					[](auto) { CHECK(false); },
					[&](float f) { CHECK(f == 1.f); called = true; },
				});
		}

		SECTION("int value")
		{
			v = 1;

			Common::visit(v, Common::Visitor
				{
					[](auto) { CHECK(false); },
					[&](int i) { CHECK(i == 1); called = true; },
				});
		}

		SECTION("string value")
		{
			v = "abc";

			Common::visit(v, Common::Visitor
				{
					[&](const std::string& s) { CHECK(s == "abc"); called = true; },
					[](auto) { CHECK(false); },
				});
		}

		CHECK(called);
	}

	SECTION("Visitor with return value")
	{
		SECTION("float value")
		{
			v = 1.f;

			auto res = Common::visit(v, Common::Visitor
				{
					[](auto) { return 0.0f; },
					[&](float f) { return 1.f; },
				});

			CHECK(res == 1.f);
		}

		SECTION("int value")
		{
			v = 1;

			auto res = Common::visit(v, Common::Visitor
				{
					[](auto) { return 0.0f; },
					[&](int f) { return 1.f; },
				});

			CHECK(res == 1.f);
		}

		SECTION("string value")
		{
			v = "abc";

			auto res = Common::visit(v, Common::Visitor
				{
					[](auto) { return size_t(0); },
					[&](const std::string& s) { return s.size(); },
				});

			CHECK(res == 3u);
		}
	}
}
