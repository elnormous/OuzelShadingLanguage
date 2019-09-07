//
//  OSL
//

#ifndef CONSTRUCT_HPP
#define CONSTRUCT_HPP

class Construct
{
public:
    enum class Kind
    {
        Declaration,
        Statement,
        Expression
    };

    explicit Construct(Kind initKind) noexcept: kind(initKind) {}
    virtual ~Construct() = default;

    inline Kind getKind() const noexcept { return kind; }

    Construct* parent = nullptr;

protected:
    const Kind kind;
};

#endif // CONSTRUCT_HPP