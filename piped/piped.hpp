#ifndef PIPED_HPP
#define PIPED_HPP

#include <concepts>
#include <type_traits>
#include <utility>

namespace piped_impl {

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

struct PipeT {};

template <typename T>
concept any_pipe = std::derived_from<T, PipeT>;

}  // namespace piped_impl

namespace piped {

struct UniversalParameter {
  template <auto W = [] {}>
  decltype(auto) Get() {
    using ActualT = typename decltype(Injector(
        piped_impl::Flag<
            piped_impl::PipeT,
            piped_impl::Reader<piped_impl::PipeT, 1, W>()>{}))::type;
    using PtrT = std::add_pointer_t<std::remove_reference_t<ActualT>>;
    if constexpr (std::is_const_v<std::remove_reference_t<ActualT>>) {
      return std::forward<ActualT>(*reinterpret_cast<PtrT>(value.cv));
    } else {
      return std::forward<ActualT>(*reinterpret_cast<PtrT>(value.v));
    }
  }

  template <typename T, auto W = [] {}>
  auto operator[](T&& value) {
    piped_impl::TypeWriter<T, piped_impl::PipeT, W>();

    if constexpr (std::is_const_v<T>) {
      this->value.cv = &value;
    } else {
      this->value.v = &value;
    }
    struct LocalPipe : piped_impl::PipeT {
      explicit LocalPipe(T& value)
          : value(value) {
      }
      operator T&&() {  // NOLINT
        return value;
      }
      T value;
    };
    return LocalPipe(value);
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

template <piped_impl::any_pipe T, typename U, auto W = [] {}>
auto operator||(T&& lhs, U&& rhs) {
  piped_impl::TypeWriter<U, piped_impl::PipeT, W>();
  if constexpr (std::is_const_v<std::remove_reference_t<U>>) {
    $.value.cv = &rhs;
  } else {
    $.value.v = &rhs;
  }

  struct LocalPipe : piped_impl::PipeT {
    explicit LocalPipe(U&& value)
        : value(std::forward<U>(value)) {
    }
    operator U&&() {  // NOLINT
      return std::forward<U>(value);
    }
    U value;
  };
  return LocalPipe(std::move(rhs));
}

}  // namespace piped

#endif  // PIPED_HPP