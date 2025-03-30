export module webpp:dom;

import :js_object;

namespace webpp {

[[clang::import_module("webpp"), clang::import_name("get_element_by_id")]]
js_handle get_element_by_id(const char* id, std::size_t len);

[[clang::import_module("webpp"), clang::import_name("create_element")]]
js_handle create_element(const char* tag, std::size_t len);

[[clang::import_module("webpp"), clang::import_name("append_child")]]
void append_child(js_handle parent, js_handle child);

[[clang::import_module("webpp"), clang::import_name("remove_child")]]
void remove_child(js_handle parent, js_handle child);

export class element : public js_object{
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

    void inner_text(std::string_view text) {
        (*this)["innerText"] = text;
    }
    void inner_html(std::string_view html) {
        (*this)["innerHTML"] = html;
    }
    void append_child(const element& child) {
        ::webpp::append_child(handle(), child.handle());
    }
    void remove_child(const element& child) {
        ::webpp::remove_child(handle(), child.handle());
    }
};

export std::expected<element, error_code> get_element_by_id(std::string_view id) {
    return element(get_element_by_id(id.data(), id.size()));
}
export std::expected<element, error_code> create_element(std::string_view tag) {
    return element(create_element(tag.data(), tag.size()));
}

}
