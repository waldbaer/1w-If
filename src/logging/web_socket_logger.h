#ifndef OWIF_LOGGER_WEB_SOCKET_LOGGER_H
#define OWIF_LOGGER_WEB_SOCKET_LOGGER_H

// ---- Includes ----
#include <Arduino.h>

#include "AsyncWebSocket.h"

namespace owif {
namespace logging {

class WebSocketLogger : public Print {
 public:
  static constexpr std::uint8_t kMaxNumberOfSinks{4};

  WebSocketLogger() = default;
  WebSocketLogger(WebSocketLogger const&) = default;
  WebSocketLogger(WebSocketLogger&&) = default;
  auto operator=(WebSocketLogger const&) -> WebSocketLogger& = default;
  auto operator=(WebSocketLogger&&) -> WebSocketLogger& = default;
  ~WebSocketLogger() = default;

  // ---- Public APIs --------------------------------------------------------------------------------------------------
  auto Begin(AsyncWebSocket& websocket) -> bool;

  // ---- Public APIs: Print Interface ---------------------------------------------------------------------------------

  auto write(std::uint8_t character) -> std::size_t final;
  auto write(std::uint8_t const* buffer, std::size_t size) -> std::size_t final;

  auto LogFullHistory(AsyncWebSocketClient* client) -> void;

 private:
  static constexpr std::uint8_t kMaxHistorySize{WS_MAX_QUEUED_MESSAGES};
  // Maximum length of a single stored history entry (raw log line). Longer lines will be truncated.
  static constexpr std::size_t kMaxHistoryEntryLength{256};

  // Stores the raw log line (without JSON serialization). The stored line will be truncated to
  // kMaxHistoryEntryLength to avoid excessive heap usage.
  auto StoreHistory(String const& raw_log_line) -> void;

  bool started_{false};
  AsyncWebSocket* web_socket_{nullptr};

  String line_buffer_{};

  std::array<String, kMaxHistorySize> history_buffer_{};
  std::uint8_t history_start_{0};
  std::uint8_t history_count_{0};
};

/*!
 * \brief Declaration of global WebSocketLogger instance.
 */
extern WebSocketLogger web_socket_logger_g;

}  // namespace logging
}  // namespace owif

#endif  // OWIF_LOGGER_WEB_SOCKET_LOGGER_H
