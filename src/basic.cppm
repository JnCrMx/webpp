export module webpp:basic;

import std;

namespace webpp {

[[clang::import_module("webpp"), clang::import_name("log")]]
void log_raw(const char* message, std::size_t len);

export void log(std::string_view message) {
    log_raw(message.data(), message.size());
}
export template<class... Args>
void log(std::format_string<Args...> fmt, Args&&... args) {
    std::string s = std::format(fmt, std::forward<Args>(args)...);
    log_raw(s.data(), s.size());
}

}
