#pragma once

#include "connection.h"
#include "rapidjson/document.h"

struct RpcConnection {
    enum class Opcode : uint32_t {
        Handshake = 0,
        Frame = 1,
        Close = 2,
        Ping = 3,
        Pong = 4,
    };

    struct MessageFrameHeader {
        Opcode opcode;
        uint32_t length;
    };

    struct MessageFrame : public MessageFrameHeader {
        char message[64 * 1024 - sizeof(MessageFrameHeader)];
    };

    enum class State : uint32_t {
        Disconnected,
        Connecting,
        Connected,
    };

    BaseConnection* connection{nullptr};
    State state{State::Disconnected};
    void (*onConnect)(){nullptr};
    void (*onDisconnect)(int errorCode, const char* message){nullptr};
    char appId[64]{};
    int lastErrorCode{0};
    char lastErrorMessage[256]{};

    static RpcConnection* Create(const char* applicationId);
    static void Destroy(RpcConnection*&);

    inline bool IsOpen() const {
        return state == State::Connected;
    }

    void Open();
    void Close();
    void Write(const void* data, size_t length);
    bool Read(rapidjson::Document& message);
};