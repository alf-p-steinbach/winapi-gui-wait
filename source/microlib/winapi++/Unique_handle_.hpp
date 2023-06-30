#pragma once    // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").

#include <microlib/winapi-header-wrappers/windows-h.for-utf8.hpp>

namespace winapi {
    template< class Handle, void(*destroy)(Handle) >
    class Unique_handle_
    {
        Handle      m_handle;
        
    public:
        ~Unique_handle_() { destroy( m_handle ); }
        Unique_handle_( const Handle handle ): m_handle( handle ) {}
        
        auto value() const -> Handle { return m_handle; }
        operator Handle() const { return value(); }
    };
    
    inline void destroy_bitmap( const HBITMAP bmp ) { ::DeleteObject( bmp ); }
    inline void destroy_brush( const HBRUSH br ) { ::DeleteObject( br ); }

    using Unique_bmp_handle     = Unique_handle_<HBITMAP, destroy_bitmap>;
    using Unique_brush_handle   = Unique_handle_<HBRUSH, destroy_brush>;
}  // namespace winapi
