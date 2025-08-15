//written with ChatGPT 5
//tiny RAII (Resource Acquisition Is Initialization) SharedLib that only wraps dlopen/dlclose

#pragma once
#include <dlfcn.h>
#include <string>
#include <utility>
#include <iostream>

struct SharedLib {
    void* handle = nullptr;

    SharedLib() = default;
    explicit SharedLib(const std::string& path) {
        handle = dlopen(path.c_str(), RTLD_NOW);
        if (!handle) {
            std::cerr << "[dlopen] " << path << " : " << dlerror() << "\n";
        }
    }
    ~SharedLib() { if (handle) dlclose(handle); }

    SharedLib(const SharedLib&) = delete;
    SharedLib& operator=(const SharedLib&) = delete;

    SharedLib(SharedLib&& o) noexcept : handle(std::exchange(o.handle, nullptr)) {}
    SharedLib& operator=(SharedLib&& o) noexcept {
        if (this != &o) {
            if (handle) dlclose(handle);
            handle = std::exchange(o.handle, nullptr);
        }
        return *this;
    }

    explicit operator bool() const { return handle != nullptr; }
};
