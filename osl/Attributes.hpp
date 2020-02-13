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
            Fog,
            Normal,
            Position,
            PositionTransformed,
            PointSize,
            Tangent,
            TesselationFactor,
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

        size_t n = 0;
    };

    class BlendIndicesAttribute: public Attribute
    {
    public:
        BlendIndicesAttribute(): Attribute(Attribute::Kind::BlendIndices) {}

        size_t n = 0;
    };

    class BlendWeightAttribute: public Attribute
    {
    public:
        BlendWeightAttribute(): Attribute(Attribute::Kind::BlendWeight) {}

        size_t n = 0;
    };

    class ColorAttribute: public Attribute
    {
    public:
        ColorAttribute(): Attribute(Attribute::Kind::Color) {}

        size_t n = 0;
    };

    class FogAttribute: public Attribute
    {
    public:
        FogAttribute(): Attribute(Attribute::Kind::Fog) {}
    };

    class NormalAttribute: public Attribute
    {
    public:
        NormalAttribute(): Attribute(Attribute::Kind::Normal) {}

        size_t n = 0;
    };

    class PositionAttribute: public Attribute
    {
    public:
        PositionAttribute(): Attribute(Attribute::Kind::Position) {}

        size_t n = 0;
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

        size_t n = 0;
    };

    class TangentAttribute: public Attribute
    {
    public:
        TangentAttribute(): Attribute(Attribute::Kind::Tangent) {}

        size_t n = 0;
    };

    class TesselationFactorAttribute: public Attribute
    {
    public:
        TesselationFactorAttribute(): Attribute(Attribute::Kind::TesselationFactor) {}

        size_t n = 0;
    };

    class TextureCoordinatesAttribute: public Attribute
    {
    public:
        TextureCoordinatesAttribute(): Attribute(Attribute::Kind::TextureCoordinates) {}

        size_t n = 0;
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
