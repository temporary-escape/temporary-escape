#pragma once

#include "../Config.hpp"
#include "../Graphics/Texture2D.hpp"
#include "../Library.hpp"
#include <list>
#include <optional>

#include <memory>

struct stbrp_context;
struct stbrp_node;

namespace Scissio {
struct SCISSIO_API ImageNode {
    Vector2i pos;
    Vector2i size;
    Texture2D* texture = nullptr;
};

class SCISSIO_API ImageAtlas {
public:
    explicit ImageAtlas(const Config& config);
    virtual ~ImageAtlas();

    std::shared_ptr<ImageNode> reserve(const Vector2i& size);

    const Vector2i& getSize() const {
        return size;
    }

private:
    class Layer {
    public:
        explicit Layer(const Config& config);
        ~Layer();

        std::optional<Vector2i> reserve(const Vector2i& size);

        Texture2D& getTexture() {
            return texture;
        }

    private:
        Texture2D texture;
        std::unique_ptr<stbrp_context> ctx;
        std::unique_ptr<stbrp_node[]> nodes;
    };

    const Config& config;
    Vector2i size;
    std::list<std::shared_ptr<Layer>> layers;
    std::list<std::shared_ptr<ImageNode>> imageNodes;
};
} // namespace Scissio
