#include "../Config.hpp"
#include "../Math/Vector.hpp"
#include "../Vulkan/Window.hpp"
#include "Inputs.hpp"

namespace Engine {
class ENGINE_API UserInput {
public:
    struct Event {
        bool started{false};
        Input type{Input::None};
        Vector2 value;
    };

    class Handler {
    public:
        virtual ~Handler() = default;

        virtual void eventUserInput(const Event& event) = 0;
    };

    explicit UserInput(const Config& config, Handler& handler);

    void eventMouseMoved(const Vector2i& pos);
    void eventMousePressed(const Vector2i& pos, MouseButton button);
    void eventMouseReleased(const Vector2i& pos, MouseButton button);
    void eventMouseScroll(int xscroll, int yscroll);
    void eventKeyPressed(Key key, Modifiers modifiers);
    void eventKeyReleased(Key key, Modifiers modifiers);
    void eventWindowResized(const Vector2i& size);

private:
    enum class Source {
        Key,
        Mouse,
        Button,
    };

    struct Binding {
        Input type{Input::None};
        Source source{Source::Key};
        Key key{Key::None};
        Modifiers modifiers{0};
    };

    const Config& config;
    Handler& handler;
    std::vector<Binding> bindings;
};
} // namespace Engine
