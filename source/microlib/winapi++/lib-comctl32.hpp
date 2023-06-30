#pragma once    // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").
#include <microlib/winapi-header-wrappers/windows-h.for-utf8.hpp>       // `UNICODE` not defined.
#include <commctrl.h>

#include <microlib/support-machinery.hpp>       // SM_FAIL, Non_copyable

namespace comctl32 {
    namespace sm = support_machinery;
    using   sm::Non_copyable;

    constexpr DWORD minimum_classes = ICC_STANDARD_CLASSES;

    struct Library_envelope: Non_copyable
    {
        Library_envelope( const DWORD classes = minimum_classes )
        {
            INITCOMMONCONTROLSEX params = {sizeof( params ), classes };
            ::InitCommonControlsEx( &params )
                or SM_FAIL( "::InitCommonControlsEx failed." );
        }
    };
}  // namespace comctl32
