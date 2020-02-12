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
            TextureCoordinates,
            In,
            Inout,
            Out
        };

        Attribute(Kind initAttributeKind): Construct(Construct::Kind::Attribute), attributeKind(initAttributeKind) {}

        inline Kind getAttributeKind() const noexcept { return attributeKind; }

    protected:
        const Kind attributeKind;
    };

    class FragmentAttribute: public Attribute
    {
    public:
        FragmentAttribute(): Attribute(Attribute::Kind::Fragment) {}
    };

    class VertexAttribute: public Attribute
    {
    public:
        VertexAttribute(): Attribute(Attribute::Kind::Vertex) {}
    };

    class BinormalAttribute: public Attribute
    {
    public:
        BinormalAttribute(): Attribute(Attribute::Kind::Binormal) {}
    };

    class BlendIndicesAttribute: public Attribute
    {
    public:
        BlendIndicesAttribute(): Attribute(Attribute::Kind::BlendIndices) {}
    };

    class BlendWeightAttribute: public Attribute
    {
    public:
        BlendWeightAttribute(): Attribute(Attribute::Kind::BlendWeight) {}
    };

    class ColorAttribute: public Attribute
    {
    public:
        ColorAttribute(): Attribute(Attribute::Kind::Color) {}
    };

    class NormalAttribute: public Attribute
    {
    public:
        NormalAttribute(): Attribute(Attribute::Kind::Normal) {}
    };

    class PositionAttribute: public Attribute
    {
    public:
        PositionAttribute(): Attribute(Attribute::Kind::Position) {}
    };

    class PositionTransformedAttribute: public Attribute
    {
    public:
        PositionTransformedAttribute(): Attribute(Attribute::Kind::PositionTransformed) {}
    };

    class PointSizeAttribute: public Attribute
    {
    public:
        PointSizeAttribute(): Attribute(Attribute::Kind::PointSize) {}
    };

    class TangentAttribute: public Attribute
    {
    public:
        TangentAttribute(): Attribute(Attribute::Kind::Tangent) {}
    };

    class TextureCoordinatesAttribute: public Attribute
    {
    public:
        TextureCoordinatesAttribute(): Attribute(Attribute::Kind::TextureCoordinates) {}
    };

    class InAttribute: public Attribute
    {
    public:
        InAttribute(): Attribute(Attribute::Kind::In) {}
    };

    class InoutAttribute: public Attribute
    {
    public:
        InoutAttribute(): Attribute(Attribute::Kind::Inout) {}
    };

    class OutAttribute: public Attribute
    {
    public:
        OutAttribute(): Attribute(Attribute::Kind::Out) {}
    };

}

#endif // ATTRIBUTES_HPP
