#ifndef OWIF_UTIL_ABORT_HANDLER_H
#define OWIF_UTIL_ABORT_HANDLER_H

#include "logging/logger.h"

extern "C" [[noreturn]] auto abort() -> void;

[[noreturn]] auto TerminateHandler() -> void;

auto SetupTerminateHandler() -> void;

#endif  // OWIF_UTIL_ABORT_HANDLER_H
