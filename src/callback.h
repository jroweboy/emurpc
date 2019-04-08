
#pragma once

class Callback {
public:
    bool IsBlocking() const {
        return is_blocking;
    }

private:
    bool is_blocking = false;
};

class FrameEndCallback final : public Callback {};

class MemoryCallback final : public Callback {};

class GPUCallback final : public Callback {};

class SpecialCallback final : public Callback {};
