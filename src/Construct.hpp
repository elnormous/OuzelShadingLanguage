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
        NONE,
        DECLARATION,
        STATEMENT,
        EXPRESSION
    };

    Construct(Kind initKind): kind(initKind) {}
    virtual ~Construct() {}

    inline Kind getKind() const { return kind; }

    Construct* parent = nullptr;

protected:
    Kind kind = Kind::NONE;
};

#endif // CONSTRUCT_HPP
