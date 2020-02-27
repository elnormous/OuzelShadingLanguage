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

        inline Kind getKind() const noexcept { return kind; }

    private:
        const Kind kind;
    };

    class BadAccessError final: public std::runtime_error
    {
    public:
        explicit BadAccessError(const std::string& str): std::runtime_error(str) {}
        explicit BadAccessError(const char* str): std::runtime_error(str) {}
    };

    template <typename First, typename Second>
    class Variant final
    {
    public:
        Variant() noexcept = default;
        template <class T>
        Variant(T value) noexcept: type(TypeId<T>::id), pointer(value) {}

        template <class T>
        Variant& operator=(T value) noexcept
        {
            type = TypeId<T>::id;
            pointer = value;
            return *this;
        }

        template <class T>
        bool is() const noexcept { return type == TypeId<T>::id; }

        template <class T>
        const T get() const
        {
            if (type != TypeId<T>::id) throw BadAccessError("Wrong type");
            return static_cast<const T>(pointer);
        }

        template <class T>
        T get()
        {
            if (type != TypeId<T>::id) throw BadAccessError("Wrong type");
            return static_cast<T>(pointer);
        }

    private:
        template<typename T> struct TypeId;
        template<> struct TypeId<std::nullptr_t> { static constexpr size_t id = 0; };
        template<> struct TypeId<First> { static constexpr size_t id = 1; };
        template<> struct TypeId<Second> { static constexpr size_t id = 2; };

        size_t type = 0;
        void* pointer = nullptr;
    };
}

#endif // CONSTRUCT_HPP
