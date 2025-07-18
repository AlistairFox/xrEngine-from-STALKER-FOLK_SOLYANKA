// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: google/protobuf/any.proto

#ifndef GOOGLE_PROTOBUF_INCLUDED_google_2fprotobuf_2fany_2eproto
#define GOOGLE_PROTOBUF_INCLUDED_google_2fprotobuf_2fany_2eproto

#include <limits>
#include <string>

#include <google/protobuf/port_def.inc>
#if PROTOBUF_VERSION < 3021000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers. Please update
#error your headers.
#endif
#if 3021008 < PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers. Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/port_undef.inc>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/arena.h>
#include <google/protobuf/arenastring.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/metadata_lite.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>  // IWYU pragma: export
#include <google/protobuf/extension_set.h>  // IWYU pragma: export
#include <google/protobuf/unknown_field_set.h>
// @@protoc_insertion_point(includes)
#include <google/protobuf/port_def.inc>
#define PROTOBUF_INTERNAL_EXPORT_google_2fprotobuf_2fany_2eproto PROTOBUF_EXPORT
PROTOBUF_NAMESPACE_OPEN
namespace internal {
class AnyMetadata;
}  // namespace internal
PROTOBUF_NAMESPACE_CLOSE

// Internal implementation detail -- do not use these members.
struct PROTOBUF_EXPORT TableStruct_google_2fprotobuf_2fany_2eproto {
  static const uint32_t offsets[];
};
PROTOBUF_EXPORT extern const ::PROTOBUF_NAMESPACE_ID::internal::DescriptorTable descriptor_table_google_2fprotobuf_2fany_2eproto;
PROTOBUF_NAMESPACE_OPEN
class Any;
struct AnyDefaultTypeInternal;
PROTOBUF_EXPORT extern AnyDefaultTypeInternal _Any_default_instance_;
PROTOBUF_NAMESPACE_CLOSE
PROTOBUF_NAMESPACE_OPEN
template<> PROTOBUF_EXPORT ::PROTOBUF_NAMESPACE_ID::Any* Arena::CreateMaybeMessage<::PROTOBUF_NAMESPACE_ID::Any>(Arena*);
PROTOBUF_NAMESPACE_CLOSE
PROTOBUF_NAMESPACE_OPEN

// ===================================================================

