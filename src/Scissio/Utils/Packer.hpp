#pragma once

#include "../Library.hpp"
#include "../Math/Vector.hpp"
#include <memory>
#include <optional>
#include <vector>

namespace Scissio {
extern SCISSIO_API std::vector<Vector4i> packRectangles(const Vector2i& maxSize, const std::vector<Vector2i>& items,
                                                        int offset);

class SCISSIO_API Packer {
public:
    explicit Packer(size_t num, const Vector2i& size);
    ~Packer();
    std::optional<Vector2i> add(const Vector2i& size);

private:
    struct Data;
    std::unique_ptr<Data> data;
};
} // namespace Scissio
