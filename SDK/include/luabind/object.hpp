// Copyright (c) 2003 Daniel Wallin and Arvid Norberg

// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF
// ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
// TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
// PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT
// SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
// ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
// ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
// OR OTHER DEALINGS IN THE SOFTWARE.

#pragma once

#include <iterator>
#include <tuple>

#include <luabind/prefix.hpp>
#include <luabind/config.hpp>
#include <luabind/error.hpp>
#include <luabind/detail/pcall.hpp>
#include <luabind/detail/stack_utils.hpp>
#include <luabind/detail/policy_cons.hpp>
#include <luabind/proxy_object.hpp>

namespace luabind
{
	class LUABIND_API object
	{
    	template<typename T>
    	friend T object_cast(const object& obj);
    	template<detail::Direction>
    	friend struct detail::primitive_converter;
    
    	friend object get_globals(lua_State*);
    	friend object get_registry(lua_State*);
    	friend object newtable(lua_State*);
    	friend class detail::proxy_object;
    	friend class detail::proxy_array_object;
    	friend class detail::proxy_raw_object;
	public:

		class array_iterator
		{
		friend class object;
		public:

			typedef std::forward_iterator_tag iterator_category;
			typedef luabind::object value_type;
			typedef value_type& reference;
			typedef value_type* pointer;
			typedef void difference_type;

			array_iterator()
				: m_obj(nullptr)
				, m_key(0)
			{
			}

			array_iterator(const array_iterator& iter)
				: m_obj(iter.m_obj)
				, m_key(iter.m_key)
			{
			}

			~array_iterator() {}

			array_iterator& operator=(const array_iterator& rhs)
			{
				m_obj = rhs.m_obj;
				m_key = rhs.m_key;
				return *this;
			}

			detail::proxy_array_object operator*()
			{
				return m_obj->make_array_proxy(m_key);
			}

			detail::proxy_array_object operator->()
			{
				return m_obj->make_array_proxy(m_key);
			}

			array_iterator& operator++()
			{
				m_key++;

				// invalidate the iterator if we hit a nil element
				lua_State* L = m_obj->lua_state();
				m_obj->pushvalue();
				lua_rawgeti(L, -1, m_key);
				if (lua_isnil(L, -1)) m_key = -1;
				lua_pop(L, 2);

				return *this;
			}

			array_iterator operator++(int)
			{
				int old_key = m_key;
				m_key++;

				// invalidate the iterator if we hit a nil element
				lua_State* L = m_obj->lua_state();
				m_obj->pushvalue();
				lua_rawgeti(L, -1, m_key);
				if (lua_isnil(L, -1)) m_key = -1;
				lua_pop(L, 2);

				return array_iterator(m_obj, old_key);
			}

			bool operator!=(const array_iterator& rhs) const
			{
				return m_obj != rhs.m_obj || m_key != rhs.m_key;
			}

			bool operator==(const array_iterator& rhs) const
			{
				return !(*this != rhs);
			}

		private:

			array_iterator(object* obj, int key)
				: m_obj(obj)
				, m_key(key)
			{
			}

			object* m_obj;
			int m_key;
		};


		class iterator
		{
		friend class object;
		public:

			typedef std::forward_iterator_tag iterator_category;
			typedef luabind::object value_type;
			typedef value_type& reference;
			typedef value_type* pointer;
			typedef void difference_type;

			iterator()
				: m_obj(nullptr)
			{
			}

			iterator(const iterator& iter)
				: m_obj(iter.m_obj)
			{
				if (m_obj)
				{
					m_key = iter.m_key;
				}
			}

			iterator& operator=(const iterator& rhs)
			{
				m_obj = rhs.m_obj;
				if (m_obj)
				{
					m_key = rhs.m_key;
				}
				else
				{
					m_key.reset();
				}
				return *this;
			}

			detail::proxy_object operator*()
			{
				return m_obj->make_proxy(m_key);
			}

			detail::proxy_object operator->()
			{
				return m_obj->make_proxy(m_key);
			}

			iterator& operator++()
			{
				lua_State* L = m_obj->lua_state();
				m_obj->pushvalue();
				m_key.get(L);

				if (lua_next(L, -2) != 0)
				{
					lua_pop(L, 1);
					m_key.replace(L);
					lua_pop(L, 1);
				}
				else
				{
					lua_pop(L, 1);
					m_obj = 0;
					m_key.reset();
				}

				return *this;
			}

