export module webpp:js_object;

import std;
import :errors;
import :strings;

namespace webpp {

using js_handle = std::int32_t;

template<typename T>
struct property_type_traits {
    using type = T;
    using get_type = T;
    using set_type = const T&;
};

template<>
struct property_type_traits<std::string> {
    using type = std::string;
    using get_type = std::string;
    using set_type = std::string_view;
};

[[clang::import_module("webpp"), clang::import_name("release_js_object")]]
void release_js_object(js_handle handle);

[[clang::import_module("webpp"), clang::import_name("dump_js_objects")]]
void dump_js_objects();

[[clang::import_module("webpp"), clang::import_name("check_instanceof")]]
bool check_instanceof(js_handle handle, const char* type, std::size_t len);

[[clang::import_module("webpp"), clang::import_name("create_object")]]
js_handle create_object();

[[clang::import_module("webpp"), clang::import_name("create_uint8array")]]
js_handle create_uint8array(const std::uint8_t* data, std::size_t size);

[[clang::import_module("webpp"), clang::import_name("set_property_string")]]
error_code set_property_string(js_handle handle, const char* name, std::size_t name_len, const char* value, std::size_t value_len);

[[clang::import_module("webpp"), clang::import_name("set_property_int")]]
error_code set_property_int(js_handle handle, const char* name, std::size_t name_len, std::int32_t value);

[[clang::import_module("webpp"), clang::import_name("set_property_float")]]
error_code set_property_float(js_handle handle, const char* name, std::size_t name_len, float value);

[[clang::import_module("webpp"), clang::import_name("set_property_bool")]]
error_code set_property_bool(js_handle handle, const char* name, std::size_t name_len, bool value);

[[clang::import_module("webpp"), clang::import_name("set_property_null")]]
error_code set_property_null(js_handle handle, const char* name, std::size_t name_len);

[[clang::import_module("webpp"), clang::import_name("set_property_undefined")]]
error_code set_property_undefined(js_handle handle, const char* name, std::size_t name_len);

[[clang::import_module("webpp"), clang::import_name("set_property_object")]]
error_code set_property_object(js_handle handle, const char* name, std::size_t name_len, js_handle value);

[[clang::import_module("webpp"), clang::import_name("get_property_string")]]
char* get_property_string(js_handle handle, const char* name, std::size_t name_len);

[[clang::import_module("webpp"), clang::import_name("get_property_bool")]]
bool get_property_bool(js_handle handle, const char* name, std::size_t name_len);

[[clang::import_module("webpp"), clang::import_name("get_property_int")]]
std::int32_t get_property_int(js_handle handle, const char* name, std::size_t name_len);

[[clang::import_module("webpp"), clang::import_name("get_property_float")]]
float get_property_float(js_handle handle, const char* name, std::size_t name_len);

[[clang::import_module("webpp"), clang::import_name("get_property_object")]]
js_handle get_property_object(js_handle handle, const char* name, std::size_t name_len);

template<typename T>
concept create_from_handle = requires(js_handle handle) {
    { T::create(handle) } -> std::same_as<std::expected<T, error_code>>;
};

bool check_instanceof(js_handle handle, std::string_view type) {
    return check_instanceof(handle, type.data(), type.size());
}

export class js_property_proxy;
export class js_object {
public:
    static void dump_all() {
        dump_js_objects();
    }

    static std::expected<js_object, error_code> create(js_handle handle) {
        return js_object(handle);
    }
    static js_object create() {
        return js_object(webpp::create_object());
    }

    js_object() = default;
    js_object(js_handle handle) : m_handle(handle) {
    }

    js_object(const js_object&) = delete;
    js_object(js_object&& other) : m_handle(std::exchange(other.m_handle, 0)) {}

    virtual ~js_object() {
        if(m_handle) {
            release_js_object(m_handle);
        }
    }

    js_object& operator=(const js_object&) = delete;
    js_object& operator=(js_object&& other) {
        if(this != &other) {
            if(m_handle) {
                release_js_object(m_handle);
            }
            m_handle = std::exchange(other.m_handle, 0);
        }
        return *this;
    }

    js_handle handle() const {
        return m_handle;
    }

    js_property_proxy operator[](const std::string& name);
    std::expected<js_property_proxy, error_code> property(const std::string& name);

    error_code set_property(std::string_view property, std::string_view value) {
        return set_property_string(m_handle, property.data(), property.size(), value.data(), value.size());
    }
    error_code set_property(std::string_view property, const std::string& value) {
        return set_property_string(m_handle, property.data(), property.size(), value.data(), value.size());
    }
    error_code set_property(std::string_view property, const char* value) {
        return set_property_string(m_handle, property.data(), property.size(), value, std::strlen(value));
    }
    error_code set_property(std::string_view property, std::int32_t value) {
        return set_property_int(m_handle, property.data(), property.size(), value);
    }
    error_code set_property(std::string_view property, float value) {
        return set_property_float(m_handle, property.data(), property.size(), value);
    }
    error_code set_property(std::string_view property, bool value) {
        return set_property_bool(m_handle, property.data(), property.size(), value);
    }
    error_code set_property(std::string_view property, std::nullptr_t) {
        return set_property_null(m_handle, property.data(), property.size());
    }
    error_code set_property(std::string_view property, std::nullopt_t) {
        return set_property_undefined(m_handle, property.data(), property.size());
    }
    error_code set_property(std::string_view property, const js_object& value) {
        return set_property_object(m_handle, property.data(), property.size(), value.handle());
    }

