#ifndef CORAL_SHADERLANGUAGE_ATTRIBUTE_HPP
#define CORAL_SHADERLANGUAGE_ATTRIBUTE_HPP

#include <Coral/ShaderLanguage/ShaderGraph.hpp>
#include <Coral/ShaderLanguage/Value.hpp>

#include <Coral/ShaderLanguage/Binding.hpp>
#include <concepts>
#include <string_view>
#include <iostream>
#include <optional>

namespace Coral::ShaderLanguage
{

enum Qualifier
{
	IN,
	OUT,
};


template <size_t N>
using Semantic = StringLiteral<N>;

constexpr Semantic SV_POSITION = "SV_POSITION";

constexpr Semantic SV_DEPTH = "SV_DEPTH";


template<Semantic S>
constexpr bool IsDefaultSemantic()
{
	return false;
}


template<>
constexpr bool IsDefaultSemantic<SV_POSITION>()
{
	return true;
}


template<>
constexpr bool IsDefaultSemantic<SV_DEPTH>()
{
	return true;
}


template<Semantic S>
constexpr DefaultSemantics Convert()
{
	static_assert(false);
}


template<>
constexpr DefaultSemantics Convert<SV_POSITION>()
{
	return DefaultSemantics::SV_POSITION;
}


template<>
constexpr DefaultSemantics Convert<SV_DEPTH>()
{
	return DefaultSemantics::SV_DEPTH;
}

/// Class defining a shader input or output attribute
/*
 * \param T the type of the attribute (must be derived from csl::Value)
 * \param S Semantic of the attribute.
 * \param L Location of the attribute ( Must be -1 if S is one of the default semantics)
 * \param Q Qualifier defining whether this is a shader input or output attribute (Must be OUT if S is one of the default semantics)
 *
 */
template<typename T, Semantic S, Location L = Location(-1), Qualifier Q = Qualifier::OUT>
class Attribute
{
public:

	Attribute() requires (Q == Qualifier::IN)
		: mValue(T(Node::create(InputAttributeExpression(T::ValueType, static_cast<uint32_t>(L), S))))
	{
	}


	Attribute() requires (Q == Qualifier::OUT)
	{
	}


	~Attribute() requires (Q == Qualifier::OUT)
	{
		if (!mValue)
		{
			mValue.emplace(MakeDefaultOutputAttributeExpression());
		}
	}


	~Attribute() requires (Q == Qualifier::IN)
	{
	}


	const T* operator->() const  requires (Q == Qualifier::IN)
	{
		return &mValue.value();
	}


	const T& operator*() const  requires (Q == Qualifier::IN)
	{
		return *mValue;
	}


	operator T() const requires (Q == Qualifier::IN)
	{
		return *mValue;
	}


	const T& Value() const  requires (Q == Qualifier::IN)
	{
		return *mValue;
	}


	Attribute& operator=(const T& value) requires (Q == Qualifier::OUT)
	{
		if (!mValue)
		{
			mValue.emplace(MakeOutputAttributeExpression(value));
		}
		else
		{
			*mValue = value;
		}
		return *this;
	}


	Attribute& operator=(T&& value) requires (Q == Qualifier::OUT)
	{
		if (!mValue)
		{
			mValue.emplace(MakeOutputAttributeExpression(std::move(value)));
		}
		else
		{
			*mValue = value;
		}

		return *this;
	}

private:

	std::shared_ptr<Node>
	MakeDefaultOutputAttributeExpression() requires (IsDefaultSemantic<S>())
	{
		T defaultValue;
		return Node::create(OutputAttributeExpression(T::ValueType, Convert<S>()), std::move(defaultValue).node());
	}


	std::shared_ptr<Node>
	MakeDefaultOutputAttributeExpression() requires (!IsDefaultSemantic<S>())
	{
		T defaultValue;
		return Node::create(OutputAttributeExpression(T::ValueType, static_cast<uint32_t>(L), S), std::move(defaultValue).node());
	}


	std::shared_ptr<Node>
	MakeOutputAttributeExpression(const T& value) requires (IsDefaultSemantic<S>())
	{
		return Node::create(OutputAttributeExpression(T::ValueType, Convert<S>()), value.node());
	}


	std::shared_ptr<Node>
	MakeOutputAttributeExpression(const T& value) requires (!IsDefaultSemantic<S>())
	{
		return Node::create(OutputAttributeExpression(T::ValueType, static_cast<uint32_t>(L), S), value.node());
	}


	std::shared_ptr<Node>
	MakeOutputAttributeExpression(T&& value) requires (IsDefaultSemantic<S>())
	{
		return Node::create(OutputAttributeExpression(T::ValueType, Convert<S>()), std::move(value).node());
	}


	std::shared_ptr<Node>
	MakeOutputAttributeExpression(T&& value) requires (!IsDefaultSemantic<S>())
	{
		return Node::create(OutputAttributeExpression(T::ValueType, static_cast<uint32_t>(L), S), std::move(value).node());
	}

	std::optional<T> mValue;
};

} // namespace ShaderLanguage 

#endif // !CORAL_SHADERLANGUAGE_ATTRIBUTE_HPP
