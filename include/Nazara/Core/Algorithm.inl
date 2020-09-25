// Copyright (C) 2020 Jérôme Leclercq
// This file is part of the "Nazara Engine - Core module"
// For conditions of distribution and use, see copyright notice in Config.hpp

// http://stackoverflow.com/questions/687490/c0x-how-do-i-expand-a-tuple-into-variadic-template-function-arguments
// Merci à Ryan "FullMetal Alchemist" Lahfa
// Merci aussi à Freedom de siteduzero.com

#include <Nazara/Core/Algorithm.hpp>
#include <Nazara/Core/AbstractHash.hpp>
#include <Nazara/Core/ByteArray.hpp>
#include <Nazara/Core/Error.hpp>
#include <Nazara/Core/Stream.hpp>
#include <cassert>
#include <climits>
#include <utility>
#include <Nazara/Core/Debug.hpp>

namespace Nz
{
	namespace Detail
	{
		// http://www.cppsamples.com/common-tasks/apply-tuple-to-function.html
		template<typename F, typename Tuple, size_t... S>
		decltype(auto) ApplyImplFunc(F&& fn, Tuple&& t, std::index_sequence<S...>)
		{
			return std::forward<F>(fn)(std::get<S>(std::forward<Tuple>(t))...);
		}

		template<typename O, typename F, typename Tuple, size_t... S>
		decltype(auto) ApplyImplMethod(O& object, F&& fn, Tuple&& t, std::index_sequence<S...>)
		{
			return (object .* std::forward<F>(fn))(std::get<S>(std::forward<Tuple>(t))...);
		}

		NAZARA_CORE_API extern const UInt8 BitReverseTable256[256];
	}

	/*!
	* \ingroup core
	* \brief Align an offset
	* \return Aligned offset according to alignment
	*
	* \param offset Base offset
	* \param alignment Non-zero alignment
	*
	* \see AlignPow2
	*/
	template<typename T>
	constexpr T Align(T offset, T alignment)
	{
		assert(alignment > 0);
		return ((offset + alignment - 1) / alignment) * alignment;
	}

	/*!
	* \ingroup core
	* \brief Align an offset
	* \return Aligned offset according to a power of two alignment
	*
	* \param offset Base offset
	* \param alignment Non-zero power of two alignment
	*
	* \see Align
	* \remark This function is quicker than Align but only works with power of two alignment values
	*/
	template<typename T>
	constexpr T AlignPow2(T offset, T alignment)
	{
		assert(alignment > 0);
		assert(IsPowerOfTwo(alignment));

		return (offset + alignment - 1) & ~(alignment - 1);
	}

	/*!
	* \ingroup core
	* \brief Applies the tuple to the function (e.g. calls the function using the tuple content as arguments)
	* \return The result of the function
	*
	* \param fn Function
	* \param t Tuple of arguments for the function
	*
	* \see Apply
	*/
	template<typename F, typename Tuple>
	decltype(auto) Apply(F&& fn, Tuple&& t)
	{
		constexpr std::size_t tSize = std::tuple_size<typename std::remove_reference<Tuple>::type>::value;

		return Detail::ApplyImplFunc(std::forward<F>(fn), std::forward<Tuple>(t), std::make_index_sequence<tSize>());
	}

	/*!
	* \ingroup core
	* \brief Applies the tuple to the member function on an object (e.g. calls the member function using the tuple content as arguments)
	* \return The result of the member function called
	*
	* \param object Object of a class
	* \param fn Member function
	* \param t Tuple of arguments for the member function
	*
	* \see Apply
	*/
	template<typename O, typename F, typename Tuple>
	decltype(auto) Apply(O& object, F&& fn, Tuple&& t)
	{
		constexpr std::size_t tSize = std::tuple_size<typename std::remove_reference<Tuple>::type>::value;

		return Detail::ApplyImplMethod(object, std::forward<F>(fn), std::forward<Tuple>(t), std::make_index_sequence<tSize>());
	}

	/*!
	* \ingroup core
	* \brief Returns the number of bits occupied by the type T
	* \return Number of bits occupied by the type
	*/
	template<typename T>
	constexpr std::size_t BitCount()
	{
		return CHAR_BIT * sizeof(T);
	}

