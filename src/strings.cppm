export module webpp:strings;

import std;

namespace webpp {
    [[clang::export_name("webpp::new_string")]]
    char* new_string(std::size_t len) {
        return new char[len];
    }

    [[clang::export_name("webpp::delete_string")]]
    void delete_string(char* ptr) {
        delete[] ptr;
    }

    void init_strings() {

    }
}
