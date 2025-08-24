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
            throw std::runtime_error(std::string("[dlopen] ") + path + std::string(" : ") + dlerror());
        }
    }
    ~SharedLib() { if (handle) dlclose(handle); }

    //Copy operations are deleted
    SharedLib(const SharedLib&) = delete;
    SharedLib& operator=(const SharedLib&) = delete;

    //Move constructor
    SharedLib(SharedLib&& o) noexcept : handle(std::exchange(o.handle, nullptr)) {}

    //Move assignment operator
    SharedLib& operator=(SharedLib&& o) noexcept {
        if (this != &o) {
            if (handle) dlclose(handle);
            handle = std::exchange(o.handle, nullptr);
        }
        return *this;
    }

    //Boolean conversion operator
    //Allows: if (lib) { /* lib loaded successfully */ }
    explicit operator bool() const { return handle != nullptr; }
};