			bool operator!=(const iterator& rhs) const
			{
				if (m_obj != rhs.m_obj) return true;
				if (m_obj == nullptr) return false;
				if (m_obj->lua_state() != rhs.m_obj->lua_state()) return true;
				if (m_key.is_valid() != rhs.m_key.is_valid()) return true;

				// TODO: fix this. add a real equality test of the keys
				return true;
			}

			bool operator==(const iterator& rhs) const
			{
				return !(*this != rhs);
			}

			object key() const;

		private:

			iterator(luabind::object* obj, detail::lua_reference const& key)
				: m_obj(obj)
				, m_key(key)
			{
			}

			object* m_obj;
			detail::lua_reference m_key;
		};

		class raw_iterator
		{
		friend class object;
		public:

			typedef std::forward_iterator_tag iterator_category;
			typedef luabind::object value_type;
			typedef value_type& reference;
			typedef value_type* pointer;
			typedef void difference_type;

			raw_iterator()
				: m_obj(nullptr)
			{
			}

			raw_iterator(const raw_iterator& iter)
				: m_obj(iter.m_obj)
				, m_key()
			{
				if (m_obj)
				{
					m_key = iter.m_key;
				}
			}

			raw_iterator& operator=(const raw_iterator& rhs)
			{
				//std::cout << "===\n";
				m_obj = rhs.m_obj;
				if (m_obj)
				{
					m_key = rhs.m_key;
				}
				else
				{
					m_key.reset();
				}
				return *this;
			}

			detail::proxy_raw_object operator*()
			{
				return m_obj->make_raw_proxy(m_key);
			}

			detail::proxy_raw_object operator->()
			{
				return m_obj->make_raw_proxy(m_key);
			}

			raw_iterator& operator++()
			{
				lua_State* L = m_obj->lua_state();
				m_obj->pushvalue();
				m_key.get(L);

				if (lua_next(L, -2) != 0)
				{
					lua_pop(L, 1);
					m_key.replace(L);
					lua_pop(L, 1);
				}
				else
				{
					lua_pop(L, 1);
					m_key.reset();
					m_obj = nullptr;
				}

				return *this;
			}

			object key() const;

			bool operator!=(const raw_iterator& rhs) const
			{
				if (m_obj != rhs.m_obj) return true;
				if (m_obj == nullptr) return false;
				if (m_obj->lua_state() != rhs.m_obj->lua_state()) return true;
				if (m_key.is_valid() != rhs.m_key.is_valid()) return true;

				// TODO: fix this. add a real equality test of the keys
				return true;
			}

			bool operator==(const raw_iterator& rhs) const
			{
				return !(*this != rhs);
			}

		private:

			raw_iterator(object* obj, detail::lua_reference const& key)
				: m_obj(obj)
				, m_key(key)
			{}

			object* m_obj;
			detail::lua_reference m_key;
		};

	public:
		object()
			: m_state(nullptr)
		{
		}

		explicit object(lua_State* L)
			: m_state(L)
		{
		}

		template<class T>
		object(lua_State* L, const T& val)
			: m_state(L)
		{
			*this = val;
		}

		object(const object& o)
			: m_state(o.m_state)
		{
			o.m_ref.get(m_state);
			m_ref.set(m_state);
		}

		~object()
		{}

		bool is_valid() const { return m_ref.is_valid(); }

		// this is a safe substitute for an implicit converter to bool
		typedef void (object::*member_ptr)() const;
		operator member_ptr() const
		{
			if (is_valid()) return &object::dummy;
			return nullptr;
		}

		int type() const
		{
			pushvalue();
			detail::stack_pop p(lua_state(), 1);
			return lua_type(lua_state(), -1);
		}

		iterator begin() const
		{
			m_ref.get(m_state);
			lua_pushnil(m_state);
			detail::stack_pop pop(m_state, 1);
			if (lua_next(m_state, -2) == 0)
				return end();
			lua_pop(m_state, 1);
			detail::lua_reference r;
			r.set(m_state);
			iterator i(const_cast<object*>(this), r);
			return i;
		}

		iterator end() const
		{
			return iterator(0, detail::lua_reference());
		}

		array_iterator abegin() const
		{
			return array_iterator(const_cast<object*>(this), 1);
		}

		array_iterator aend() const
		{
			return array_iterator(const_cast<object*>(this), -1);
		}

		raw_iterator raw_begin() const
		{
			m_ref.get(m_state);
			lua_pushnil(m_state);
			detail::stack_pop pop(m_state, 1);
			if (lua_next(m_state, -2) == 0)
				return raw_end();
			lua_pop(m_state, 1);
			detail::lua_reference r;
			r.set(m_state);
			raw_iterator i(const_cast<object*>(this), r);
			return i;
		}