    template<typename T>
    std::expected<T, error_code> get_property(std::string_view property) const {
        if constexpr (std::is_same_v<T, bool>) {
            return get_property_bool(m_handle, property.data(), property.size());
        } else if constexpr (std::is_same_v<T, std::int32_t>) {
            return get_property_int(m_handle, property.data(), property.size());
        } else if constexpr (std::is_same_v<T, float>) {
            return get_property_float(m_handle, property.data(), property.size());
        } else if constexpr (create_from_handle<T>) {
            return T::create(get_property_object(m_handle, property.data(), property.size()));
        } else if constexpr (std::is_same_v<T, std::string>) {
            char* cstr = get_property_string(m_handle, property.data(), property.size());
            std::string str{cstr};
            delete_string(cstr);
            return str;
        } else {
            static_assert(false, "Unsupported type");
        }
    }

    template<typename T>
    std::expected<T, error_code> as() {
        if constexpr (create_from_handle<T>) {
            auto res = T::create(m_handle);
            if(res) {
                m_handle = 0;
            }
            return res;
        } else {
            static_assert(false, "Unsupported type");
        }
    }
protected:
    js_handle release() {
        return std::exchange(m_handle, 0);
    }
private:
    js_handle m_handle = 0;
};

export class js_property_proxy {
    public:
        js_property_proxy(js_handle handle, const std::string& name, bool owning = false) : m_handle(handle), m_name(name), m_owning(owning) {}
        ~js_property_proxy() {
            if(m_owning) {
                release_js_object(m_handle);
            }
        }

        js_property_proxy& operator=(std::string_view value) {
            set_property_string(m_handle, m_name.data(), m_name.size(), value.data(), value.size());
            return *this;
        }
        js_property_proxy& operator=(const std::string& value) {
            return *this = std::string_view(value);
        }
        js_property_proxy& operator=(const char* value) {
            return *this = std::string_view(value);
        }

        js_property_proxy operator=(std::int32_t value) {
            set_property_int(m_handle, m_name.data(), m_name.size(), value);
            return *this;
        }
        js_property_proxy operator=(float value) {
            set_property_float(m_handle, m_name.data(), m_name.size(), value);
            return *this;
        }
        js_property_proxy operator=(bool value) {
            set_property_bool(m_handle, m_name.data(), m_name.size(), value);
            return *this;
        }
        js_property_proxy operator=(std::nullptr_t) {
            set_property_null(m_handle, m_name.data(), m_name.size());
            return *this;
        }
        js_property_proxy operator=(std::nullopt_t) {
            set_property_undefined(m_handle, m_name.data(), m_name.size());
            return *this;
        }
        js_property_proxy operator=(const js_object& value) {
            set_property_object(m_handle, m_name.data(), m_name.size(), value.handle());
            return *this;
        }

        js_property_proxy operator[](const std::string& name) {
            return js_property_proxy(get_property_object(m_handle, m_name.data(), m_name.size()), name, true);
        }

        template<typename T>
        std::expected<T, error_code> as() {
            if constexpr (create_from_handle<T>) {
                return T::create(get_property_object(m_handle, m_name.data(), m_name.size()));
            } else if constexpr (std::is_same_v<T, bool>) {
                return get_property_bool(m_handle, m_name.data(), m_name.size());
            } else if constexpr (std::is_same_v<T, std::int32_t>) {
                return get_property_int(m_handle, m_name.data(), m_name.size());
            } else if constexpr (std::is_same_v<T, float>) {
                return get_property_float(m_handle, m_name.data(), m_name.size());
            } else if constexpr (std::is_same_v<T, std::string>) {
                char* cstr = get_property_string(m_handle, m_name.data(), m_name.size());
                std::string str{cstr};
                delete_string(cstr);
                return str;
            } else {
                static_assert(false, "Unsupported type");
            }
        }
    private:
        js_handle m_handle;
        std::string m_name;
        bool m_owning = false;
};

js_property_proxy js_object::operator[](const std::string& name) {
    return js_property_proxy(m_handle, name);
}
std::expected<js_property_proxy, error_code> js_object::property(const std::string& name) {
    return js_property_proxy(m_handle, name);
}

export class uint8array : public js_object {
    public:
        static std::expected<uint8array, error_code> create(js_handle handle) {
            if(!check_instanceof(handle, "Uint8Array")) {
                return std::unexpected(error_code::invalid_type);
            }
            return uint8array(handle);
        }
        static uint8array create(const std::uint8_t* data, std::size_t size) {
            return uint8array(webpp::create_uint8array(data, size));
        }
        static uint8array create(std::span<const std::uint8_t> data) {
            return create(data.data(), data.size());
        }
        static uint8array create(std::string_view data) {
            return create(reinterpret_cast<const std::uint8_t*>(data.data()), data.size());
        }

        uint8array(js_handle handle) : js_object(handle) {}
        uint8array(const uint8array&) = delete;
        uint8array(uint8array&& other) : js_object(std::move(other)) {}
        virtual ~uint8array() {}
};

}
