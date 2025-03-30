export module webpp;

export import :errors;
export import :basic;
export import :strings;
export import :js_object;
export import :event_loop;
export import :dom;
export import :network;
export import :coroutine;

namespace webpp {
export void init() {
    init_strings();
}
}
