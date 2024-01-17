#include "ComponentModel.hpp"

using namespace Engine;

ComponentModel::ComponentModel(EntityId entity, const ModelPtr& model) : Component{entity} {
    setModel(model);
}
