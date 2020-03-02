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
            Depth,
            Fog,
            Normal,
            Position,
            PositionTransformed,
            PointSize,
            Tangent,
            TesselationFactor,
            TextureCoordinates
        };

        Attribute(const Attribute&) = delete;
        Attribute(Kind initAttributeKind): Construct(Construct::Kind::Attribute), attributeKind(initAttributeKind) {}

        Attribute& operator=(const Attribute&) = delete;

        inline Kind getAttributeKind() const noexcept { return attributeKind; }

    private:
        const Kind attributeKind;
    };

    class BinormalAttribute final: public Attribute
    {
    public:
        BinormalAttribute(size_t i) noexcept:
            Attribute(Attribute::Kind::Binormal), n(i) {}

        size_t n = 0;
    };

    class BlendIndicesAttribute final: public Attribute
    {
    public:
        BlendIndicesAttribute(size_t i) noexcept:
            Attribute(Attribute::Kind::BlendIndices), n(i) {}

        size_t n = 0;
    };

    class BlendWeightAttribute final: public Attribute
    {
    public:
        BlendWeightAttribute(size_t i) noexcept:
            Attribute(Attribute::Kind::BlendWeight), n(i) {}

        size_t n = 0;
    };

    class ColorAttribute final: public Attribute
    {
    public:
        ColorAttribute(size_t i) noexcept:
            Attribute(Attribute::Kind::Color), n(i) {}

        size_t n = 0;
    };

    class DepthAttribute final: public Attribute
    {
    public:
        DepthAttribute(size_t i) noexcept:
            Attribute(Attribute::Kind::Depth), n(i) {}

        size_t n = 0;
    };

    class FogAttribute final: public Attribute
    {
    public:
        FogAttribute() noexcept: Attribute(Attribute::Kind::Fog) {}
    };

    class NormalAttribute final: public Attribute
    {
    public:
        NormalAttribute(size_t i) noexcept:
            Attribute(Attribute::Kind::Normal), n(i) {}

        size_t n = 0;
    };

    class PositionAttribute final: public Attribute
    {
    public:
        PositionAttribute(size_t i) noexcept:
            Attribute(Attribute::Kind::Position), n(i) {}

        size_t n = 0;
    };

    class PositionTransformedAttribute final: public Attribute
    {
    public:
        PositionTransformedAttribute() noexcept: Attribute(Attribute::Kind::PositionTransformed) {}
    };

    class PointSizeAttribute final: public Attribute
    {
    public:
        PointSizeAttribute(size_t i) noexcept:
            Attribute(Attribute::Kind::PointSize), n(i) {}

        size_t n = 0;
    };

    class TangentAttribute final: public Attribute
    {
    public:
        TangentAttribute(size_t i) noexcept:
            Attribute(Attribute::Kind::Tangent), n(i) {}

        size_t n = 0;
    };

    class TesselationFactorAttribute final: public Attribute
    {
    public:
        TesselationFactorAttribute(size_t i) noexcept:
            Attribute(Attribute::Kind::TesselationFactor), n(i) {}

        size_t n = 0;
    };

    class TextureCoordinatesAttribute final: public Attribute
    {
    public:
        TextureCoordinatesAttribute(size_t i) noexcept:
            Attribute(Attribute::Kind::TextureCoordinates), n(i) {}

        size_t n = 0;
    };
}

#endif // ATTRIBUTES_HPP
