#pragma once

#include "object.h"

namespace aspirin {

namespace detail {
extern void register_internal_plugin(const std::string &name,
                                     const std::string &plugin_name);
extern void clear_internal_plugins();
} // namespace detail

class APR_EXPORT PluginManager {
public:
    static PluginManager *instance() {
        static PluginManager instance;
        return &instance;
    }

    ref<Object> create_object(const Properties &, const Class *);
    template <typename T> ref<T> create_object(const Properties &props) {
        return static_cast<T *>(create_object(props, APR_CLASS(T)).get());
    }

protected:
    PluginManager()  = default;
    ~PluginManager() = default;
};

} // namespace aspirin