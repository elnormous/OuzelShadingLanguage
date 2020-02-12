//
//  OSL
//

#ifndef ATTRIBUTES_HPP
#define ATTRIBUTES_HPP

#include "Construct.hpp"

namespace ouzel
{
    enum class Program
    {
        Fragment,
        Vertex
    };

    enum class Semantic
    {
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

    class Attribute: public Construct
    {
    public:
        enum class Kind
        {
            Fragment,
            Vertex,
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

        Attribute(Kind initAttributeKind): Construct(Construct::Kind::Attribute), attributeKind(initAttributeKind) {}

        inline Kind getAttributeKind() const noexcept { return attributeKind; }

    protected:
        const Kind attributeKind;
    };
}

#endif // ATTRIBUTES_HPP
