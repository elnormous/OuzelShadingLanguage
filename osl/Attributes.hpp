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

        explicit Attribute(Kind initAttributeKind):
            Construct{Construct::Kind::Attribute},
            attributeKind{initAttributeKind} {}

        Attribute(const Attribute&) = delete;
        Attribute& operator=(const Attribute&) = delete;

        const Kind attributeKind;
    };

    using AttributeRef = std::reference_wrapper<const Attribute>;

    class BinormalAttribute final: public Attribute
    {
    public:
        explicit BinormalAttribute(std::size_t i) noexcept:
        Attribute{Attribute::Kind::Binormal}, n{i} {}

        const std::size_t n = 0;
    };

    class BlendIndicesAttribute final: public Attribute
    {
    public:
        explicit BlendIndicesAttribute(std::size_t i) noexcept:
            Attribute{Attribute::Kind::BlendIndices}, n{i} {}

        const std::size_t n = 0;
    };

    class BlendWeightAttribute final: public Attribute
    {
    public:
        explicit BlendWeightAttribute(std::size_t i) noexcept:
            Attribute{Attribute::Kind::BlendWeight}, n{i} {}

        const std::size_t n = 0;
    };

    class ColorAttribute final: public Attribute
    {
    public:
        explicit ColorAttribute(std::size_t i) noexcept:
            Attribute{Attribute::Kind::Color}, n{i} {}

        const std::size_t n = 0;
    };

    class DepthAttribute final: public Attribute
    {
    public:
        explicit DepthAttribute(std::size_t i) noexcept:
            Attribute{Attribute::Kind::Depth}, n{i} {}

        const std::size_t n = 0;
    };

    class FogAttribute final: public Attribute
    {
    public:
        FogAttribute() noexcept: Attribute{Attribute::Kind::Fog} {}
    };

    class NormalAttribute final: public Attribute
    {
    public:
        explicit NormalAttribute(std::size_t i) noexcept:
            Attribute{Attribute::Kind::Normal}, n{i} {}

        const std::size_t n = 0;
    };

    class PositionAttribute final: public Attribute
    {
    public:
        explicit PositionAttribute(std::size_t i) noexcept:
            Attribute{Attribute::Kind::Position}, n{i} {}

        const std::size_t n = 0;
    };

    class PositionTransformedAttribute final: public Attribute
    {
    public:
        PositionTransformedAttribute() noexcept: Attribute{Attribute::Kind::PositionTransformed} {}
    };

    class PointSizeAttribute final: public Attribute
    {
    public:
        explicit PointSizeAttribute(std::size_t i) noexcept:
            Attribute{Attribute::Kind::PointSize}, n{i} {}

        const std::size_t n = 0;
    };

    class TangentAttribute final: public Attribute
    {
    public:
        explicit TangentAttribute(std::size_t i) noexcept:
            Attribute{Attribute::Kind::Tangent}, n{i} {}

        const std::size_t n = 0;
    };

    class TesselationFactorAttribute final: public Attribute
    {
    public:
        explicit TesselationFactorAttribute(std::size_t i) noexcept:
            Attribute{Attribute::Kind::TesselationFactor}, n{i} {}

        const std::size_t n = 0;
    };

    class TextureCoordinatesAttribute final: public Attribute
    {
    public:
        explicit TextureCoordinatesAttribute(std::size_t i) noexcept:
            Attribute{Attribute::Kind::TextureCoordinates}, n{i} {}

        const std::size_t n = 0;
    };

    inline std::string toString(Attribute::Kind attributeKind)
    {
        switch (attributeKind)
        {
            case Attribute::Kind::Binormal: return "Binormal";
            case Attribute::Kind::BlendIndices: return "BlendIndices";
            case Attribute::Kind::BlendWeight: return "BlendWeight";
            case Attribute::Kind::Color: return "Color";
            case Attribute::Kind::Depth: return "Depth";
            case Attribute::Kind::Fog: return "Fog";
            case Attribute::Kind::Normal: return "Normal";
            case Attribute::Kind::Position: return "Position";
            case Attribute::Kind::PositionTransformed: return "PositionTransformed";
            case Attribute::Kind::PointSize: return "PointSize";
            case Attribute::Kind::Tangent: return "Tangent";
            case Attribute::Kind::TesselationFactor: return "TesselationFactor";
            case Attribute::Kind::TextureCoordinates: return "TextureCoordinates";
        }

        throw std::runtime_error("Unknown attribute kind");
    }
}

#endif // ATTRIBUTES_HPP