	/*!
	* \ingroup core
	* \brief Computes the hash of a hashable object
	* \return A bytearray which represents the hash
	*
	* \param hash Enumeration of type HashType
	* \param v Object to hash
	*
	* \remark a HashAppend specialization for type T is required
	*
	* \see ComputeHash
	*/
	template<typename T>
	ByteArray ComputeHash(HashType hash, const T& v)
	{
		return ComputeHash(AbstractHash::Get(hash).get(), v);
	}

	/*!
	* \ingroup core
	* \brief Computes the hash of a hashable object
	* \return A bytearray which represents the hash
	*
	* \param hash Pointer to abstract hash
	* \param v Object to hash
	*
	* \remark Produce a NazaraAssert if pointer to Abstracthash is invalid
	* \remark a HashAppend specialization for type T is required
	*
	* \see ComputeHash
	*/
	template<typename T>
	ByteArray ComputeHash(AbstractHash* hash, const T& v)
	{
		NazaraAssert(hash != nullptr, "Invalid abstracthash pointer");

		hash->Begin();

		HashAppend(hash, v);

		return hash->End();
	}

	/*!
	* \ingroup core
	* \brief Returns the number of elements in a C-array
	* \return The number of elements
	*
	* \see CountOf
	*/
	template<typename T, std::size_t N>
	constexpr std::size_t CountOf(T(&)[N]) noexcept
	{
		return N;
	}

	/*!
	* \ingroup core
	* \brief Returns the number of elements in a container
	* \return The number of elements
	*
	* \param c Container with the member function "size()"
	*
	* \see CountOf
	*/
	template<typename T>
	std::size_t CountOf(const T& c)
	{
		return c.size();
	}

	inline bool HashAppend(AbstractHash* hash, const std::string_view& v)
	{
		hash->Append(reinterpret_cast<const UInt8*>(v.data()), v.size());
		return true;
	}

	/*!
	* \ingroup core
	* \brief Combines two hash in one
	*
	* \param seed First value that will be modified (expected to be 64bits)
	* \param v Second value to hash
	*/
	// Algorithm from CityHash by Google
	// http://stackoverflow.com/questions/8513911/how-to-create-a-good-hash-combine-with-64-bit-output-inspired-by-boosthash-co
	template<typename T>
	void HashCombine(std::size_t& seed, const T& v)
	{
		const UInt64 kMul = 0x9ddfea08eb382d69ULL;

		std::hash<T> hasher;
		UInt64 a = (hasher(v) ^ seed) * kMul;
		a ^= (a >> 47);

		UInt64 b = (seed ^ a) * kMul;
		b ^= (b >> 47);

		seed = static_cast<std::size_t>(b * kMul);
	}

	/*!
	* \ingroup core
	* \brief Check if a value is a power of two
	* \return true if value is a power of two
	*
	* \param value Non-zero value
	*/
	template<typename T>
	bool IsPowerOfTwo(T value)
	{
		assert(value != 0);
		return (value & (value - 1)) == 0;
	}

	/*!
	* \ingroup core
	* \brief Reverse the bit order of the integer
	* \return integer with reversed bits
	*
	* \param integer Integer whose bits are to be reversed
	*/
	template<typename T>
	T ReverseBits(T integer)
	{
		T reversed = 0;
		for (std::size_t i = 0; i < sizeof(T); ++i)
			reversed |= T(Detail::BitReverseTable256[(integer >> i * 8) & 0xFF]) << (sizeof(T) * 8 - (i + 1) * 8);

		return reversed;
	}

	template<typename T>
	constexpr auto UnderlyingCast(T value) -> std::underlying_type_t<T>
	{
		return static_cast<std::underlying_type_t<T>>(value);
	}

	template<typename T> struct PointedType<T*>                { using type = T; };
	template<typename T> struct PointedType<T* const>          { using type = T; };
	template<typename T> struct PointedType<T* volatile>       { using type = T; };
	template<typename T> struct PointedType<T* const volatile> { using type = T; };


