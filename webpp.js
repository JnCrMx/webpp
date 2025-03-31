function get_string(instance, ptr, len) {
    const cArray = new Uint8Array(instance.exports.memory.buffer, ptr, len);
    return new TextDecoder().decode(cArray);
}
function stringifyEvent(e) {
    const obj = {};
    for (let k in e) {
        obj[k] = e[k];
    }
    return JSON.stringify(obj, (k, v) => {
        if (v instanceof Node) return 'Node';
        if (v instanceof Window) return 'Window';
        return v;
    }, ' ');
}
function copy_string(instance, s) {
    const buffer = new TextEncoder().encode(s);
    const str_ptr = instance.exports["webpp::new_string"](buffer.length);
    const data = new Uint8Array(instance.exports.memory.buffer, str_ptr, buffer.length);
    data.set(buffer);
    return [str_ptr, buffer.length];
}
function copy_data(instance, buffer) {
    const str_ptr = instance.exports["webpp::new_string"](buffer.length);
    const data = new Uint8Array(instance.exports.memory.buffer, str_ptr, buffer.length);
    data.set(buffer);
    return [str_ptr, buffer.length];
}
function copy_string_null(instance, s) {
    const buffer = new TextEncoder().encode(s);
    const str_ptr = instance.exports["webpp::new_string"](buffer.length+1);
    const data = new Uint8Array(instance.exports.memory.buffer, str_ptr, buffer.length+1);
    data.set(buffer);
    data[buffer.length] = 0;
    return str_ptr;
}
function delete_string(instance, ptr) {
    instance.exports["webpp::delete_string"](ptr);
}

