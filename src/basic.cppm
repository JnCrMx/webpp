export module webpp:basic;

import std;
import :js_object;

namespace webpp {

[[clang::import_module("webpp"), clang::import_name("log")]]
void log_raw(const char* message, std::size_t len);

[[clang::import_module("webpp"), clang::import_name("eval")]]
js_handle eval_raw(const char* message, std::size_t len);

export void log(std::string_view message) {
    log_raw(message.data(), message.size());
}
export template<class... Args>
void log(std::format_string<Args...> fmt, Args&&... args) {
    std::string s = std::format(fmt, std::forward<Args>(args)...);
    log_raw(s.data(), s.size());
}

export js_object eval(std::string_view code) {
    return js_object(eval_raw(code.data(), code.size()));
}
export template<class... Args>
js_object eval(std::format_string<Args...> fmt, Args&&... args) {
    std::string s = std::format(fmt, std::forward<Args>(args)...);
    return js_object(eval_raw(s.data(), s.size()));
}

}
