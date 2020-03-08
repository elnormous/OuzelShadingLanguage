//
//  OSL
//

#ifndef CONSTRUCT_HPP
#define CONSTRUCT_HPP

namespace ouzel
{
    class Construct
    {
    public:
        enum class Kind
        {
            Declaration,
            Statement,
            Expression,
            Attribute
        };

        explicit Construct(Kind initKind) noexcept: kind(initKind) {}
        virtual ~Construct() = default;
        Construct(const Construct&) = delete;
        Construct& operator=(const Construct&) = delete;
        Construct(Construct&&) = delete;
        Construct& operator=(Construct&&) = delete;

        const Kind kind;
    };
}

#endif // CONSTRUCT_HPP
