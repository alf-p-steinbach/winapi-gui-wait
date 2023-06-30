#pragma once    // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").

#include <microlib/support-machinery/basic-types.hpp>           // C_string_ptr
#include <microlib/support-machinery/type-builders.hpp>         // in_, ref_

#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace support_machinery {
    using   std::string, std::to_string,    // <string>
            std::string_view,
            std::is_arithmetic_v,           // <type_traits>
            std::enable_if_t;               // <utility>

    inline namespace string_building {
        
        inline auto operator~( in_<string> s )
            -> C_string_ptr
        { return s.c_str(); }

        struct String_builder: string
        {
            struct Start{};

            String_builder() {}
            String_builder( Start ) {}

            operator C_string_ptr() const { return c_str(); }
        };
        
        constexpr String_builder::Start sb = {};

        inline auto operator<<( String_builder& destination, in_<string_view> source )
            -> String_builder&
        {
            destination.append( source );
            return destination;
        }

        template< class T, bool enabled_ = enable_if_t<is_arithmetic_v<T>, bool>() >
        inline auto operator<<( String_builder& destination, in_<T> value )
            -> String_builder&
        {
            destination.append( to_string( value ) );               // TODO: use from_chars
            return destination;
        }

        template< class T >
        inline auto operator<<( String_builder&& destination, in_<T> source )
            -> String_builder&&
        { return move( destination << source ); }
    }  // namespace string_building
}  // namespace support_machinery
