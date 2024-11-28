#pragma once

template <typename F>
class Defer {
   public:
    Defer(F&& f) : f{std::move(f)} {}

    ~Defer() { now(); }

    void defuse() { shouldRun = false; }

    void now() {
        if (shouldRun) {
            f();
            shouldRun = false;
        }
    }

   private:
    bool shouldRun{true};
    F f;
};