		raw_iterator raw_end() const
		{
			return raw_iterator(nullptr, detail::lua_reference());
		}

		void set() const
		{
			// you are trying to access an invalid object
			assert((m_state != nullptr) && "you are trying to access an invalid (uninitialized) object");

			allocate_slot();
			m_ref.replace(m_state);
		}
		lua_State* lua_state() const { return m_state; }
		void pushvalue() const
		{
			// you are trying to dereference an invalid object
			assert((m_ref.is_valid()) && "you are trying to access an invalid (uninitialized) object");
			assert((m_state != nullptr) && "internal error, please report");
			m_ref.get(m_state);
		}

		void swap(object& rhs);

		template<class T>
		object raw_at(const T& key)
		{
			lua_State* L = lua_state();
			pushvalue();
			detail::convert_to_lua(L, key);
			lua_rawget(L, -2);
			detail::lua_reference ref;
			ref.set(L);
			lua_pop(L, 1);
			return object(L, ref, true);
		}

		template<class T>
		object at(const T& key)
		{
			lua_State* L = lua_state();
			pushvalue();
			detail::convert_to_lua(L, key);
			lua_gettable(L, -2);
			detail::lua_reference ref;
			ref.set(L);
			lua_pop(L, 1);
			return object(L, ref, true);
		}

		template<class T>
		detail::proxy_object operator[](const T& key) const
		{
			detail::convert_to_lua(m_state, key);
			detail::lua_reference ref;
			ref.set(m_state);
			return detail::proxy_object(const_cast<object*>(this), ref);
		}

		// *****************************
		// OPERATOR =

		template<typename T>
		object& operator=(const T& val) const
		{
			assert((m_state != nullptr) && "you cannot assign a non-lua value to an uninitialized object");
			// you cannot assign a non-lua value to an uninitialized object

			detail::convert_to_lua(m_state, val);
			set();
			return const_cast<luabind::object&>(*this);
		}

		object& operator=(const object& o) const;
		object& operator=(const detail::proxy_object& o) const;
		object& operator=(const detail::proxy_raw_object& o) const;
		object& operator=(const detail::proxy_array_object& o) const;

		template<typename T, typename... Policies>
		void assign(const T& val, const detail::policy_cons<Policies...> p) const
		{
			assert((m_state != nullptr) && "you cannot assign a non-lua value to an uninitialized object");
			// you cannot assign a non-lua value to an uninitialized object

			detail::convert_to_lua_p(m_state, val, p);
			set();
		}

		// const overload should return a tuple_object..?
		inline detail::tuple_object_ref operator,(const object& rhs) const;

		// *****************************
		// OPERATOR()

        template<typename... Ts>
        decltype(auto) operator()(const Ts&... args) const
        {
            using caller_t = detail::proxy_caller<std::add_pointer_t<std::add_const_t<Ts>>...>;
            return caller_t(const_cast<luabind::object*>(this), std::make_tuple(&args...));
        }

		detail::proxy_object make_proxy(detail::lua_reference const& key)
		{
			return detail::proxy_object(this, key);
		}

		detail::proxy_raw_object make_raw_proxy(detail::lua_reference const& key)
		{
			return detail::proxy_raw_object(this, key);
		}

		detail::proxy_array_object make_array_proxy(int key)
		{
			return detail::proxy_array_object(this, key);
		}

		// TODO: it's not possible to make object friend with wrapped_constructor_helper::apply (since
		// it's an inner class), that's why this interface is public
//	private:

		object(lua_State* L, detail::lua_reference const& ref, bool/*, reference*/)
			: m_state(L)
			, m_ref(ref)
		{
		}

		inline detail::lua_reference get_index() const
		{
			return m_ref;
		}

private:

		void dummy() const {}

		void allocate_slot() const
		{
			if (!m_ref.is_valid())
			{
				lua_pushboolean(m_state, 0);
				m_ref.set(m_state);
			}
		}

#pragma warning(push)
#pragma warning(disable:4251)
		mutable detail::lua_reference m_ref;
		mutable lua_State* m_state;
#pragma warning(pop)
	};


	// *************************************
	// OBJECT

	inline void object::swap(object& rhs)
	{
		// you cannot swap objects from different lua states
		assert((lua_state() == rhs.lua_state()) && "you cannot swap objects from different lua states");
		m_ref.swap(rhs.m_ref);
	}

