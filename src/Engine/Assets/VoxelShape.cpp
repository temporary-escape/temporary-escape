#include "VoxelShape.hpp"

using namespace Engine;

const std::array<VoxelShape::Type, VoxelShape::numOfShapes> VoxelShape::allTypes = {
    Cube,
    Wedge,
    Corner,
    Penta,
};

const std::array<std::string, VoxelShape::numOfShapes> VoxelShape::typeNames = {
    "shape_cube",
    "shape_wedge",
    "shape_corner",
    "shape_penta",
};

const std::array<std::string, VoxelShape::numOfShapes> VoxelShape::typeFriendlyNames = {
    "Cube",
    "Wedge",
    "Corner",
    "Penta",
};

const std::array<VoxelShape::Face, 7> VoxelShape::allSides = {
    Face::Default,
    Face::PositiveX,
    Face::NegativeX,
    Face::PositiveY,
    Face::NegativeY,
    Face::PositiveZ,
    Face::NegativeZ,
};
