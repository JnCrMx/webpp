#pragma once

import std;

#define PROPERTY_READ_ONLY(name, key, type) \
    property_type_traits<type>::get_type name() const { \
        return *get_property<property_type_traits<type>::get_type>(key); \
    }
#define PROPERTY_WRITE_ONLY(name, key, type) \
    void name(property_type_traits<type>::set_type value) { \
        set_property(key, value); \
    }
#define PROPERTY_READ_WRITE(name, key, type) \
    PROPERTY_READ_ONLY(name, key, type) \
    PROPERTY_WRITE_ONLY(name, key, type)