	inline object object::iterator::key() const
	{
		lua_State* L = m_obj->lua_state();
		return object(L, m_key, true);
	}

	inline object object::raw_iterator::key() const
	{
		lua_State* L = m_obj->lua_state();
		return object(L, m_key, true);
	}

	namespace detail
	{
		// tuple object ----------------------------------------------

		struct tuple_object;

		struct tuple_object_ref
		{
			tuple_object_ref(object* a, object* b)
				: n(2)
			{ refs[0] = a; refs[1] = b; }

			tuple_object_ref& operator,(const object& x)
			{ refs[n++] = const_cast<object*>(&x); return *this; }

			struct assign_into
			{
				assign_into() {}

				template<class T>
				assign_into(tuple_object_ref& to, const T& val)
					: target(&to)
					, n(0)
				{ 
					if (n >= target->n) return; 
					*target->refs[n++] = val; 
				}

				template<class T>
				assign_into& operator,(const T& val)
				{ 
					if (n >= target->n) return *this;
					*target->refs[n++] = val; 
					return *this;
				}

				tuple_object_ref* target = nullptr;
				std::size_t n = 0;
			};

			template<class T>
			assign_into operator=(const T& val)
			{ return assign_into(*this, val); }

			tuple_object_ref(const tuple_object_ref&);
			assign_into operator=(const tuple_object_ref& x)
			{
				for (std::size_t i = 0; i < n && i < x.n; ++i)
					*refs[i] = *x.refs[i];
				return assign_into();
			}

			inline assign_into operator=(const tuple_object&);

			std::size_t n;
			object* refs[10];
		};

		struct tuple_object
		{
			tuple_object(const object& x)
				: n(0)
			{ objs[n++] = x; }

			tuple_object(const tuple_object_ref& x)
			{
				for (std::size_t i = 0; i < x.n; ++i)
					objs[i] = *x.refs[i];

				n = x.n;
			}

			std::size_t n;
			object objs[10];
		};

		inline tuple_object_ref::assign_into tuple_object_ref::operator=(const tuple_object& x)
		{
			for (std::size_t i = 0; i < n && i < x.n; ++i)
				*refs[i] = x.objs[i];
			return assign_into();
		}

		// *************************************
		// PROXY CALLER

        template <typename... Ts>
		template<typename... Policies>
		inline luabind::object proxy_caller<Ts...>::operator[](const policy_cons<Policies...> p)
		{
			m_called = true;
			lua_State* L = m_obj->lua_state();
			m_obj->pushvalue();
			detail::push_args_from_tuple<1>::apply(L, m_args, p);
			if (pcall(L, sizeof...(Ts), 1))
			{ 
#ifndef LUABIND_NO_EXCEPTIONS
				throw error(L);
#else
				error_callback_fun e = get_error_callback();
				if (e) e(L);
	
				assert(0 && "the lua function threw an error and exceptions are disabled."
					"if you want to handle this error use luabind::set_error_callback()");
				// std::terminate();
#endif
			}
			detail::lua_reference ref;
			ref.set(L);
			return luabind::object(m_obj->lua_state(), ref, true/*luabind::object::reference()*/);
		}

		// *************************************
		// PROXY OBJECT

		template<typename T>
		inline object proxy_object::raw_at(const T& key)
		LUABIND_PROXY_RAW_AT_BODY

		template<class T>
		inline object proxy_object::at(const T& key)
		LUABIND_PROXY_AT_BODY

		inline lua_State* proxy_object::lua_state() const
		{
			return m_obj->lua_state();
		}

		inline proxy_object::operator luabind::object()
		{
			lua_State* L = m_obj->lua_state();
			pushvalue();
			detail::lua_reference ref;
			ref.set(L);
			return luabind::object(L, ref, true/*luabind::object::reference()*/);
		}


		// *************************************
		// PROXY ARRAY OBJECT

		template<class T>
		inline object proxy_array_object::raw_at(const T& key)
		LUABIND_PROXY_ARRAY_RAW_AT_BODY

		template<class T>
		inline object proxy_array_object::at(const T& key)
		LUABIND_PROXY_ARRAY_AT_BODY

#undef LUABIND_PROXY_ARRAY_AT_BODY
#undef LUABIND_PROXY_ARRAY_RAW_AT_BODY

		inline lua_State* proxy_array_object::lua_state() const
		{
			return m_obj->lua_state();
		}

