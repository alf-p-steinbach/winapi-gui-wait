#pragma once    // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").

#include <limits.h>     // CHAR_BIT
#include <stddef.h>     // size_t, ptrdiff_t

#include <type_traits>

namespace support_machinery {
    using   std::is_unsigned_v, std::make_signed_t;     // <type_traits>

    using Byte                  = unsigned char;
    using C_string_ptr          = const char*;
    using Mutable_cstr_ptr      = char*;

    constexpr int bits_per_byte = CHAR_BIT;
    template< class Type > constexpr int bits_per_ = sizeof( Type )*bits_per_byte;
 
    template< class Unsigned >
    constexpr auto as_signed( const Unsigned value )
        -> auto
    {
        static_assert( is_unsigned_v<Unsigned> );
        return static_cast<make_signed_t<Unsigned>>( value );
    }

    // The direct signed equivalent of `size_t` is convenient for `npos` < 0.
    using Signed_size_t = make_signed_t<size_t>;
    
    using Size          = Signed_size_t;
    using Index         = Signed_size_t;

}  // namespace support_machinery
