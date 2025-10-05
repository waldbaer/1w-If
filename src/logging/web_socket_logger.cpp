#include "logging/web_socket_logger.h"

#include <Arduino.h>

#include "web_server/web_socket_protocol.h"

namespace owif {
namespace logging {

// ---- Public APIs ----------------------------------------------------------------------------------------------------

auto WebSocketLogger::Begin(AsyncWebSocket& websocket) -> bool {
  bool result{false};
  if (not started_) {
    web_socket_ = &websocket;
    started_ = true;
    result = true;
  }
  return result;
}

auto WebSocketLogger::write(std::uint8_t character) -> std::size_t {
  if (started_) {
    line_buffer_ += (char)character;
    if (character == '\n') {
      // Store the raw line (without JSON wrapper) to keep history compact.
      StoreHistory(line_buffer_);

      // Serialize the complete line once and send.
      String const serialized_log_json{web_server::web_socket::WebSocketProtocol::SerializeLog(line_buffer_)};
      web_socket_->textAll(serialized_log_json);
      line_buffer_.clear();
    }
    return 1;
  }
  return 0;
}

auto WebSocketLogger::write(std::uint8_t const* buffer, std::size_t size) -> std::size_t {
  for (std::size_t i{0}; i < size; i++) {
    write(buffer[i]);
  };
  return size;
}

auto WebSocketLogger::LogFullHistory(AsyncWebSocketClient* client) -> void {
  for (std::uint8_t i{0}; i < history_count_; i++) {
    if (!web_socket_->availableForWrite(client->id())) {
      break;
    }

    std::uint8_t const history_buffer_index{static_cast<std::uint8_t>((history_start_ + i) % kMaxHistorySize)};
    // history_buffer_ stores raw lines; serialize before sending so the client receives
    // the same JSON message format as live logs.
    String const serialized_log_json{
        web_server::web_socket::WebSocketProtocol::SerializeLog(history_buffer_[history_buffer_index])};
    client->text(serialized_log_json);
  }
}

// ---- Private APIs ---------------------------------------------------------------------------------------------------

auto WebSocketLogger::StoreHistory(String const& log) -> void {
  // Truncate long lines to avoid unbounded memory usage and reduce heap fragmentation.
  String stored{log};
  if (stored.length() > static_cast<int>(kMaxHistoryEntryLength)) {
    stored = stored.substring(0, static_cast<int>(kMaxHistoryEntryLength));
  }

  if (history_count_ < kMaxHistorySize) {
    history_buffer_[(history_start_ + history_count_) % kMaxHistorySize] = stored;
    history_count_++;
  } else {
    history_buffer_[history_start_] = stored;
    history_start_ = (history_start_ + 1) % kMaxHistorySize;
  }
}

// ---- Global WebSocketLogger Instance ----
WebSocketLogger web_socket_logger_g{};

}  // namespace logging
}  // namespace owif
