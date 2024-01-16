#ifndef PIPED_HPP
#define PIPED_HPP

#include <concepts>
#include <type_traits>
#include <utility>

#if defined(__GNUG__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnon-template-friend"
#else
#warning This version of the Piped library is for GCC
#endif

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

struct UniversalParameter {
  template <auto W = [] {}>
  decltype(auto) Get() const {
    using ActualT = typename decltype(Injector(
        impl::Flag<impl::PipeT, impl::Reader<impl::PipeT, 1, W>()>{}))::type;
    using PtrT = decltype(ActualT{}.template operator()<void*>(
        nullptr, std::declval<void*&>()))::type;
    auto extract = []() -> PtrT&& {
      std::remove_reference_t<PtrT>* place;
      ActualT{}(nullptr, place);
      return static_cast<PtrT&&>(*place);
    };
    return extract();
  }

  template <typename T, auto W = [] {}>
  auto operator[](T&& value) const {
    using PureT = std::remove_reference_t<T>;
    auto l = []<typename LT = PureT*>(PureT * in_val,
                                      std::type_identity<LT>::type & out_val)
                 ->std::type_identity<T> {
      if constexpr (std::same_as<LT, PureT*>) {
        static PureT* static_ptr = nullptr;
        out_val = static_ptr;
        static_ptr = in_val;
      }
      return {};
    };
    impl::TypeWriter<decltype(l), impl::PipeT, W>();
    PureT* ptr = nullptr;
    l(&value, ptr);
    return impl::PipeT{};
  }
} constexpr $;

template <auto W = [] {}>
decltype(auto) operator!(UniversalParameter param) {
  return param.Get<W>();
}

namespace impl {

template <typename U, auto W = [] {}>
auto operator||(impl::PipeT&& lhs, U&& rhs) {
  return $.operator[]<U, W>(std::forward<U>(rhs));
}

template <auto W = [] {}>
static constexpr decltype(auto) operator||(impl::PipeT&& lhs,
                                           const UniversalParameter& rhs) {
  return rhs.Get<W>();
}

}  // namespace impl

}  // namespace piped

#if defined(__GNUG__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif

#endif  // PIPED_HPP
