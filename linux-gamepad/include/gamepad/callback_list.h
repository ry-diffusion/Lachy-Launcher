#pragma once

#include <vector>

namespace gamepad {

using CallbackId = long int;

class CallbackAutoHandler {

private:
    struct Entry {
        void* list;
        CallbackId id;
        void (*deleter)(void* list, CallbackId id);
    };
    std::vector<Entry> callbacks;

public:
    ~CallbackAutoHandler();

    void addCallbackDeleter(void* list, CallbackId id, void (*deleter)(void* list, CallbackId id)) {
        callbacks.push_back({list, id, deleter});
    }

};

template <typename T>
class CallbackList {

private:
    struct Entry {
        CallbackId id;
        T value;
    };

    std::vector<Entry> callbacks;
    CallbackId nextId = 0;

public:
    CallbackId add(T t) {
        auto id = nextId++;
        callbacks.push_back({id, t});
        return id;
    }

    void add(CallbackAutoHandler& h, T t) {
        h.addCallbackDeleter(this, add(t), [](void* list, CallbackId id) {
            ((CallbackList<T>*) list)->remove(id);
        });
    }

    void remove(CallbackId id) {
        for (auto it = callbacks.begin(); it != callbacks.end(); it++) {
            if (it->id == id) {
                callbacks.erase(it);
                return;
            }
        }
    }

    template <typename ...Args>
    void operator ()(Args&&... args) {
        for (auto const& c : callbacks)
            c.value(args...);
    }

};

inline CallbackAutoHandler::~CallbackAutoHandler() {
    for (auto& p : callbacks)
        p.deleter(p.list, p.id);
}

}