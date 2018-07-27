//
//  OSL
//

#pragma once

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

    inline Kind getKind() const { return kind; }

    Construct* parent = nullptr;

protected:
    Kind kind = Kind::NONE;
};
