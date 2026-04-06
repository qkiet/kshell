#ifndef KSH_DEBUG_LOGGER_H
#define KSH_DEBUG_LOGGER_H

#include <iostream>

class DebugLogger {
  public:
    static DebugLogger &get() {
        static DebugLogger inst;
        return inst;
    }

    static void configure(bool enabled) { get().mEnabled = enabled; }

    template <typename... Args>
    static void print(Args &&...args) {
        if (!get().mEnabled) {
            return;
        }
        (std::cout << ... << std::forward<Args>(args)) << std::endl;
    }

    template <typename... Args>
    static void error(Args &&...args) {
        if (!get().mEnabled) {
            return;
        }
        (std::cerr << ... << std::forward<Args>(args)) << std::endl;
    }

  private:
    DebugLogger() = default;
    DebugLogger(const DebugLogger &) = delete;
    DebugLogger &operator=(const DebugLogger &) = delete;
    bool mEnabled = true;
};

#endif // KSH_DEBUG_LOGGER_H
