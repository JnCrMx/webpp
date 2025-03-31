export module webpp:event_loop;

import std;
import :js_object;

#include "macros.hpp"

namespace webpp {

[[clang::import_module("webpp"), clang::import_name("set_timeout")]]
void set_timeout(unsigned long millis, void* callback_data);

export using event_callback = std::function<void(js_handle, std::string_view)>;
export class callback_data {
    public:
        void abandon() {
            callback = nullptr;
        }
        void replace(event_callback new_callback) {
            callback = new_callback;
        }
        callback_data(event_callback callback, bool once) : callback{callback}, once{once} {}
    private:
        friend void callback(void* ptr, js_handle data, const char* string, std::size_t length);

        event_callback callback;
        bool once = false;
};

[[clang::export_name("webpp::callback")]]
void callback(void* ptr, js_handle data, const char* string, std::size_t length) {
    callback_data* cb = static_cast<callback_data*>(ptr);
    if(cb->callback) {
        cb->callback(data, std::string_view{string, length});
    }
    if(cb->once) {
        delete cb;
    }
}

export callback_data* set_timeout(std::chrono::milliseconds duration, std::function<void()> callback) {
    callback_data* data = new callback_data{[f = std::move(callback)](js_handle, std::string_view) {
        f();
    }, true};
    set_timeout(duration.count(), data);
    return data;
}

export class event : public js_object {
public:
    static std::expected<event, error_code> create(js_handle handle) {
        if(!check_instanceof(handle, "Event")) {
            return std::unexpected(error_code::invalid_type);
        }
        return event(handle);
    }

    event() : js_object() {}

    event(js_handle handle) : js_object(handle) {}
    event(const event&) = delete;
    event(event&& other) : js_object(std::move(other)) {}
    virtual ~event() {}

    event& operator=(const event&) = delete;
    event& operator=(event&& other) {
        js_object::operator=(std::move(other));
        return *this;
    }

    PROPERTY_READ_ONLY(bubbles, "bubbles", bool);
    PROPERTY_READ_ONLY(cancelable, "cancelable", bool);
    PROPERTY_READ_ONLY(composed, "composed", bool);
    PROPERTY_READ_ONLY(current_target, "currentTarget", js_object);
    PROPERTY_READ_ONLY(default_prevented, "defaultPrevented", bool);
    PROPERTY_READ_ONLY(event_phase, "eventPhase", int);
    PROPERTY_READ_ONLY(is_trusted, "isTrusted", bool);
    PROPERTY_READ_ONLY(target, "target", js_object);
    PROPERTY_READ_ONLY(time_stamp, "timeStamp", int);
    PROPERTY_READ_ONLY(type, "type", std::string);
};

}
