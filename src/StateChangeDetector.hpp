#pragma once

template <typename T = bool>
class StateChangeDetector {
public:
    StateChangeDetector() {}
    StateChangeDetector(T&& state): state(state) {}

    T state = false;

    bool hasChanged(T nowState) {
        bool trigger = state != nowState;
        state = nowState;
        return trigger;
    }

    bool hasChangedOn(bool nowState) {
        bool trigger = !state && nowState;
        state = nowState;
        return trigger;
    }

    T diff(T nowState) {
        T diff = nowState - state;
        state = nowState;
        return diff;
    }
};