		inline proxy_array_object::operator luabind::object()
		{
			lua_State* L = m_obj->lua_state();
			pushvalue();
			detail::lua_reference ref;
			ref.set(L);
			return luabind::object(L, ref, true/*luabind::object::reference()*/);
		}


		// *************************************
		// PROXY RAW OBJECT

		template<class T>
		inline object proxy_raw_object::raw_at(const T& key)
		LUABIND_PROXY_RAW_AT_BODY

		template<class T>
		inline object proxy_raw_object::at(const T& key)
		LUABIND_PROXY_AT_BODY

#undef LUABIND_PROXY_RAW_AT_BODY
#undef LUABIND_PROXY_AT_BODY

		inline lua_State* proxy_raw_object::lua_state() const
		{
			return m_obj->lua_state();
		}

		inline proxy_raw_object::operator luabind::object()
		{
			lua_State* L = lua_state();
			pushvalue();
			detail::lua_reference ref;
			ref.set(L);
			return luabind::object(L, ref, true);
		}


		// *************************************
		// PROXY CALLER
		template<typename... Ts>
		proxy_caller<Ts...>::~proxy_caller() LUABIND_DTOR_NOEXCEPT
		{
			if (m_called) return;

			m_called = true;
			lua_State* L = m_obj->lua_state();
			m_obj->pushvalue();

			push_args_from_tuple<1>::apply(L, m_args);
			if (pcall(L, sizeof...(Ts), 0))
			{ 
#ifndef LUABIND_NO_EXCEPTIONS
				throw luabind::error(L);
#else
				error_callback_fun e = get_error_callback();
				if (e) e(L);
                else
                {

                    assert(0 && "the lua function threw an error and exceptions are disabled."
                        "if you want to handle this error use luabind::set_error_callback()");
                    // std::terminate();
                }
#endif
			}
		}

		template<typename... Ts>
		proxy_caller<Ts...>::operator luabind::object()
		{
			m_called = true;
			lua_State* L = m_obj->lua_state();
			m_obj->pushvalue();

			push_args_from_tuple<1>::apply(L, m_args);
			if (pcall(L, sizeof...(Ts), 1))
			{ 
#ifndef LUABIND_NO_EXCEPTIONS
				throw luabind::error(L);
#else
				error_callback_fun e = get_error_callback();
				if (e) e(L);
	
				assert(0 && "the lua function threw an error and exceptions are disabled."
					"if you want to handle this error use luabind::set_error_callback()");
				// std::terminate();
#endif
			}
			detail::lua_reference ref;
			ref.set(L);
			return luabind::object(m_obj->lua_state(), ref, true/*luabind::object::reference()*/);
		}

	}

	inline detail::tuple_object_ref object::operator,(const object& rhs) const
	{
		return detail::tuple_object_ref(
			const_cast<object*>(this), const_cast<object*>(&rhs));
	}

	typedef detail::tuple_object function_;

#define LUABIND_DECLARE_OPERATOR(MACRO)\
	MACRO(object, object) \
	MACRO(object, detail::proxy_object) \
	MACRO(object, detail::proxy_array_object) \
	MACRO(object, detail::proxy_raw_object) \
	MACRO(detail::proxy_object, object) \
	MACRO(detail::proxy_object, detail::proxy_object) \
	MACRO(detail::proxy_object, detail::proxy_array_object) \
	MACRO(detail::proxy_object, detail::proxy_raw_object) \
	MACRO(detail::proxy_array_object, object) \
	MACRO(detail::proxy_array_object, detail::proxy_object) \
	MACRO(detail::proxy_array_object, detail::proxy_array_object) \
	MACRO(detail::proxy_array_object, detail::proxy_raw_object) \
	MACRO(detail::proxy_raw_object, object) \
	MACRO(detail::proxy_raw_object, detail::proxy_object) \
	MACRO(detail::proxy_raw_object, detail::proxy_array_object) \
	MACRO(detail::proxy_raw_object, detail::proxy_raw_object)


#define LUABIND_EQUALITY_OPERATOR(lhs, rhs) LUABIND_API bool operator==(const lhs&, const rhs&);
	LUABIND_DECLARE_OPERATOR(LUABIND_EQUALITY_OPERATOR)
#undef LUABIND_EQUALITY_OPERATOR

#define LUABIND_LESSTHAN_OPERATOR(lhs, rhs) LUABIND_API bool operator<(const lhs&, const rhs&);
	LUABIND_DECLARE_OPERATOR(LUABIND_LESSTHAN_OPERATOR)
#undef LUABIND_LESSTHAN_OPERATOR

#define LUABIND_LESSOREQUAL_OPERATOR(lhs_t, rhs_t) LUABIND_API bool operator<=(const lhs_t&, const rhs_t&);
	LUABIND_DECLARE_OPERATOR(LUABIND_LESSOREQUAL_OPERATOR)
#undef LUABIND_LESSOREQUAL_OPERATOR

#define LUABIND_INEQUALITY_OPERATOR(lhs_t, rhs_t)\
	inline bool operator!=(const rhs_t& rhs, const lhs_t& lhs) \
	{ \
		return !(rhs == lhs); \
	}

