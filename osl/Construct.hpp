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

        explicit Construct(Kind initKind) noexcept: kind{initKind} {}
        virtual ~Construct() = default;
        Construct(const Construct&) = delete;
        Construct& operator=(const Construct&) = delete;
        Construct(Construct&&) = delete;
        Construct& operator=(Construct&&) = delete;

        const Kind kind;
    };

    inline std::string toString(Construct::Kind kind)
    {
        switch (kind)
        {
            case Construct::Kind::Declaration: return "Declaration";
            case Construct::Kind::Statement: return "Statement";
            case Construct::Kind::Expression: return "Expression";
            case Construct::Kind::Attribute: return "Attribute";
        }

        throw std::runtime_error{"Unknown construct kind"};
    }
}

#endif // CONSTRUCT_HPP
