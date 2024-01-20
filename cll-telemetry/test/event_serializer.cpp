#include <cll/event_serializer.h>
#include <cll/event_serializer_extensions.h>
#include <cll/event.h>
#include <gtest/gtest.h>

using namespace cll;

TEST(EventSerializerTest, BasicTest) {
    EventSerializer serializer;
    serializer.setIKey("test-ikey");
    serializer.setApp("my-app", "1.0.0");
    serializer.setOsInfo("Linux", "1.2.3.4");
    serializer.setEpoch(12345678L);

    Event testEvent ("Event Name", {{"some", "data"}}, EventFlags::LatencyRealtime | EventFlags::PersistenceCritical,
                     {}, Event::Time(std::chrono::seconds(1528389000)));

    nlohmann::json json = serializer.createEnvelopeFor(testEvent);

    ASSERT_EQ(json.dump(), "{\"appId\":\"my-app\",\"appVer\":\"1.0.0\",\"data\":{\"some\":\"data\"},\"epoch\":\"12345678\",\"flags\":514,\"iKey\":\"test-ikey\",\"name\":\"Event Name\",\"os\":\"Linux\",\"osVer\":\"1.2.3.4\",\"popSample\":100.0,\"seqNum\":0,\"time\":\"2018-06-07T16:30:00.000Z\",\"ver\":\"2.1\"}");
}

TEST(EventSerializerTest, UserInfoExtension) {
    EventSerializer serializer;

    UserInfoExtension userExt;
    userExt.setLocalId("1234");
    serializer.addExtension(userExt);

    Event testEvent ("Event Name", {{"some", "data"}}, EventFlags::LatencyRealtime | EventFlags::PersistenceCritical,
                     {}, Event::Time(std::chrono::seconds(1528389000)));

    nlohmann::json json = serializer.createEnvelopeFor(testEvent)["ext"]["user"];
    ASSERT_EQ(json.dump(),"{\"localId\":\"1234\",\"ver\":\"1.0\"}");
}

TEST(EventSerializerTest, AndroidExtension) {
    EventSerializer serializer;

    AndroidExtension androidExt;
    serializer.addExtension(androidExt);

    Event testEvent ("Event Name", {{"some", "data"}}, EventFlags::LatencyRealtime | EventFlags::PersistenceCritical,
                     {"1234"}, Event::Time(std::chrono::seconds(1528389000)));

    nlohmann::json json = serializer.createEnvelopeFor(testEvent)["ext"]["android"];
    ASSERT_EQ(json.dump(), "{\"libVer\":\"3.170921.0\",\"tickets\":[\"1234\"],\"ver\":\"1.0\"}");
}


TEST(EventSerializerTest, DefaultExtensions) {
    EventSerializer serializer;

    DefaultExtensions extensions;
    extensions.addTo(serializer);

    Event testEvent ("Event Name", {{"some", "data"}}, EventFlags::LatencyRealtime | EventFlags::PersistenceCritical,
                     {"1234"}, Event::Time(std::chrono::seconds(1528389000)));

    nlohmann::json json = serializer.createEnvelopeFor(testEvent)["ext"];
    ASSERT_TRUE(json.count("user") > 0);
    ASSERT_TRUE(json.count("os") > 0);
    ASSERT_TRUE(json.count("device") > 0);
    ASSERT_TRUE(json.count("android") > 0);
}
