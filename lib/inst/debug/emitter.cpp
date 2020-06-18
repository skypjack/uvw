struct UVW_EXTERN FakeEvent {
};

struct UVW_EXTERN TestEmitter : uvw::Emitter<TestEmitter> {
    void emit() {
        publish(FakeEvent{});
    }
};
