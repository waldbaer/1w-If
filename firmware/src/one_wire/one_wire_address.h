#ifndef OWIF_ONE_WIRE_ONE_WIRE_ADDRESS_H
#define OWIF_ONE_WIRE_ONE_WIRE_ADDRESS_H

#include <WString.h>

#include <cstdint>
#include <memory>

namespace owif {
namespace one_wire {

/*!
 * \brief Representation of 1-Wire Address.
 */
class OneWireAddress {
 public:
  using FamilyCode = std::uint8_t;

  static auto FromOwfsFormat(String const& address) -> std::unique_ptr<OneWireAddress>;

  OneWireAddress() = default;
  explicit OneWireAddress(std::uint64_t addr);

  OneWireAddress(OneWireAddress const&) = default;
  OneWireAddress(OneWireAddress&&) = default;
  auto operator=(OneWireAddress const&) -> OneWireAddress& = default;
  auto operator=(OneWireAddress&&) -> OneWireAddress& = default;

  auto GetFullAddress() const -> std::uint64_t;
  auto GetFamilyCode() const -> FamilyCode;
  auto GetCrc() const -> std::uint8_t;
  auto Format() const -> String;

  auto operator==(OneWireAddress const& other) const -> bool;
  auto operator==(std::uint64_t const& other) const -> bool;

  auto operator!=(OneWireAddress const& other) const -> bool;
  auto operator!=(std::uint64_t const& other) const -> bool;

  auto operator<(OneWireAddress const& other) const -> bool;

 private:
  std::uint64_t address_;
};

}  // namespace one_wire
}  // namespace owif

#endif  // OWIF_ONE_WIRE_ONE_WIRE_ADDRESS_H
