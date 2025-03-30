export module webpp:event_loop;

import std;
import :js_object;

namespace webpp {
    [[clang::import_module("webpp"), clang::import_name("set_timeout")]]
    void set_timeout(unsigned long millis, void* callback_data);

    export using event_callback = std::function<void(js_handle)>;
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
            friend void callback(void* ptr, js_handle data);

            event_callback callback;
            bool once = false;
    };

    [[clang::export_name("webpp::callback")]]
    void callback(void* ptr, js_handle data) {
        callback_data* cb = static_cast<callback_data*>(ptr);
        if(cb->callback) {
            cb->callback(data);
        }
        if(cb->once) {
            delete cb;
        }
    }

    export callback_data* set_timeout(std::chrono::milliseconds duration, std::function<void()> callback) {
        callback_data* data = new callback_data{[f = std::move(callback)](js_handle data) {
            f();
        }, true};
        set_timeout(duration.count(), data);
        return data;
    }
}
