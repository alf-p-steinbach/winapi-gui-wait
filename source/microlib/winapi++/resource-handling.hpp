#pragma once    // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").

#include <microlib/winapi-header-wrappers/windows-h.for-utf8.hpp>

#include <type_traits>
#include <string>

namespace winapi {
    namespace sm = support_machinery;
    using   sm::const_, sm::in_,
            sm::bits_per_;
    using   std::is_integral_v,                 // <type_traits>
            std::string, std::to_string;        // <string>

    const HINSTANCE h_instance = ::GetModuleHandle( nullptr );

    // Windows' `ATOM` is an integer type with 16 significant bits.
    static_assert( is_integral_v<ATOM> );
    static_assert( bits_per_<ATOM> >= 16 );

    inline auto as_pseudo_ptr( const ATOM value )
        -> const char*
    { return MAKEINTATOM( value ); }
    
    inline auto is_pseudo_ptr( const_<const char*> p )
        -> bool
    { return !!IS_INTRESOURCE( p ); }

    inline auto id_from_pseudo_ptr( const_<const char*> p )
        -> ATOM
    { return (is_pseudo_ptr( p )? static_cast<ATOM>( reinterpret_cast<uintptr_t>( p ) ) : 0); }
    
    inline auto atom_id_string( const ATOM id )
        -> string
    { return "#" + to_string( id ); }
}  // namespace winapi
