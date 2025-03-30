export module webpp:network;

import :js_object;
import :event_loop;
import :coroutine;

namespace webpp {

[[clang::import_module("webpp"), clang::import_name("fetch")]]
void fetch(const char* url, std::size_t url_size, void* callback_data);

[[clang::import_module("webpp"), clang::import_name("response_text")]]
void response_text(js_handle handle, void* callback_data);

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

    bool ok() {
        return *(*this)["ok"].as<bool>();
    }

    void text(std::function<void(std::string_view)> callback) {
        callback_data* data = new callback_data{[f = std::move(callback)](js_handle data) {
            f(std::string_view{reinterpret_cast<const char*>(data)});
        }, true};
        response_text(handle(), data);
    }

    struct text_awaiter : coro::generic_awaiter<std::string> {
        text_awaiter(response& res) {
            callback = new callback_data{[this](js_handle data) {
                result = std::string{reinterpret_cast<const char*>(data)};
                try_resume();
            }, true};
            response_text(res.handle(), callback);
        }
        text_awaiter(text_awaiter&& other) : coro::generic_awaiter<std::string>{std::move(other)} {
            callback->replace([this](js_handle data) {
                result = std::string{reinterpret_cast<const char*>(data)};
                try_resume();
            });
        }
    };
    text_awaiter text() {
        return text_awaiter{*this};
    }
    text_awaiter co_text() {
        return text_awaiter{*this};
    }
};

export void fetch(std::string_view url, std::function<void(response)> callback) {
    callback_data* data = new callback_data{[f = std::move(callback)](js_handle data) {
        f(response{data});
    }, true};
    fetch(url.data(), url.size(), data);
}

namespace coro {
struct fetch_awaiter : generic_awaiter<response> {
    fetch_awaiter(std::string_view url) {
        callback = new callback_data{[this](js_handle data) {
            result = response{data};
            try_resume();
        }, true};
        webpp::fetch(url.data(), url.size(), callback);
    }
    fetch_awaiter(fetch_awaiter&& other) : generic_awaiter<response>{std::move(other)} {
        callback->replace([this](js_handle data) {
            result = response{data};
            try_resume();
        });
    }
};
export using fetch = fetch_awaiter;
}

}
