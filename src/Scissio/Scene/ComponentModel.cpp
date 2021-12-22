#include "ComponentModel.hpp"

using namespace Scissio;

ComponentModel::ComponentModel(Object& object, AssetModelPtr model) : Component(object), model(std::move(model)) {
}
