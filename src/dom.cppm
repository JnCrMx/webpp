export module webpp:dom;

import :js_object;
import :event_loop;
import :coroutine;

#include "macros.hpp"

namespace webpp {

[[clang::import_module("webpp"), clang::import_name("get_element_by_id")]]
js_handle get_element_by_id(const char* id, std::size_t len);

[[clang::import_module("webpp"), clang::import_name("create_element")]]
js_handle create_element(const char* tag, std::size_t len);

[[clang::import_module("webpp"), clang::import_name("append_child")]]
void append_child(js_handle parent, js_handle child);

[[clang::import_module("webpp"), clang::import_name("remove_child")]]
void remove_child(js_handle parent, js_handle child);

[[clang::import_module("webpp"), clang::import_name("add_event_listener")]]
void add_event_listener(js_handle handle, const char* event, std::size_t event_size, callback_data* data, bool once);

std::set<std::string> parse_classes(std::string_view classes) {
    std::set<std::string> result;
    std::istringstream ss{std::string{classes}};
    std::string token;
    while(std::getline(ss, token, ' ')) {
        result.insert(token);
    }
    return result;
}

struct event_awaiter : coro::generic_awaiter<webpp::event> {
    auto create_callback() {
        return [this](js_handle handle, std::string_view) {
            complete(webpp::event{handle});
        };
    }

    event_awaiter(js_handle handle, std::string_view event) {
        callback = new callback_data{create_callback(), true};
        webpp::add_event_listener(handle, event.data(), event.size(), callback, true);
    }
    event_awaiter(event_awaiter&& other) : coro::generic_awaiter<webpp::event>{std::move(other)} {
        callback->replace(create_callback());
    }
};

struct event_target {
    callback_data* add_event_listener(this const auto& self, std::string_view event, std::function<void(webpp::event)>&& callback, bool once = false) {
        callback_data* data = new callback_data{[callback = std::move(callback)](js_handle handle, std::string_view){
            callback(webpp::event(handle));
        }, once};
        webpp::add_event_listener(self.handle(), event.data(), event.size(), data, once);
        return data;
    }

    event_awaiter co_event(this const auto& self, std::string_view event) {
        return event_awaiter{self.handle(), event};
    }
    event_awaiter event(this const auto& self, std::string_view event) { return self.co_event(event); } // for convenience
};

export class element : public js_object, public event_target {
public:
    static std::expected<element, error_code> create(js_handle handle) {
        if(!check_instanceof(handle, "HTMLElement")) {
            return std::unexpected(error_code::invalid_type);
        }
        return element(handle);
    }

    element(js_handle handle) : js_object(handle) {}
    element(const element&) = delete;
    element(element&& other) : js_object(std::move(other)) {}
    virtual ~element() {}

    element& operator=(const element&) = delete;
    element& operator=(element&& other) {
        js_object::operator=(std::move(other));
        return *this;
    }

    PROPERTY_READ_ONLY(client_width,  "clientWidth",  int);
    PROPERTY_READ_ONLY(client_height, "clientHeight", int);
    PROPERTY_READ_ONLY(client_top,    "clientTop",    int);
    PROPERTY_READ_ONLY(client_left,   "clientLeft",   int);
    PROPERTY_READ_ONLY(offset_width,  "offsetWidth",  int);
    PROPERTY_READ_ONLY(offset_height, "offsetHeight", int);
    PROPERTY_READ_ONLY(offset_top,    "offsetTop",    int);
    PROPERTY_READ_ONLY(offset_left,   "offsetLeft",   int);

    PROPERTY_READ_WRITE(inner_text, "innerText", std::string);
    PROPERTY_READ_WRITE(inner_html, "innerHTML", std::string);

    PROPERTY_READ_ONLY(tag_name, "tagName", std::string);
    PROPERTY_READ_WRITE(id, "id", std::string);
    PROPERTY_READ_WRITE(class_name, "className", std::string);
    PROPERTY_READ_ONLY(style, "style", js_object);

    void append_child(const element& child) {
        ::webpp::append_child(handle(), child.handle());
    }
    void remove_child(const element& child) {
        ::webpp::remove_child(handle(), child.handle());
    }

    std::set<std::string> classes() {
        return parse_classes(class_name());
    }
    void classes(const std::set<std::string>& classes) {
        class_name(std::accumulate(classes.begin(), classes.end(), std::string{}, [](const std::string& a, const std::string& b) {
            return a + " " + b;
        }));
    }

    void add_class(const std::string& name) {
        auto classes = this->classes();
        classes.insert(name);
        this->classes(classes);
    }
    void remove_class(const std::string& name) {
        auto classes = this->classes();
        classes.erase(name);
        this->classes(classes);
    }
    void toggle_class(const std::string& name) {
        auto classes = this->classes();
        if(classes.contains(name)) {
            classes.erase(name);
        } else {
            classes.insert(name);
        }
        this->classes(classes);
    }
};

export std::expected<element, error_code> get_element_by_id(std::string_view id) {
    return element(get_element_by_id(id.data(), id.size()));
}
export std::expected<element, error_code> create_element(std::string_view tag) {
    return element(create_element(tag.data(), tag.size()));
}

struct window_t : public event_target {
    constexpr static js_handle handle() {return 1;}
};
export inline const window_t window;

struct document_t : public event_target {
    constexpr static js_handle handle() {return 2;}
};
export inline const document_t document;

}
