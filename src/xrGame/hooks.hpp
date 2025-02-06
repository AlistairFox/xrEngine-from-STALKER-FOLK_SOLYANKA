#pragma once
#include "minhook/include/MinHook.h"

struct minhook_strategy
{
	bool initialize() const
	{
		return MH_Initialize() == MH_OK;
	}

	void deinitialize() const
	{
		MH_DisableHook(MH_ALL_HOOKS);
		MH_Uninitialize();
	}

	template <typename _Func_ty>
	bool install_hook(_Func_ty _Callable, _Func_ty& _Payload, _Func_ty& _Org) const
	{
		return MH_CreateHook(_Callable, _Payload, reinterpret_cast<LPVOID*>(&_Org)) == MH_OK;
	}

	bool enable_hooks() const
	{
		return MH_EnableHook(MH_ALL_HOOKS) == MH_OK;
	}
};

template <typename _Hk_strategy, typename = void>
struct is_hook_strategy : std::false_type {};

template <typename _Hk_strategy>
struct is_hook_strategy <_Hk_strategy, std::void_t<
	decltype(std::declval<_Hk_strategy>().initialize()),
	decltype(std::declval<_Hk_strategy>().deinitialize()),
	decltype(std::declval<_Hk_strategy>().
		template install_hook<uintptr_t>(
			std::declval<uintptr_t>(), // _Callable
			std::declval<uintptr_t&>(), // _Payload
			std::declval<uintptr_t&>()))>> // _Orginal
	: std::true_type{};

template <typename _Hk_strategy>
constexpr bool is_hook_strategy_v = is_hook_strategy<_Hk_strategy>::value;

template <typename _Hook_strategy>
class basic_hook
{
public:
	static_assert(is_hook_strategy_v<_Hook_strategy>, "_Hook_strategy type must satisfy strategy requirements");

	using value_type = _Hook_strategy;
	using hook_type = value_type;

	basic_hook()
	{
		_Ok = _Strategy.initialize();
	}

	~basic_hook()
	{
		_Strategy.deinitialize();
	}

	template <typename _Func_ty,
		typename = std::enable_if_t<std::is_pointer_v<_Func_ty>>>
	_Func_ty install_hook(const std::string& module, const std::string& func_name, _Func_ty overloaded, _Func_ty& original_func) 
	{
		if (!_Ok)
			return {};
		
		HMODULE _Internal = GetModuleHandle(module.data());

		if (!_Internal)
			return {};

		_Func_ty _Callable = reinterpret_cast<_Func_ty>(GetProcAddress(_Internal, func_name.data()));
		if (!_Callable || !_Strategy.install_hook<_Func_ty>(_Callable, overloaded, original_func))
		{
			_Ok = false;
			return {};
		}

		return _Callable;
	}

	bool enable_hooks() const
	{
		if (!_Ok)
			return false;
		return _Strategy.enable_hooks();
	}

private:
	bool _Ok;
	hook_type _Strategy;
};

template <typename _Hook_strategy>
using hook = basic_hook<_Hook_strategy>;