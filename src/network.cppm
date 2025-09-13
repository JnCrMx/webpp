export module webpp:network;

import :js_object;
import :event_loop;
import :coroutine;

#include "macros.hpp"

namespace webpp {

[[clang::import_module("webpp"), clang::import_name("fetch")]]
void fetch(const char* url, std::size_t url_size, js_handle options, void* callback_data);

[[clang::import_module("webpp"), clang::import_name("response_text")]]
void response_text(js_handle handle, void* callback_data);

[[clang::import_module("webpp"), clang::import_name("response_bytes")]]
void response_bytes(js_handle handle, void* callback_data);

export class headers : public js_object {
public:
    static std::expected<headers, error_code> create(js_handle handle) {
        if(!check_instanceof(handle, "Headers")) {
            return std::unexpected(error_code::invalid_type);
        }
        return headers(handle);
    }

    headers() : js_object() {}

    headers(js_handle handle) : js_object(handle) {}
    headers(const headers&) = delete;
    headers(headers&& other) : js_object(std::move(other)) {}
    virtual ~headers() {}

    headers& operator=(const headers&) = delete;
    headers& operator=(headers&& other) {
        js_object::operator=(std::move(other));
        return *this;
    }

    // TODO
};

export class response : public js_object {
public:
    static std::expected<response, error_code> create(js_handle handle) {
        if(!check_instanceof(handle, "Response")) {
            return std::unexpected(error_code::invalid_type);
        }
        return response(handle);
    }

    response() : js_object() {}

    response(js_handle handle) : js_object(handle) {}
    response(const response&) = delete;
    response(response&& other) : js_object(std::move(other)) {}
    virtual ~response() {}

    response& operator=(const response&) = delete;
    response& operator=(response&& other) {
        js_object::operator=(std::move(other));
        return *this;
    }

    PROPERTY_READ_ONLY(body_used, "bodyUsed", bool);
    PROPERTY_READ_ONLY(headers, "headers", webpp::headers);
    PROPERTY_READ_ONLY(ok, "ok", bool);
    PROPERTY_READ_ONLY(redirected, "redirected", bool);
    PROPERTY_READ_ONLY(status, "status", int);
    PROPERTY_READ_ONLY(status_text, "statusText", std::string);
    PROPERTY_READ_ONLY(type, "type", std::string);
    PROPERTY_READ_ONLY(url, "url", std::string);

    void text(std::function<void(std::string_view)> callback) {
        callback_data* data = new callback_data{[f = std::move(callback)](js_handle, std::string_view data) {
            f(data);
        }, true};
        response_text(handle(), data);
    }
    void bytes(std::function<void(std::span<const char>)> callback) {
        callback_data* data = new callback_data{[f = std::move(callback)](js_handle, std::string_view data) {
            f({const_cast<char*>(data.data()), data.size()});
        }, true};
        response_bytes(handle(), data);
    }

    struct text_awaiter : coro::generic_awaiter<std::string> {
        auto create_callback() {
            return [this](js_handle, std::string_view data) {
                complete(std::string{data});
            };
        }

        text_awaiter(response& res) {
            callback = new callback_data{create_callback(), true};
            response_text(res.handle(), callback);
        }
        text_awaiter(text_awaiter&& other) : coro::generic_awaiter<std::string>{std::move(other)} {
            callback->replace(create_callback());
        }
    };
    text_awaiter co_text() {
        return text_awaiter{*this};
    }
    text_awaiter text() { return co_text(); } // for convenience

    struct bytes_awaiter : coro::generic_awaiter<std::vector<char>> {
        auto create_callback() {
            return [this](js_handle, std::string_view data) {
                const char* ptr = reinterpret_cast<const char*>(data.data());
                complete(std::vector<char>{ptr, ptr + data.size()});
            };
        }

        bytes_awaiter(response& res) {
            callback = new callback_data{create_callback(), true};
            response_bytes(res.handle(), callback);
        }
        bytes_awaiter(bytes_awaiter&& other) : coro::generic_awaiter<std::vector<char>>{std::move(other)} {
            callback->replace(create_callback());
        }
    };
    bytes_awaiter co_bytes() {
        return bytes_awaiter{*this};
    }
    bytes_awaiter bytes() { return co_bytes(); } // for convenience
};

export void fetch(std::string_view url, std::function<void(response)> callback, const js_object& options = {}) {
    callback_data* data = new callback_data{[f = std::move(callback)](js_handle data, std::string_view) {
        f(response{data});
    }, true};
    fetch(url.data(), url.size(), options.handle(), data);
}

namespace coro {
struct fetch_awaiter : generic_awaiter<response> {
    fetch_awaiter(std::string_view url, const js_object& options = {}) {
        callback = new callback_data{[this](js_handle data, std::string_view) {
            result = response{data};
            try_resume();
        }, true};
        webpp::fetch(url.data(), url.size(), options.handle(), callback);
    }
    fetch_awaiter(fetch_awaiter&& other) : generic_awaiter<response>{std::move(other)} {
        callback->replace([this](js_handle data, std::string_view) {
            result = response{data};
            try_resume();
        });
    }
};
export using fetch = fetch_awaiter;
}

}
