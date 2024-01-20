#include <cll/event_uploader.h>
#include <cll/http/mock_http_client.h>
#include <gtest/gtest.h>

using namespace cll;
using namespace cll::http;

class CustomUploadStep : public EventUploadStep {

public:
    bool requestStarted = false;
    int timesRequestWasCalled = 0;
    int timesAuthFailureWasCalled = 0;

    void onRequest(EventUploadRequest& request) override {
        requestStarted = true;
        timesRequestWasCalled++;
        request.headers.emplace_back("X-TestHeader", "True");
    }

    bool onAuthenticationFailed() override {
        if (!requestStarted)
            throw std::runtime_error("onAuthenticationFailed: requestStarted not set");
        timesAuthFailureWasCalled++;
        return true;
    }

};

TEST(EventUploaderTest, SuccessUpload) {
    MockHttpClient client;
    client.addMockedResponse("http://localhost/submit", {200, {}, "{\"acc\":2}"});
    EventUploader uploader (client, "http://localhost/submit");
    const char* dataText = "This is a test event data\r\nAnd another event\r\n";
    std::vector<char> data (strlen(dataText));
    memcpy(data.data(), dataText, data.size());
    VectorBatchedEventList events (data, 2, false);
    ASSERT_TRUE(uploader.sendEvents(events));
}

TEST(EventUploaderTest, RateLimitedUpload) {
    MockHttpClient client;
    client.addMockedResponse("http://localhost/submit", {429, {{"Retry-After", "50"}}, ""});
    EventUploader uploader (client, "http://localhost/submit");
    VectorBatchedEventList events (std::vector<char>(), 2, false);
    auto res = uploader.sendEvents(events);
    ASSERT_FALSE(res);
    ASSERT_EQ(res.state, EventUploadStatus::State::ErrorRateLimit);
    ASSERT_EQ(res.retryAfter, std::chrono::seconds(50));
}

TEST(EventUploaderTest, CustomStep) {
    MockHttpClient client;
    client.addMockedResponse("http://localhost/submit", {401, {}, ""});
    EventUploader uploader (client, "http://localhost/submit");
    CustomUploadStep step;
    uploader.addStep(step);
    const char* dataText = "This is a test event data\r\nAnd another event\r\n";
    std::vector<char> data (strlen(dataText));
    memcpy(data.data(), dataText, data.size());
    VectorBatchedEventList events (data, 2, false);
    ASSERT_FALSE(uploader.sendEvents(events));
    ASSERT_EQ(step.timesRequestWasCalled, 2);
    ASSERT_EQ(step.timesAuthFailureWasCalled, 2);
    client.addMockedResponse("http://localhost/submit", {200, {}, "{\"acc\":2}"});
    ASSERT_TRUE(uploader.sendEvents(events, false));
    ASSERT_EQ(step.timesRequestWasCalled, 3);
    ASSERT_EQ(step.timesAuthFailureWasCalled, 2);
}