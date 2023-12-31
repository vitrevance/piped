#ifndef PIPED_HPP
#define PIPED_HPP

#include <concepts>
#include <type_traits>
#include <utility>

namespace piped {

namespace impl {

template <typename Scope, int N>
struct Flag {
  friend constexpr auto Injector(Flag<Scope, N>);
};

template <typename Scope, int N, typename T>
struct Writer {
  friend constexpr auto Injector(Flag<Scope, N>) {
    return std::type_identity<T>{};
  }
};

template <typename Scope, int N = 1, auto W = [] {}>
consteval auto Reader() {
  if constexpr (requires { Injector(Flag<Scope, N>{}); }) {
    return Reader<Scope, N + 1, [] {
      W();
    }>();
  } else {
    return N - 1;
  }
}

template <typename T, typename Scope, auto W>
auto TypeWriter() {
  if constexpr (Reader<Scope, 1, W>() == 0) {
    return Writer<Scope, 1, T>{};
  } else {
    return Writer<Scope, Reader<Scope, 1, W>() + 1, T>{};
  }
}

using PipeT = std::type_identity<decltype([] {})>;

}  // namespace impl

using LocalPipeT = decltype([] {
  struct LocalPipe : impl::PipeT {};
  return LocalPipe{};
});

struct UniversalParameter {
  template <auto W = [] {}>
  decltype(auto) Get() {
    using ActualT = typename decltype(Injector(
        impl::Flag<impl::PipeT, impl::Reader<impl::PipeT, 1, W>()>{}))::type;
    using PtrT = std::add_pointer_t<std::remove_reference_t<ActualT>>;
    if constexpr (std::is_const_v<std::remove_reference_t<ActualT>>) {
      return std::forward<ActualT>(*reinterpret_cast<PtrT>(value.cv));
    } else {
      return std::forward<ActualT>(*reinterpret_cast<PtrT>(value.v));
    }
  }

  template <typename T, auto W = [] {}>
  auto operator[](T&& value) {
    impl::TypeWriter<T, impl::PipeT, W>();

    if constexpr (std::is_const_v<std::remove_reference_t<T>>) {
      this->value.cv = &value;
    } else {
      this->value.v = &value;
    }

    return LocalPipeT{};
  }
  union {
    void* v;
    const void* cv;
  } value{nullptr};
} inline thread_local $;

template <auto W = [] {}>
decltype(auto) operator!(UniversalParameter param) {
  return param.Get<W>();
}

template <typename U, auto W = [] {}>
auto operator||(LocalPipeT&& lhs, U&& rhs) {
  impl::TypeWriter<U, impl::PipeT, W>();
  if constexpr (std::is_const_v<std::remove_reference_t<U>>) {
    $.value.cv = &rhs;
  } else {
    $.value.v = &rhs;
  }

  return LocalPipeT{};
}

template <auto W = [] {}>
static constexpr auto operator||(LocalPipeT&& lhs, UniversalParameter& rhs) {
  return rhs.Get<W>();
}

}  // namespace piped

#endif  // PIPED_HPP
