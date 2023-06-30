#pragma once    // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").

namespace support_machinery {

    template< class Type > using const_ = const Type;
    template< class Type > using ref_   = Type&;
    template< class Type > using in_    = ref_<const Type>;

}  // namespace support_machinery
