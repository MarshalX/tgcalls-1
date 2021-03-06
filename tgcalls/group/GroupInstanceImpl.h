#ifndef TGCALLS_GROUP_INSTANCE_IMPL_H
#define TGCALLS_GROUP_INSTANCE_IMPL_H

#include <functional>
#include <vector>
#include <string>
#include <memory>
#include <map>

#include "../Instance.h"

namespace webrtc {
class AudioDeviceModule;
class TaskQueueFactory;
}

namespace rtc {
template <class T>
class scoped_refptr;
}

namespace tgcalls {

class LogSinkImpl;
class GroupInstanceManager;

struct GroupConfig {
    FilePath logPath;
};

struct GroupLevelValue {
    float level = 0.;
    bool voice = false;
};

struct GroupLevelUpdate {
    uint32_t ssrc = 0;
    GroupLevelValue value;
};

struct GroupLevelsUpdate {
    std::vector<GroupLevelUpdate> updates;
};

struct GroupInstanceDescriptor {
    GroupConfig config;
    std::function<void(bool)> networkStateUpdated;
    std::function<void(GroupLevelsUpdate const &)> audioLevelsUpdated;
    std::string initialInputDeviceId;
    std::string initialOutputDeviceId;
    bool debugIgnoreMissingSsrcs = false;
    std::function<rtc::scoped_refptr<webrtc::AudioDeviceModule>(webrtc::TaskQueueFactory*)> createAudioDeviceModule;
    std::shared_ptr<VideoCaptureInterface> videoCapture;
    std::function<void(std::vector<uint32_t> const &)> incomingVideoSourcesUpdated;
    std::function<void(std::vector<uint32_t> const &)> participantDescriptionsRequired;
};

struct GroupJoinPayloadFingerprint {
    std::string hash;
    std::string setup;
    std::string fingerprint;
};

struct GroupJoinPayloadVideoSourceGroup {
    std::vector<uint32_t> ssrcs;
    std::string semantics;
};

struct GroupJoinPayloadVideoPayloadFeedbackType {
    std::string type;
    std::string subtype;
};

struct GroupJoinPayloadVideoPayloadType {
    uint32_t id = 0;
    std::string name;
    uint32_t clockrate = 0;
    uint32_t channels = 0;
    std::vector<GroupJoinPayloadVideoPayloadFeedbackType> feedbackTypes;
    std::vector<std::pair<std::string, std::string>> parameters;
};

struct GroupJoinPayload {
    std::string ufrag;
    std::string pwd;
    std::vector<GroupJoinPayloadFingerprint> fingerprints;

    std::vector<GroupJoinPayloadVideoPayloadType> videoPayloadTypes;
    std::vector<std::pair<uint32_t, std::string>> videoExtensionMap;
    uint32_t ssrc = 0;
    std::vector<GroupJoinPayloadVideoSourceGroup> videoSourceGroups;
};

struct GroupParticipantDescription {
    std::string endpointId;
    uint32_t audioSsrc = 0;
    std::vector<GroupJoinPayloadVideoPayloadType> videoPayloadTypes;
    std::vector<std::pair<uint32_t, std::string>> videoExtensionMap;
    std::vector<GroupJoinPayloadVideoSourceGroup> videoSourceGroups;
    bool isRemoved = false;
};

struct GroupJoinResponseCandidate {
    std::string port;
    std::string protocol;
    std::string network;
    std::string generation;
    std::string id;
    std::string component;
    std::string foundation;
    std::string priority;
    std::string ip;
    std::string type;

    std::string tcpType;
    std::string relAddr;
    std::string relPort;
};

struct GroupJoinResponsePayload {
    std::string ufrag;
    std::string pwd;
    std::vector<GroupJoinPayloadFingerprint> fingerprints;
    std::vector<GroupJoinResponseCandidate> candidates;
};

template <typename T>
class ThreadLocalObject;

class GroupInstanceInterface {
protected:
    GroupInstanceInterface() = default;

public:
    virtual ~GroupInstanceInterface() = default;

    virtual void stop() = 0;

    virtual void emitJoinPayload(std::function<void(GroupJoinPayload)> completion) = 0;
    virtual void setJoinResponsePayload(GroupJoinResponsePayload payload, std::vector<tgcalls::GroupParticipantDescription> &&participants) = 0;
    virtual void addParticipants(std::vector<GroupParticipantDescription> &&participants) = 0;
    virtual void removeSsrcs(std::vector<uint32_t> ssrcs) = 0;

    virtual void setIsMuted(bool isMuted) = 0;
    virtual void setVideoCapture(std::shared_ptr<VideoCaptureInterface> videoCapture, std::function<void(GroupJoinPayload)> completion) = 0;
    virtual void setAudioOutputDevice(std::string id) = 0;
    virtual void setAudioInputDevice(std::string id) = 0;

    virtual void addIncomingVideoOutput(uint32_t ssrc, std::weak_ptr<rtc::VideoSinkInterface<webrtc::VideoFrame>> sink) = 0;

    virtual void setVolume(uint32_t ssrc, double volume) = 0;
    virtual void setFullSizeVideoSsrc(uint32_t ssrc) = 0;

    struct AudioDevice {
      enum class Type {Input, Output};
      std::string name;
      std::string guid;
    };

};

class GroupInstanceImpl final : public GroupInstanceInterface {
public:
	explicit GroupInstanceImpl(GroupInstanceDescriptor &&descriptor);
	~GroupInstanceImpl();

    void stop();

    void emitJoinPayload(std::function<void(GroupJoinPayload)> completion);
    void setJoinResponsePayload(GroupJoinResponsePayload payload, std::vector<tgcalls::GroupParticipantDescription> &&participants);
    void addParticipants(std::vector<GroupParticipantDescription> &&participants);
    void removeSsrcs(std::vector<uint32_t> ssrcs);

    void setIsMuted(bool isMuted);
    void setVideoCapture(std::shared_ptr<VideoCaptureInterface> videoCapture, std::function<void(GroupJoinPayload)> completion);
    void setAudioOutputDevice(std::string id);
    void setAudioInputDevice(std::string id);

    void addIncomingVideoOutput(uint32_t ssrc, std::weak_ptr<rtc::VideoSinkInterface<webrtc::VideoFrame>> sink);

    void setVolume(uint32_t ssrc, double volume);
    void setFullSizeVideoSsrc(uint32_t ssrc);

    static std::vector<GroupInstanceInterface::AudioDevice> getAudioDevices(GroupInstanceInterface::AudioDevice::Type type);
private:
	std::unique_ptr<ThreadLocalObject<GroupInstanceManager>> _manager;
	std::unique_ptr<LogSinkImpl> _logSink;

};

} // namespace tgcalls

#endif
