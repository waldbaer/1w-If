#ifndef OWIF_UTIL_LANGUAGE_H
#define OWIF_UTIL_LANGUAGE_H

#include <memory>
#include <type_traits>

namespace std {

template <typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
  return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

template <class T>
using underlying_type_t = typename underlying_type<T>::type;

}  // namespace std

// ---- Common type-trait utilities ----

template <typename Enum>
constexpr std::underlying_type_t<Enum> ToUnderlying(Enum e) noexcept {
  static_assert(std::is_enum<Enum>::value, "Template parameter must be an enum type.");
  return static_cast<std::underlying_type_t<Enum>>(e);
}

#endif  // OWIF_UTIL_LANGUAGE_H
