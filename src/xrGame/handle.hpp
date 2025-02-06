#pragma once
#include <Windows.h>
#include <xutility>

template <typename handle_t>
class basic_handle
{
public:
	basic_handle(const basic_handle& object) = delete;
	basic_handle& operator=(const basic_handle& object) = delete;

	constexpr basic_handle(handle_t object) noexcept : handle{ object } {}
	constexpr basic_handle(basic_handle&& object) noexcept : handle{ object.release() } {}
	constexpr basic_handle() noexcept : handle{} {}

	~basic_handle() { close(); }

	[[nodiscard]] handle_t get() const noexcept
	{
		return handle;
	}

	handle_t release() noexcept
	{
		return std::exchange(handle, nullptr);
	}

	bool reset()
	{
		return close();
	}

	bool close()
	{
		if (handle)
		{
			PVOID address = handle;
			handle = nullptr;
			return CloseHandle(address);
		}
		return true;
	}

	explicit operator bool() const noexcept
	{
		return handle != nullptr && handle != INVALID_HANDLE_VALUE;
	}

private:
	handle_t handle;
};

using handle_ptr = basic_handle<HANDLE>;