class PROTOBUF_EXPORT Any final :
    public ::PROTOBUF_NAMESPACE_ID::Message /* @@protoc_insertion_point(class_definition:google.protobuf.Any) */ {
 public:
  inline Any() : Any(nullptr) {}
  ~Any() override;
  explicit PROTOBUF_CONSTEXPR Any(::PROTOBUF_NAMESPACE_ID::internal::ConstantInitialized);

  Any(const Any& from);
  Any(Any&& from) noexcept
    : Any() {
    *this = ::std::move(from);
  }

  inline Any& operator=(const Any& from) {
    CopyFrom(from);
    return *this;
  }
  inline Any& operator=(Any&& from) noexcept {
    if (this == &from) return *this;
    if (GetOwningArena() == from.GetOwningArena()
  #ifdef PROTOBUF_FORCE_COPY_IN_MOVE
        && GetOwningArena() != nullptr
  #endif  // !PROTOBUF_FORCE_COPY_IN_MOVE
    ) {
      InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }

  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* descriptor() {
    return GetDescriptor();
  }
  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* GetDescriptor() {
    return default_instance().GetMetadata().descriptor;
  }
  static const ::PROTOBUF_NAMESPACE_ID::Reflection* GetReflection() {
    return default_instance().GetMetadata().reflection;
  }
  static const Any& default_instance() {
    return *internal_default_instance();
  }
  static inline const Any* internal_default_instance() {
    return reinterpret_cast<const Any*>(
               &_Any_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    0;

  // implements Any -----------------------------------------------

  bool PackFrom(const ::PROTOBUF_NAMESPACE_ID::Message& message) {
    GOOGLE_DCHECK_NE(&message, this);
    return _impl_._any_metadata_.PackFrom(GetArena(), message);
  }
  bool PackFrom(const ::PROTOBUF_NAMESPACE_ID::Message& message,
                ::PROTOBUF_NAMESPACE_ID::ConstStringParam type_url_prefix) {
    GOOGLE_DCHECK_NE(&message, this);
    return _impl_._any_metadata_.PackFrom(GetArena(), message, type_url_prefix);
  }
  bool UnpackTo(::PROTOBUF_NAMESPACE_ID::Message* message) const {
    return _impl_._any_metadata_.UnpackTo(message);
  }
  static bool GetAnyFieldDescriptors(
      const ::PROTOBUF_NAMESPACE_ID::Message& message,
      const ::PROTOBUF_NAMESPACE_ID::FieldDescriptor** type_url_field,
      const ::PROTOBUF_NAMESPACE_ID::FieldDescriptor** value_field);
  template <typename T, class = typename std::enable_if<!std::is_convertible<T, const ::PROTOBUF_NAMESPACE_ID::Message&>::value>::type>
  bool PackFrom(const T& message) {
    return _impl_._any_metadata_.PackFrom<T>(GetArena(), message);
  }
  template <typename T, class = typename std::enable_if<!std::is_convertible<T, const ::PROTOBUF_NAMESPACE_ID::Message&>::value>::type>
  bool PackFrom(const T& message,
                ::PROTOBUF_NAMESPACE_ID::ConstStringParam type_url_prefix) {
    return _impl_._any_metadata_.PackFrom<T>(GetArena(), message, type_url_prefix);}
  template <typename T, class = typename std::enable_if<!std::is_convertible<T, const ::PROTOBUF_NAMESPACE_ID::Message&>::value>::type>
  bool UnpackTo(T* message) const {
    return _impl_._any_metadata_.UnpackTo<T>(message);
  }
  template<typename T> bool Is() const {
    return _impl_._any_metadata_.Is<T>();
  }
  static bool ParseAnyTypeUrl(::PROTOBUF_NAMESPACE_ID::ConstStringParam type_url,
                              std::string* full_type_name);
  friend void swap(Any& a, Any& b) {
    a.Swap(&b);
  }
  inline void Swap(Any* other) {
    if (other == this) return;
  #ifdef PROTOBUF_FORCE_COPY_IN_SWAP
    if (GetOwningArena() != nullptr &&
        GetOwningArena() == other->GetOwningArena()) {
   #else  // PROTOBUF_FORCE_COPY_IN_SWAP
    if (GetOwningArena() == other->GetOwningArena()) {
  #endif  // !PROTOBUF_FORCE_COPY_IN_SWAP
      InternalSwap(other);
    } else {
      ::PROTOBUF_NAMESPACE_ID::internal::GenericSwap(this, other);
    }
  }
  void UnsafeArenaSwap(Any* other) {
    if (other == this) return;
    GOOGLE_DCHECK(GetOwningArena() == other->GetOwningArena());
    InternalSwap(other);
  }

  // implements Message ----------------------------------------------

  Any* New(::PROTOBUF_NAMESPACE_ID::Arena* arena = nullptr) const final {
    return CreateMaybeMessage<Any>(arena);
  }
  using ::PROTOBUF_NAMESPACE_ID::Message::CopyFrom;
  void CopyFrom(const Any& from);
  using ::PROTOBUF_NAMESPACE_ID::Message::MergeFrom;
  void MergeFrom( const Any& from) {
    Any::MergeImpl(*this, from);
  }
  private:
  static void MergeImpl(::PROTOBUF_NAMESPACE_ID::Message& to_msg, const ::PROTOBUF_NAMESPACE_ID::Message& from_msg);
  public:
  PROTOBUF_ATTRIBUTE_REINITIALIZES void Clear() final;
  bool IsInitialized() const final;

  size_t ByteSizeLong() const final;
  const char* _InternalParse(const char* ptr, ::PROTOBUF_NAMESPACE_ID::internal::ParseContext* ctx) final;
  uint8_t* _InternalSerialize(
      uint8_t* target, ::PROTOBUF_NAMESPACE_ID::io::EpsCopyOutputStream* stream) const final;
  int GetCachedSize() const final { return _impl_._cached_size_.Get(); }

  private:
  void SharedCtor(::PROTOBUF_NAMESPACE_ID::Arena* arena, bool is_message_owned);
  void SharedDtor();
  void SetCachedSize(int size) const final;
  void InternalSwap(Any* other);

  private:
  friend class ::PROTOBUF_NAMESPACE_ID::internal::AnyMetadata;
  static ::PROTOBUF_NAMESPACE_ID::StringPiece FullMessageName() {
    return "google.protobuf.Any";
  }
  protected:
  explicit Any(::PROTOBUF_NAMESPACE_ID::Arena* arena,
                       bool is_message_owned = false);
  public:

  static const ClassData _class_data_;
  const ::PROTOBUF_NAMESPACE_ID::Message::ClassData*GetClassData() const final;

  ::PROTOBUF_NAMESPACE_ID::Metadata GetMetadata() const final;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  enum : int {
    kTypeUrlFieldNumber = 1,
    kValueFieldNumber = 2,
  };
  // string type_url = 1;
  void clear_type_url();
  const std::string& type_url() const;
  template <typename ArgT0 = const std::string&, typename... ArgT>
  void set_type_url(ArgT0&& arg0, ArgT... args);
  std::string* mutable_type_url();
  PROTOBUF_NODISCARD std::string* release_type_url();
  void set_allocated_type_url(std::string* type_url);
  private:
  const std::string& _internal_type_url() const;
  inline PROTOBUF_ALWAYS_INLINE void _internal_set_type_url(const std::string& value);
  std::string* _internal_mutable_type_url();
  public:

  // bytes value = 2;
  void clear_value();
  const std::string& value() const;
  template <typename ArgT0 = const std::string&, typename... ArgT>
  void set_value(ArgT0&& arg0, ArgT... args);
  std::string* mutable_value();
  PROTOBUF_NODISCARD std::string* release_value();
  void set_allocated_value(std::string* value);
  private:
  const std::string& _internal_value() const;
  inline PROTOBUF_ALWAYS_INLINE void _internal_set_value(const std::string& value);
  std::string* _internal_mutable_value();
  public:

  // @@protoc_insertion_point(class_scope:google.protobuf.Any)
 private:
  class _Internal;

  template <typename T> friend class ::PROTOBUF_NAMESPACE_ID::Arena::InternalHelper;
  typedef void InternalArenaConstructable_;
  typedef void DestructorSkippable_;
  struct Impl_ {
    ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr type_url_;
    ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr value_;
    mutable ::PROTOBUF_NAMESPACE_ID::internal::CachedSize _cached_size_;
    ::PROTOBUF_NAMESPACE_ID::internal::AnyMetadata _any_metadata_;
  };
  union { Impl_ _impl_; };
  friend struct ::TableStruct_google_2fprotobuf_2fany_2eproto;
};
// ===================================================================


// ===================================================================

#ifdef __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif  // __GNUC__
// Any

// string type_url = 1;
inline void Any::clear_type_url() {
  _impl_.type_url_.ClearToEmpty();
}
inline const std::string& Any::type_url() const {
  // @@protoc_insertion_point(field_get:google.protobuf.Any.type_url)
  return _internal_type_url();
}
template <typename ArgT0, typename... ArgT>
inline PROTOBUF_ALWAYS_INLINE
void Any::set_type_url(ArgT0&& arg0, ArgT... args) {
 
 _impl_.type_url_.Set(static_cast<ArgT0 &&>(arg0), args..., GetArenaForAllocation());
  // @@protoc_insertion_point(field_set:google.protobuf.Any.type_url)
}
inline std::string* Any::mutable_type_url() {
  std::string* _s = _internal_mutable_type_url();
  // @@protoc_insertion_point(field_mutable:google.protobuf.Any.type_url)
  return _s;
}
inline const std::string& Any::_internal_type_url() const {
  return _impl_.type_url_.Get();
}
inline void Any::_internal_set_type_url(const std::string& value) {
  
  _impl_.type_url_.Set(value, GetArenaForAllocation());
}
inline std::string* Any::_internal_mutable_type_url() {
  
  return _impl_.type_url_.Mutable(GetArenaForAllocation());
}
inline std::string* Any::release_type_url() {
  // @@protoc_insertion_point(field_release:google.protobuf.Any.type_url)
  return _impl_.type_url_.Release();
}
inline void Any::set_allocated_type_url(std::string* type_url) {
  if (type_url != nullptr) {
    
  } else {
    
  }
  _impl_.type_url_.SetAllocated(type_url, GetArenaForAllocation());
#ifdef PROTOBUF_FORCE_COPY_DEFAULT_STRING
  if (_impl_.type_url_.IsDefault()) {
    _impl_.type_url_.Set("", GetArenaForAllocation());
  }
#endif // PROTOBUF_FORCE_COPY_DEFAULT_STRING
  // @@protoc_insertion_point(field_set_allocated:google.protobuf.Any.type_url)
}

// bytes value = 2;
inline void Any::clear_value() {
  _impl_.value_.ClearToEmpty();
}
inline const std::string& Any::value() const {
  // @@protoc_insertion_point(field_get:google.protobuf.Any.value)
  return _internal_value();
}
template <typename ArgT0, typename... ArgT>
inline PROTOBUF_ALWAYS_INLINE
void Any::set_value(ArgT0&& arg0, ArgT... args) {
 
 _impl_.value_.SetBytes(static_cast<ArgT0 &&>(arg0), args..., GetArenaForAllocation());
  // @@protoc_insertion_point(field_set:google.protobuf.Any.value)
}
inline std::string* Any::mutable_value() {
  std::string* _s = _internal_mutable_value();
  // @@protoc_insertion_point(field_mutable:google.protobuf.Any.value)
  return _s;
}
inline const std::string& Any::_internal_value() const {
  return _impl_.value_.Get();
}
inline void Any::_internal_set_value(const std::string& value) {
  
  _impl_.value_.Set(value, GetArenaForAllocation());
}
inline std::string* Any::_internal_mutable_value() {
  
  return _impl_.value_.Mutable(GetArenaForAllocation());
}
inline std::string* Any::release_value() {
  // @@protoc_insertion_point(field_release:google.protobuf.Any.value)
  return _impl_.value_.Release();
}
inline void Any::set_allocated_value(std::string* value) {
  if (value != nullptr) {
    
  } else {
    
  }
  _impl_.value_.SetAllocated(value, GetArenaForAllocation());
#ifdef PROTOBUF_FORCE_COPY_DEFAULT_STRING
  if (_impl_.value_.IsDefault()) {
    _impl_.value_.Set("", GetArenaForAllocation());
  }
#endif // PROTOBUF_FORCE_COPY_DEFAULT_STRING
  // @@protoc_insertion_point(field_set_allocated:google.protobuf.Any.value)
}

#ifdef __GNUC__
  #pragma GCC diagnostic pop
#endif  // __GNUC__

// @@protoc_insertion_point(namespace_scope)

PROTOBUF_NAMESPACE_CLOSE

// @@protoc_insertion_point(global_scope)

#include <google/protobuf/port_undef.inc>
#endif  // GOOGLE_PROTOBUF_INCLUDED_GOOGLE_PROTOBUF_INCLUDED_google_2fprotobuf_2fany_2eproto
