#include <catch2/catch_all.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>

#include <Common/TypeName.hpp>

#include <iostream>

struct MyStruct
{

};

class MyClass
{

};

namespace Namespace1
{
	struct MyStruct1
	{

	};

	class MyClass1
	{

	};

	namespace Namespace2
	{
		struct MyStruct2
		{

		};

		class MyClass2
		{

		};
	};
} 


TEST_CASE("Test typeName")
{
	CHECK(Common::typeName<MyStruct>() == "MyStruct");
	CHECK(Common::typeName<Namespace1::MyStruct1>() == "Namespace1::MyStruct1");
	CHECK(Common::typeName<Namespace1::Namespace2::MyStruct2>() == "Namespace1::Namespace2::MyStruct2");

	CHECK(Common::typeName<MyClass>() == "MyClass");
	CHECK(Common::typeName<Namespace1::MyClass1>() == "Namespace1::MyClass1");
	CHECK(Common::typeName<Namespace1::Namespace2::MyClass2>() == "Namespace1::Namespace2::MyClass2");

	// The typename function does not work with alias types or templated types
	CHECK_FALSE(Common::typeName<size_t>() == "size_t");
}
