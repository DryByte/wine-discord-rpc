#pragma once

#include <windows.h>

// This is to wrap the platform specific kinds of connect/read/write.

#include <stdint.h>
#include <stdlib.h>

// not really connectiony, but need per-platform
int GetProcessId();

class BaseConnection {
public:
    bool isOpen{false};
    virtual void Destroy(BaseConnection*&) = 0;
    virtual bool Open() = 0;
    virtual bool Close() = 0;
    virtual bool Write(const void* data, size_t length) = 0;
    virtual bool Read(void* data, size_t length) = 0;
};

class BaseConnectionWin : public BaseConnection {
    HANDLE pipe{INVALID_HANDLE_VALUE};

public:
    void Destroy(BaseConnection*&) override;
    bool Open() override;
    bool Close() override;
    bool Write(const void* data, size_t length) override;
    bool Read(void* data, size_t length) override;
};

class BaseConnectionUnix : public BaseConnection {
    int sock{-1};

public:
    void Destroy(BaseConnection*&) override;
    bool Open() override;
    bool Close() override;
    bool Write(const void* data, size_t length) override;
    bool Read(void* data, size_t length) override;
};