export const instantiateStreaming = async (source) => {
    let instance = null;
    let module = null;
    let js_objects = [null, window, document];

    const create_object_ref = (obj) => {
        js_objects.push(obj);
        return js_objects.length - 1;
    };
    const release_object_ref = (index) => {
        if (js_objects[index] === null) {
            console.log(`release_object_ref: ${index} already released`);
            return;
        }
        js_objects[index] = null;
    }

    const set_property_generic = (obj_index, name_ptr, name_len, value) => {
        const obj = js_objects[obj_index];
        const prop = get_string(instance, name_ptr, name_len);
        obj[prop] = value;
    }
    const get_property_generic = (obj_index, name_ptr, name_len) => {
        const obj = js_objects[obj_index];
        const prop = get_string(instance, name_ptr, name_len);
        return obj[prop];
    }

    const invoke_callback = (callback, handle, ptr, len) => {
        instance.exports["webpp::callback"](callback, handle, ptr, len);
    }


    const import_object = {
        webpp: {
            release_js_object: (index) => {
                release_object_ref(index);
            },
            dump_js_objects: () => {
                console.log(js_objects);
            },
            check_instanceof: (index, type_ptr, type_len) => {
                const obj = js_objects[index];
                const type = get_string(instance, type_ptr, type_len);
                return obj instanceof window[type];
            },
            eval: (ptr, len) => {
                const res = eval(get_string(instance, ptr, len));
                const str_ptr = copy_string_null(instance, res.toString());
                return str_ptr;
            },
            get_element_by_id: (ptr, len) => {
                const id = get_string(instance, ptr, len);
                const elem = document.getElementById(id);
                if(!elem) {
                    return 0;
                }
                return create_object_ref(elem);
            },
            create_element: (ptr, len) => {
                const tag = get_string(instance, ptr, len);
                const elem = document.createElement(tag);
                return create_object_ref(elem);
            },
            remove_element: (elem_index) => {
                const elem = js_objects[elem_index];
                elem.remove();
            },
            append_child: (parent_index, child_index) => {
                const parent = js_objects[parent_index];
                const child = js_objects[child_index];
                parent.appendChild(child);
            },
            remove_child: (parent_index, child_index) => {
                const parent = js_objects[parent_index];
                const child = js_objects[child_index];
                parent.removeChild(child);
            },
            add_event_listener: (elem_index, event_ptr, event_len, callback, once) => {
                const elem = js_objects[elem_index];
                const event = get_string(instance, event_ptr, event_len);
                elem.addEventListener(event, (e) => {
                    invoke_callback(callback, create_object_ref(e), 0, 0);
                }, { once: once });
            },
            log: (ptr, len) => {
                console.log(get_string(instance, ptr, len));
            },
            set_property_string: (obj_index, name_ptr, name_len, value_ptr, value_len) => { set_property_generic(obj_index, name_ptr, name_len, get_string(instance, value_ptr, value_len)); },
            set_property_int: set_property_generic,
            set_property_float: set_property_generic,
            set_property_bool: set_property_generic,
            set_property_null: (obj_index, name_ptr, name_len) => { set_property_generic(obj_index, name_ptr, name_len, null); },
            set_property_undefined: (obj_index, name_ptr, name_len) => { set_property_generic(obj_index, name_ptr, name_len, undefined); },
            set_property_object: (obj_index, name_ptr, name_len, value_index) => { set_property_generic(obj_index, name_ptr, name_len, js_objects[value_index]); },
            get_property_string: (obj_index, name_ptr, name_len) => { return copy_string_null(instance, get_property_generic(obj_index, name_ptr, name_len)); },
            get_property_object: (obj_index, name_ptr, name_len) => { return create_object_ref(get_property_generic(obj_index, name_ptr, name_len)); },
            get_property_int: get_property_generic,
            get_property_float: get_property_generic,
            get_property_bool: get_property_generic,
            set_timeout: (timeout, callback) => {
                setTimeout(() => {
                    invoke_callback(callback, 0, 0, 0);
                }, timeout);
            },
            fetch: (url_ptr, url_len, callback) => {
                fetch(get_string(instance, url_ptr, url_len))
                    .then(response => invoke_callback(callback, create_object_ref(response), 0, 0));
            },
            response_text: (response_index, callback) => {
                const response = js_objects[response_index];
                response.text().then(text => {
                    const [ptr, len] = copy_string(instance, text);
                    invoke_callback(callback, 0, ptr, len);
                    delete_string(instance, ptr);
                });
            },
            response_bytes: (response_index, callback) => {
                const response = js_objects[response_index];
                response.bytes().then(bytes => {
                    const [ptr, len] = copy_data(instance, bytes);
                    invoke_callback(callback, 0, ptr, len);
                    delete_string(instance, ptr);
                });
            },
            event_prevent_default: (event_index) => {
                const event = js_objects[event_index];
                event.preventDefault();
            },
            event_stop_propagation: (event_index) => {
                const event = js_objects[event_index];
                event.stopPropagation();
            },
            event_stop_immediate_propagation: (event_index) => {
                const event = js_objects[event_index];
                event.stopImmediatePropagation();
            },
        },
        wasi_snapshot_preview1: {
            fd_close: () => { console.log("fd_close"); },
            fd_seek: () => { console.log("fd_seek"); },
            fd_write: () => { console.log("fd_write"); },
            fd_fdstat_get: () => { console.log("fd_fdstat_get"); },
            fd_prestat_get: () => { console.log("fd_prestat_get"); },
            fd_prestat_dir_name: () => { console.log("fd_prestat_dir_name"); },
            environ_get: () => { console.log("environ_get"); },
            environ_sizes_get: () => { console.log("environ_sizes_get"); },
            proc_exit: () => { console.log("proc_exit"); },
            random_get(buf, buf_len) {
                const data = new Uint8Array(instance.exports.memory.buffer, buf, buf_len);
                for (let i = 0; i < buf_len; ++i) {
                    data[i] = (Math.random() * 256) | 0;
                }
            }
        }
    };

    ({ instance, module } = await WebAssembly.instantiateStreaming(
        source, import_object
    ));
    return { instance, module };
}
