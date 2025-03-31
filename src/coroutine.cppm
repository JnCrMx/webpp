export module webpp:coroutine;

import std;
import :event_loop;

import :basic;

namespace webpp::coro {

export template<typename Return>
struct promise;

export template<typename Return>
struct coroutine : std::coroutine_handle<promise<Return>> {
    using promise_type = promise<Return>;

    ~coroutine() {
        if(this->done() && this->promise().caller) {
            this->destroy();
        }
    }

    template<typename T>
    bool await_suspend(std::coroutine_handle<T> caller) const noexcept {
        this->promise().caller = caller;
        return true;
    }
    bool await_ready() const noexcept {
        return this->done();
    }

    auto await_resume() {
        if constexpr (std::is_same_v<Return, void>) {
            return;
        } else {
            return this->promise().result;
        }
    }
};

template<typename Return>
struct final_awaiter {
    bool await_ready() const noexcept { return false; }
    std::coroutine_handle<> await_suspend(std::coroutine_handle<promise<Return>> handle) const noexcept {
        auto caller = handle.promise().caller;
        if(caller) {
            return caller;
        } else {
            if(handle.promise().self_destruct) {
                handle.destroy();
            }
            return std::noop_coroutine();
        }
    }
    auto await_resume() const noexcept {}
};

template<typename Return>
struct promise_base {
    std::coroutine_handle<> caller;
    bool self_destruct = false;

    std::suspend_never initial_suspend() noexcept { return {}; }
    final_awaiter<Return> final_suspend() noexcept { return {}; }
    void unhandled_exception() noexcept {}
};

export template<typename Return>
struct promise : promise_base<Return>
{
    Return result{};

    void return_value(Return&& value) {
        result = std::move(value);
    }

    coroutine<Return> get_return_object() {
        return coroutine<Return>{std::coroutine_handle<promise<Return>>::from_promise(*this)};
    }
};

export template<>
struct promise<void> : promise_base<void>
{
    void return_void() {}
    coroutine<void> get_return_object() {
        return coroutine<void>{std::coroutine_handle<promise<void>>::from_promise(*this)};
    }
};

template<typename T>
using transformed_type_t = std::conditional_t<std::is_void_v<T>, std::monostate, T>;

template<typename Awaitable>
auto transform_type(Awaitable&& value) -> coroutine<transformed_type_t<decltype(value.await_resume())>> {
    if constexpr (std::is_void_v<decltype(value.await_resume())>) {
        co_await value;
        co_return std::monostate{};
    } else {
        co_return co_await value;
    }
}

export template<typename... Awaitables>
[[nodiscard("This is a coroutine, you must either co_await or submit it.")]]
auto when_all(Awaitables&&... coros) -> coroutine<std::tuple<transformed_type_t<decltype(coros.await_resume())>...>> {
    co_return std::tuple{co_await transform_type(coros)...};
}

export template<typename Return>
void submit(coroutine<Return>&& coro) {
    coro.promise().self_destruct = true;
}

export struct timeout {
    std::chrono::milliseconds duration;
    std::coroutine_handle<> handle;

    timeout(std::chrono::milliseconds duration) : duration(duration) {}

    bool await_ready() const noexcept { return false; }
    bool await_suspend(std::coroutine_handle<> handle) noexcept {
        this->handle = handle;
        webpp::set_timeout(duration, [handle]() {
            handle.resume();
        });
        return true;
    }
    void await_resume() const noexcept {}
};
export auto next_tick() {
    return timeout{std::chrono::milliseconds{0}};
}

template<typename T>
concept awaitable = requires(T t) {
    { t.await_ready() };
    { t.await_suspend(std::declval<std::coroutine_handle<>>()) };
    { t.await_resume() };
};


export template<typename First, typename SecondFunc>
struct chain_awaiter {
    using FirstReturn = decltype(std::declval<First>().await_resume());
    using Second = std::invoke_result_t<SecondFunc, FirstReturn&&>;
    using SecondReturn = decltype(std::declval<Second>().await_resume());

    coroutine<SecondReturn> callback;

    chain_awaiter(First&& first, SecondFunc&& second) {
        callback = [](First first, SecondFunc second) -> coroutine<SecondReturn> {
            co_return co_await second(co_await first);
        }(std::move(first), std::move(second));
    }
    chain_awaiter(const chain_awaiter&) = delete;
    chain_awaiter(chain_awaiter&& other) : callback(std::move(other.callback)) {}

    auto await_suspend(std::coroutine_handle<> handle) noexcept {
        return callback.await_suspend(handle);
    }

    bool await_ready() const noexcept {
        return callback.await_ready();
    }
    auto await_resume() noexcept {
        return callback.await_resume();
    }

    template<typename Func>
    auto then(this auto&& self, Func&& next) {
        using Ret = std::invoke_result_t<Func, SecondReturn&&>;
        if constexpr (awaitable<Ret>) {
            return chain_awaiter<std::decay_t<decltype(self)>, Func>(std::move(self), std::move(next));
        } else {
            static_assert(false, "Can only chain awaitables");
        }
    }
};

template<typename T>
struct generic_awaiter {
    std::coroutine_handle<> handle;
    callback_data* callback;
    T result;
    bool done = false;

    generic_awaiter() = default;
    generic_awaiter(const generic_awaiter&) = delete;
    generic_awaiter(generic_awaiter&& other) : handle(other.handle), callback(other.callback), result(std::move(other.result)), done(other.done) {
        other.done = true;
    }

    ~generic_awaiter() {
        if(!done) {
            callback->abandon();
        }
    }

    void complete(T&& result) {
        this->result = std::move(result);
        try_resume();
    }

    void try_resume() {
        if(!done && handle) {
            handle.resume();
        }
        done = true;
    }

    bool await_ready() const noexcept { return done; }
    bool await_suspend(std::coroutine_handle<> handle) noexcept {
        this->handle = handle;
        return true;
    }
    T await_resume() noexcept {
        return std::move(result);
    }

    template<typename Func>
    auto then(this auto&& self, Func&& next) {
        using Ret = std::invoke_result_t<Func, T&&>;
        if constexpr (awaitable<Ret>) {
            return chain_awaiter<std::decay_t<decltype(self)>, Func>(std::move(self), std::move(next));
        } else {
            static_assert(false, "Can only chain awaitables");
        }
    }
};

}

namespace webpp {
    export using coro::coroutine; // NOLINT: We are exporting this under a different name and it is actually used.
}