	LUABIND_DECLARE_OPERATOR(LUABIND_INEQUALITY_OPERATOR)

#undef LUABIND_INEQUALITY_OPERATOR

#define LUABIND_GREATEROREQUAL_OPERATOR(lhs_t, rhs_t)\
	inline bool operator>=(const rhs_t& rhs, const lhs_t& lhs) \
	{ \
		return !(rhs < lhs); \
	}

	LUABIND_DECLARE_OPERATOR(LUABIND_GREATEROREQUAL_OPERATOR)

#undef LUABIND_GREATEROREQUAL_OPERATOR

#define LUABIND_GREATERTHAN_OPERATOR(lhs_t, rhs_t)\
	inline bool operator>(const lhs_t& lhs, const rhs_t& rhs) \
	{ \
		return !(lhs <= rhs); \
	}

	LUABIND_DECLARE_OPERATOR(LUABIND_GREATERTHAN_OPERATOR)
#undef LUABIND_GREATERTHAN_OPERATOR

#undef LUABIND_DECLARE_OPERATOR

	inline void setmetatable(object const& obj, object const& metatable)
	{
		obj.get_index().get(obj.lua_state());
		detail::stack_pop pop(obj.lua_state(), 1);
		metatable.get_index().get(obj.lua_state());
		lua_setmetatable(obj.lua_state(), -2);
	}
}

namespace std
{

#define LUABIND_DEFINE_SWAP(t1,t2)\
	inline void swap(t1 lhs, t2 rhs)\
	{\
			assert((lhs.lua_state() == rhs.lua_state()) && "you cannot swap objects from different lua states");\
			rhs.pushvalue();\
			lhs.pushvalue();\
			rhs.set();\
			lhs.set();\
	}

	inline void swap(luabind::object& lhs, luabind::object& rhs)
	{
		lhs.swap(rhs);
	}

	// object against all other
	LUABIND_DEFINE_SWAP(luabind::object&, const luabind::detail::proxy_object&)
	LUABIND_DEFINE_SWAP(luabind::object&, const luabind::detail::proxy_raw_object&)
	LUABIND_DEFINE_SWAP(luabind::object&, const luabind::detail::proxy_array_object&)
	LUABIND_DEFINE_SWAP(const luabind::detail::proxy_object&, luabind::object&)
	LUABIND_DEFINE_SWAP(const luabind::detail::proxy_raw_object&, luabind::object&)
	LUABIND_DEFINE_SWAP(const luabind::detail::proxy_array_object&, luabind::object&)

	// proxy_object against all other
	LUABIND_DEFINE_SWAP(const luabind::detail::proxy_object&, const luabind::detail::proxy_object&)
	LUABIND_DEFINE_SWAP(const luabind::detail::proxy_object&, const luabind::detail::proxy_raw_object&)
	LUABIND_DEFINE_SWAP(const luabind::detail::proxy_object&, const luabind::detail::proxy_array_object&)
	LUABIND_DEFINE_SWAP(const luabind::detail::proxy_raw_object&, const luabind::detail::proxy_object&)
	LUABIND_DEFINE_SWAP(const luabind::detail::proxy_array_object&, const luabind::detail::proxy_object&)

	// proxy_raw_object against all other
	LUABIND_DEFINE_SWAP(const luabind::detail::proxy_raw_object&, const luabind::detail::proxy_raw_object&)
	LUABIND_DEFINE_SWAP(const luabind::detail::proxy_raw_object&, const luabind::detail::proxy_array_object&)
	LUABIND_DEFINE_SWAP(const luabind::detail::proxy_array_object&, const luabind::detail::proxy_raw_object&)

	// proxy_array_object against all other
	LUABIND_DEFINE_SWAP(const luabind::detail::proxy_array_object&, const luabind::detail::proxy_array_object&)

#undef LUABIND_DEFINE_SWAP

} // std
