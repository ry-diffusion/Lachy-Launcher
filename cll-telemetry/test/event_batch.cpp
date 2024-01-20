#include "event_batch.h"

#include <gtest/gtest.h>

using namespace cll;

void EventBatchTest::BasicTest(EventBatch& batch) {
    // Add event
    nlohmann::json event = {{"test", "This is a test log entry"}};
    ASSERT_FALSE(batch.hasEvents());
    auto eventStr = event.dump();
    ASSERT_TRUE(batch.addEvent(event));
    ASSERT_TRUE(batch.hasEvents());
    // Get the event back
    auto upEv = batch.getEventsForUpload(1, 128); // this should return event + "\r\n"
    ASSERT_NE(upEv, nullptr);
    ASSERT_EQ(upEv->getDataSize(), eventStr.size() + 2);
    ASSERT_TRUE(memcmp(eventStr.data(), upEv->getData(), eventStr.size()) == 0);
    ASSERT_TRUE(memcmp(&upEv->getData()[eventStr.size()], "\r\n", 2) == 0);
    // Check simple (full) deletion behaviour
    batch.onEventsUploaded(*upEv);
    ASSERT_FALSE(batch.hasEvents());
    upEv = batch.getEventsForUpload(1, 128);
    ASSERT_TRUE(upEv == nullptr || upEv->getDataSize() == 0);
}

void EventBatchTest::SetUpTestEvents(cll::EventBatch& batch, size_t eventCount) {
    for (int i = 0; i < eventCount; i++) {
        nlohmann::json event = GetJsonFor(i);
        ASSERT_TRUE(batch.addEvent(event)) << "Index: " << i;
    }
}

void EventBatchTest::ReadIncremental(cll::EventBatch& batch, size_t eventCount) {
    for (size_t i = 1; i < eventCount; i++) {
        auto val = batch.getEventsForUpload(i, i * 128);
        ASSERT_GT(val->getDataSize(), 0) << "Iteration: " << i;
        auto evs = GetMessagesInEventList(val.get());
        ASSERT_EQ(evs.size(), i);
        for (size_t j = 0; j < i; j++) {
            nlohmann::json expected = GetJsonFor(j);
            ASSERT_EQ(expected.dump(), evs[j]);
        }
    }
}

void EventBatchTest::ReadIncrementalWithRemoval(cll::EventBatch& batch, size_t eventCount) {
    size_t maxCount = 1;
    size_t gotEvents = 0;
    while (gotEvents < eventCount) {
        auto val = batch.getEventsForUpload(maxCount, eventCount * maxCount);
        ASSERT_GT(val->getDataSize(), 0) << "Got count: " << gotEvents << "; Max count: " << maxCount;
        for (std::string msg : GetMessagesInEventList(val.get())) {
            nlohmann::json expected = GetJsonFor((int) gotEvents);
            ASSERT_EQ(expected.dump(), msg);
            gotEvents++;
            ASSERT_LE(gotEvents, eventCount);
        }
        batch.onEventsUploaded(*val);
        maxCount++;
    }
}


std::vector<std::string> EventBatchTest::GetMessagesInEventList(cll::BatchedEventList* val) {
    char const* ptr = val->getData();
    std::vector<std::string> list;
    if (val == nullptr)
        return list;
    while (true) {
        char const* e = (char const*) memchr(ptr, '\n', val->getDataSize() - (ptr - val->getData()));
        if (e == nullptr) {
            if (ptr != val->getData() + val->getDataSize())
                throw std::runtime_error("getMessagesInEventList: Has extra data after message");
            break;
        }
        if (*e != '\n' || *(e - 1) != '\r')
            throw std::runtime_error("getMessagesInEventList: Doesn't end with \\r\\n");
        list.push_back(std::string(ptr, e - ptr - 1));
        ptr = e + 1;
    }
    return list;
}