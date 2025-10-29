#include "util/abort_handler.h"

#include <exception>

#include "logging/logger.h"

extern "C" [[noreturn]] auto abort() -> void {
  owif::logging::logger_g.Abort(F("Aborting further execution. abort() called."));
}

[[noreturn]] auto TerminateHandler() -> void {
  owif::logging::logger_g.Abort(F("Terminating further execution. std::terminate() called."));
}

auto SetupTerminateHandler() -> void { std::set_terminate(TerminateHandler); }