	template<typename T>
	bool Serialize(SerializationContext& context, T&& value)
	{
		return Serialize(context, std::forward<T>(value), TypeTag<std::decay_t<T>>());
	}

	/*!
	* \ingroup core
	* \brief Serializes a boolean
	* \return true if serialization succeeded
	*
	* \param context Context for the serialization
	* \param value Boolean to serialize
	*
	* \see Serialize, Unserialize
	*/
	inline bool Serialize(SerializationContext& context, bool value, TypeTag<bool>)
	{
		if (context.writeBitPos == 8)
		{
			context.writeBitPos = 0;
			context.writeByte = 0;
		}

		if (value)
			context.writeByte |= 1 << context.writeBitPos;

		if (++context.writeBitPos >= 8)
			return Serialize(context, context.writeByte, TypeTag<UInt8>());
		else
			return true;
	}

	/*!
	* \ingroup core
	* \brief Serializes a std::string
	* \return true if successful
	*
	* \param context Context for the serialization
	* \param value String to serialize
	*/
	bool Serialize(SerializationContext& context, const std::string& value, TypeTag<std::string>)
	{
		if (!Serialize(context, UInt32(value.size()), TypeTag<UInt32>()))
			return false;

		return context.stream->Write(value.data(), value.size()) == value.size();
	}

	/*!
	* \ingroup core
	* \brief Serializes an arithmetic type
	* \return true if serialization succeeded
	*
	* \param context Context for the serialization
	* \param value Arithmetic type to serialize
	*
	* \see Serialize, Unserialize
	*/
	template<typename T>
	std::enable_if_t<std::is_arithmetic<T>::value, bool> Serialize(SerializationContext& context, T value, TypeTag<T>)
	{
		// Flush bits in case a writing is in progress
		context.FlushBits();

		if (context.endianness != Endianness_Unknown && context.endianness != GetPlatformEndianness())
			SwapBytes(&value, sizeof(T));

		return context.stream->Write(&value, sizeof(T)) == sizeof(T);
	}


	template<typename T>
	bool Unserialize(SerializationContext& context, T* value)
	{
		return Unserialize(context, value, TypeTag<T>());
	}

	/*!
	* \ingroup core
	* \brief Unserializes a boolean
	* \return true if unserialization succedeed
	*
	* \param context Context for the unserialization
	* \param value Pointer to boolean to unserialize
	*
	* \see Serialize, Unserialize
	*/
	inline bool Unserialize(SerializationContext& context, bool* value, TypeTag<bool>)
	{
		if (context.readBitPos == 8)
		{
			if (!Unserialize(context, &context.readByte, TypeTag<UInt8>()))
				return false;

			context.readBitPos = 0;
		}

		if (value)
			*value = (context.readByte & (1 << context.readBitPos)) != 0;

		context.readBitPos++;

		return true;
	}

	/*!
	* \brief Unserializes a string
	* \return true if successful
	*
	* \param context Context of unserialization
	* \param string std::string to unserialize
	*/
	bool Unserialize(SerializationContext& context, std::string* string, TypeTag<std::string>)
	{
		UInt32 size;
		if (!Unserialize(context, &size, TypeTag<UInt32>()))
			return false;

		string->resize(size);
		return context.stream->Read(&(*string)[0], size) == size;
	}

	/*!
	* \ingroup core
	* \brief Unserializes an arithmetic type
	* \return true if unserialization succedeed
	*
	* \param context Context for the unserialization
	* \param value Pointer to arithmetic type to serialize
	*
	* \remark Produce a NazaraAssert if pointer to value is invalid
	*
	* \see Serialize, Unserialize
	*/
	template<typename T>
	std::enable_if_t<std::is_arithmetic<T>::value, bool> Unserialize(SerializationContext& context, T* value, TypeTag<T>)
	{
		NazaraAssert(value, "Invalid data pointer");

		context.ResetReadBitPosition();

		if (context.stream->Read(value, sizeof(T)) == sizeof(T))
		{
			if (context.endianness != Endianness_Unknown && context.endianness != GetPlatformEndianness())
				SwapBytes(value, sizeof(T));

			return true;
		}
		else
			return false;
	}
}

#include <Nazara/Core/DebugOff.hpp>
