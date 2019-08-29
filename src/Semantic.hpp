//
//  OSL
//

#ifndef SEMANTIC_HPP
#define SEMANTIC_HPP

#include <string>

enum class Semantic
{
    None,
    Binormal,
    BlendIndices,
    BlendWeight,
    Color,
    Normal,
    Position,
    PositionTransformed,
    PointSize,
    Tangent,
    TextureCoordinates
};

inline std::string toString(Semantic semantic)
{
    switch (semantic)
    {
        case Semantic::None: return "None";
        case Semantic::Binormal: return "Binormal";
        case Semantic::BlendIndices: return "BlendIndices";
        case Semantic::BlendWeight: return "BlendWeight";
        case Semantic::Color: return "Color";
        case Semantic::Normal: return "Normal";
        case Semantic::Position: return "Position";
        case Semantic::PositionTransformed: return "PositionTransformed";
        case Semantic::PointSize: return "PointSize";
        case Semantic::Tangent: return "Tangent";
        case Semantic::TextureCoordinates: return "TextureCoordinates";
        default: return "Unknown";
    }
}

#endif // SEMANTIC_HPP
