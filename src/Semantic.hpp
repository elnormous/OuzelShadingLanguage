//
//  OSL
//

#ifndef SEMANTIC_HPP
#define SEMANTIC_HPP

#include <string>

enum class Semantic
{
    NONE,
    BINORMAL,
    BLEND_INDICES,
    BLEND_WEIGHT,
    COLOR,
    NORMAL,
    POSITION,
    POSITION_TRANSFORMED,
    POINT_SIZE,
    TANGENT,
    TEXTURE_COORDINATES
};

inline std::string toString(Semantic semantic)
{
    switch (semantic)
    {
        case Semantic::NONE: return "NONE";
        case Semantic::BINORMAL: return "BINORMAL";
        case Semantic::BLEND_INDICES: return "BLEND_INDICES";
        case Semantic::BLEND_WEIGHT: return "BLEND_WEIGHT";
        case Semantic::COLOR: return "COLOR";
        case Semantic::NORMAL: return "NORMAL";
        case Semantic::POSITION: return "POSITION";
        case Semantic::POSITION_TRANSFORMED: return "POSITION_TRANSFORMED";
        case Semantic::POINT_SIZE: return "POINT_SIZE";
        case Semantic::TANGENT: return "TANGENT";
        case Semantic::TEXTURE_COORDINATES: return "TEXTURE_COORDINATES";
        default: return "Unknown";
    }
}

#endif // SEMANTIC_HPP
