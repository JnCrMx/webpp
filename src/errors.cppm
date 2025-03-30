export module webpp:errors;

namespace webpp {

export enum class error_code {
    success = 0,
    invalid_handle = 1,
    invalid_type = 2,
    invalid_property = 3,
    invalid_value = 4,
    invalid_name = 5,
    invalid_length = 6,
    element_not_found = 7,
};

}
