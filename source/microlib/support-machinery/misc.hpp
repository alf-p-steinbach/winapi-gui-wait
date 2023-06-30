#pragma once    // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").

#include <exception>            // std::(exception_ptr, 
#include <functional>           // std::(reference_wrapper, function)
#include <iterator>             // std::size
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>          // std::is_base_of
#include <utility>
#include <vector>

#include <microlib/support-machinery/type-builders.hpp>         // in_

#define SM_WITH( ... )  if( const auto& _ = __VA_ARGS__; ((void)_, true) or true )
    
namespace support_machinery {
    using   std::current_exception, std::exception_ptr,         // <exception>
            std::reference_wrapper, std::function,              // <functional>
            std::size,                                          // <iterator>
            std::optional,
            std::exception, std::runtime_error,                 // <stdexcept>
            std::string,
            std::string_view,
            std::forward, std::move,                            // <utility>
            std::vector;

    template< class Derived, class Base >
    constexpr bool is_derived_and_base_  = std::is_base_of_v<Base, Derived>;    // (sic)

    template< class Container >
    constexpr auto int_size_of( in_<Container> c )
        -> int
    { return static_cast<int>( size( c ) ); }

    template< class... Supported_options >
    class Option_refs_:
        public optional<reference_wrapper<const Supported_options>>...
    {
    public:
        // Compilation error if one of the values is of unsupported type.
        template< class... Options >
        Option_refs_( in_<Options>... options ):
            optional<reference_wrapper<const Options>>( options )...
        {}

        // Empty optional if the Option is not present.
        template< class Option >
        auto get_() const
            -> optional<reference_wrapper<const Option>>
        { return *static_cast<const optional<reference_wrapper<const Option>>*>( this ); }
        
        template< class Option >
        auto has_() const
            -> bool
        { return get_<Option>().has_value(); }

        template< class Option >
        auto get_with_default( in_<Option> a_default ) const
            -> ref_<const Option>
        {
            const optional<reference_wrapper<const Option>> option = get_<Option>();
            return (option.has_value()? option.value().get() : a_default);
        }

        template< class Option >
        auto operator|( in_<Option> a_default ) const
            -> ref_<const Option>
        { return get_with_default( a_default ); }
    };

    // Convenience base class for option types.
    template< class Value >
    struct Option_
    {
        Value   value;
        operator ref_<const Value> () const { return value; }
    };

    template< class... Types >
    struct Type_list_
    {
        template< class... More_types >
        using Plus_ = Type_list_< Types..., More_types... >;
        
        template< template< class... > class Foo_ >
        using Specialization_of_ = Foo_<Types...>;
    };
    
    class Non_copyable      // Is not movable either.
    {
        using Self = Non_copyable;
        Non_copyable( in_<Self> ) = delete;
        auto operator=( in_<Self> ) -> ref_<Self> = delete;
        
    public:
        Non_copyable() {}
    };
}  // namespace support_machinery
