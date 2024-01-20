#pragma once

#include <cll/event_batch.h>

namespace cll {

class BufferedEventList : public BatchedEventList {

public:
    enum class Type { Wrapped, Memory, Merged };

private:
    const Type type;

public:
    BufferedEventList(Type type) : type(type) {}

    inline Type getType() const { return type; }

};

class WrapperBufferedEventList : public BufferedEventList {

public:
    std::unique_ptr<BatchedEventList> wrapped;
    bool hasMemoryEvents;

    WrapperBufferedEventList(Type type, std::unique_ptr<BatchedEventList> wrapped, bool hasMemoryEvents = false) :
            BufferedEventList(type), wrapped(std::move(wrapped)), hasMemoryEvents(hasMemoryEvents) {}

    const char* getData() const override {
        return wrapped->getData();
    }

    size_t getDataSize() const override {
        return wrapped->getDataSize();
    }

    size_t getEventCount() const override {
        return wrapped->getEventCount();
    }

    bool hasMoreEvents() const override {
        return hasMemoryEvents || wrapped->hasMoreEvents();
    }

};

class MergedBufferedEventList : public BufferedEventList {

public:
    std::unique_ptr<BatchedEventList> wrapped, mem;
    std::vector<char> data;

    MergedBufferedEventList(std::unique_ptr<BatchedEventList> wrapped, std::unique_ptr<BatchedEventList> mem) :
            BufferedEventList(Type::Merged), wrapped(std::move(wrapped)), mem(std::move(mem)) {
        size_t ws = this->wrapped->getDataSize();
        size_t ms = this->mem->getDataSize();
        data.resize(ws + ms);
        memcpy(data.data(), this->wrapped->getData(), ws);
        memcpy(&data.data()[ws], this->mem->getData(), ms);
    }

    const char* getData() const override {
        return data.data();
    }

    size_t getDataSize() const override {
        return data.size();
    }

    size_t getEventCount() const override {
        return wrapped->getEventCount() + mem->getEventCount();
    }

    bool hasMoreEvents() const override {
        return wrapped->hasMoreEvents() || mem->hasMoreEvents();
    }

};

}
