#ifndef CORAL_UNITTESTS_CATCH_HPP
#define CORAL_UNITTESTS_CATCH_HPP

#include <catch2/catch_all.hpp>

#include <Coral/Coral.hpp>

namespace UnitTests
{

std::string createRandomString(size_t size);

class CoralTestFixture
{
public:

	CoralTestFixture();

	Coral::Context& context() const;

private:

	Coral::ContextPtr mContext;
};

} // namespace UnitTests

#endif // !CORAL_UNITTESTS_CATCH_HPP