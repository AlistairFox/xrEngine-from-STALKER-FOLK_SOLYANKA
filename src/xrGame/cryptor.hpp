#pragma once
#include <type_traits>
#include <array>
#include <string>

struct _Crypt_strong
{
	static constexpr char _Key = __TIME__[6];
	static constexpr char _Key2 = __TIME__[7];
};

struct _Crypt_medium
{
	static constexpr char _Key = __TIME__[3];
	static constexpr char _Key2 = __TIME__[4];
};

struct _Crypt_weak
{
	static constexpr char _Key = __TIME__[0];
	static constexpr char _Key2 = __TIME__[1];
};

template <typename _Crypt_method, typename = void>
struct is_crypt_method : std::false_type {};

template <typename _Crypt_method>
struct is_crypt_method<_Crypt_method, std::void_t<decltype(std::declval<_Crypt_method>()._Key),
	decltype(std::declval<_Crypt_method>()._Key2)>> : std::true_type {};

template<typename _Crypt_method>
constexpr bool is_crypt_method_v = is_crypt_method<_Crypt_method>::value;

template <typename _Crypt_method = _Crypt_strong,
	typename _Ty, size_t _Size,
	typename = std::enable_if_t<is_crypt_method_v<_Crypt_method>>>
constexpr std::array<std::remove_all_extents_t<_Ty>, _Size> __obfuscate(const _Ty(&input)[_Size]) noexcept
{
	std::array<std::remove_all_extents_t<_Ty>, _Size> _Obfuscated{};
	for (size_t i = 0; i < _Size - 1; ++i)
		_Obfuscated[i] = input[i] ^ (_Crypt_method::_Key + i % (static_cast<uintptr_t>(1) + _Crypt_method::_Key2));

	return _Obfuscated;
}

template <typename _Crypt_method = _Crypt_strong,
	typename _Ty, size_t _Size,
	typename = std::enable_if_t<is_crypt_method_v<_Crypt_strong>>>
_NODISCARD std::basic_string<_Ty> __deobfuscate(const std::array<_Ty, _Size>& array)
{
	std::basic_string<_Ty> _Deobfuscated;
	for (size_t i = 0; i < array.size() - 1; i++)
		_Deobfuscated.push_back(array[i] ^ (_Crypt_method::_Key + i % (static_cast<uintptr_t>(1) + _Crypt_method::_Key2)));

	return _Deobfuscated;
}

#define xor(string) __deobfuscate(__obfuscate(string))