//
//  OSL
//

#ifndef ATTRIBUTES_HPP
#define ATTRIBUTES_HPP

#include "Construct.hpp"

namespace ouzel
{
    class Attribute: public Construct
    {
    public:
        enum class Kind
        {
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
            TextureCoordinates
        };

        Attribute(Kind initAttributeKind): Construct(Construct::Kind::Attribute), attributeKind(initAttributeKind) {}

        inline Kind getAttributeKind() const noexcept { return attributeKind; }

    protected:
        const Kind attributeKind;
    };

    class BinormalAttribute final: public Attribute
    {
    public:
        BinormalAttribute(): Attribute(Attribute::Kind::Binormal) {}

        size_t n = 0;
    };

    class BlendIndicesAttribute final: public Attribute
    {
    public:
        BlendIndicesAttribute(): Attribute(Attribute::Kind::BlendIndices) {}

        size_t n = 0;
    };

    class BlendWeightAttribute final: public Attribute
    {
    public:
        BlendWeightAttribute(): Attribute(Attribute::Kind::BlendWeight) {}

        size_t n = 0;
    };

    class ColorAttribute final: public Attribute
    {
    public:
        ColorAttribute(): Attribute(Attribute::Kind::Color) {}

        size_t n = 0;
    };

    class FogAttribute final: public Attribute
    {
    public:
        FogAttribute(): Attribute(Attribute::Kind::Fog) {}
    };

    class NormalAttribute final: public Attribute
    {
    public:
        NormalAttribute(): Attribute(Attribute::Kind::Normal) {}

        size_t n = 0;
    };

    class PositionAttribute final: public Attribute
    {
    public:
        PositionAttribute(): Attribute(Attribute::Kind::Position) {}

        size_t n = 0;
    };

    class PositionTransformedAttribute final: public Attribute
    {
    public:
        PositionTransformedAttribute(): Attribute(Attribute::Kind::PositionTransformed) {}
    };

    class PointSizeAttribute final: public Attribute
    {
    public:
        PointSizeAttribute(): Attribute(Attribute::Kind::PointSize) {}

        size_t n = 0;
    };

    class TangentAttribute final: public Attribute
    {
    public:
        TangentAttribute(): Attribute(Attribute::Kind::Tangent) {}

        size_t n = 0;
    };

    class TesselationFactorAttribute final: public Attribute
    {
    public:
        TesselationFactorAttribute(): Attribute(Attribute::Kind::TesselationFactor) {}

        size_t n = 0;
    };

    class TextureCoordinatesAttribute final: public Attribute
    {
    public:
        TextureCoordinatesAttribute(): Attribute(Attribute::Kind::TextureCoordinates) {}

        size_t n = 0;
    };
}

#endif // ATTRIBUTES_HPP
