import webpp;
import std;

extern "C" void __wasm_call_ctors();
[[clang::export_name("_initialize")]]
void _initialize() {
    __wasm_call_ctors();
}

[[clang::export_name("main")]]
int main() {
    webpp::init();

    webpp::log("Hello World");

    auto div = *webpp::create_element("div");
    div.inner_text("Hello World");
    webpp::get_element_by_id("main")->append_child(div);

    webpp::set_timeout(std::chrono::seconds(1), []() {
        webpp::log("Hello World");
    });

    webpp::fetch("https://jsonplaceholder.typicode.com/todos/1", [](webpp::response res) {
        if(!res.ok()) {
            return;
        }
        res.text([](std::string_view text) {
            webpp::log("With callbacks: {}", text);
        });
    });

    webpp::coro::submit([]() -> webpp::coroutine<void> {
        std::array requests = {
            webpp::coro::fetch("https://jsonplaceholder.typicode.com/todos/1"),
            webpp::coro::fetch("https://jsonplaceholder.typicode.com/todos/2")
        };
        std::array resonses = {co_await requests[0], co_await requests[1]};
        std::array texts = {resonses[0].text(), resonses[1].text()};

        webpp::log("With coroutine: {} and {}", co_await texts[0], co_await texts[1]);

        auto text3 = webpp::coro::fetch("https://jsonplaceholder.typicode.com/todos/3").then(std::mem_fn(&webpp::response::co_text))
            .then([](std::string text) -> webpp::coroutine<std::string> {
                co_await webpp::coro::timeout{std::chrono::seconds(5)};
                co_return text;
            });
        auto text4 = webpp::coro::fetch("https://jsonplaceholder.typicode.com/todos/4").then(std::mem_fn(&webpp::response::co_text))
            .then([](std::string text) -> webpp::coroutine<std::string> {
                co_await webpp::coro::timeout{std::chrono::seconds(5)};
                co_return text + "!";
            })
            .then([](std::string text) -> webpp::coroutine<std::string> { co_return text + "!"; })
            .then([](std::string text) -> webpp::coroutine<std::string> { co_return text + "!"; });
        auto sleep = []() -> webpp::coroutine<void> {
            co_await webpp::coro::timeout{std::chrono::seconds(5)};
        }();
        auto [a, b, c] = co_await webpp::coro::when_all(std::move(text3), std::move(text4), std::move(sleep));
        webpp::log("Using chained awaitables: {} and {}", a, b);

        webpp::js_object::dump_all();
    }());
}

extern "C" {
void* __cxa_allocate_exception(std::size_t) { __builtin_trap(); return nullptr; }
void __cxa_throw(void *, void *, void (*) (void *)) { __builtin_trap(); }
void __cxa_rethrow() { __builtin_trap(); }
}
