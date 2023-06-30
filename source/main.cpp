 // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").
#include "resources.h"          // IDR_SPRITES

#include <microlib/support-machinery.hpp>

#include <microlib/winapi++.hpp>
#include <microlib/winapi-header-wrappers/windowsx-h.for-utf8.hpp>   // Message crackers, e.g. HANDLE_WM_CLOSE.

#include <algorithm>
#include <functional>           // std::invoke
#include <memory>               // std::(unique_ptr, make_unique)
#include <string>               // std::(string, to_string)
#include <string_view>          // std::string_view
#include <optional>
#include <utility>

#include <stdio.h>              // fprintf
#include <stdlib.h>             // EXIT_SUCCESS, EXIT_FAILURE

#define FAIL SM_FAIL

namespace app {
    namespace sm = support_machinery;
    using namespace sm::string_building;            // operator<<
    using   sm::const_, sm::ref_, sm::in_,
            sm::hopefully, sm::C_string_ptr,
            sm::zero_to,
            sm::pop_exception, sm::push_current_exception;
    using   std::min,                           // <algorithm>
            std::invoke,                        // <functional>
            std::unique_ptr, std::make_unique,  // <memory>
            std::string, std::to_string,        // <string>
            std::string_view,
            std::optional,
            std::move;                          // <utility>

    namespace main_window {
        struct Cmd{ enum Enum: int { exit = 100, mystery }; };

        struct State
        {
            string                          basic_title;
            winapi::Unique_bmp_handle       h_sprites_bitmap;
            winapi::Unique_brush_handle     h_bg_brush;
            
            State( string a_title ):
                basic_title( move( a_title ) ),
                h_sprites_bitmap( 0 ),
                h_bg_brush( 0 )
            {
                // Saving the image with MS Paint ensures that it's compatible with `LoadImage`.
                h_sprites_bitmap = static_cast<HBITMAP>( ::LoadImage(
                    winapi::h_instance,
                    winapi::as_pseudo_ptr( IDR_SPRITES ),
                    IMAGE_BITMAP,
                    0, 0,                           // w, h
                    LR_CREATEDIBSECTION             // DIB instead of device compatible
                    ) );
                hopefully( h_sprites_bitmap != 0 )
                    or FAIL( sb << "Failed to load sprites bitmap, error code " << GetLastError() << "." );
      
                // h_bg_brush = ::CreatePatternBrush( h_sprites_bitmap );
                // hopefully( h_bg_brush != 0 )
                    // or FAIL( "Failed to create background fill brush, error code " + to_string( GetLastError() ) + "." );
            }
            
            ~State() {}
        };
        
        unique_ptr<State>   p_state;
        
        void basic_fill_background( const HWND window, const HDC dc, const RECT& rect )
        {
            const HBRUSH fill = ::CreateSolidBrush( RGB( 0xFF, 0x80, 0 ) );
            ::FillRect( dc, &rect, fill );
            ::DeleteObject( fill );
        }

        void fill_background( const HWND window, const HDC dc, const RECT& update_rect )
        {
            basic_fill_background( window, dc, update_rect );
        }

        void fill_control_background( const HWND window, const int control_id, const HDC dc, const RECT& update_rect )
        {
            (void) control_id;
            basic_fill_background( window, dc, update_rect );
        }

        auto i_first_difference( in_<string_view> a, in_<string_view> b )
            -> int
        {
            const int n_bytes = static_cast<int>( min( a.length(), b.length() ) );
            for( const int i: zero_to( n_bytes ) ) {
                if( a[i] != b[i] ) { return i; }
            }
            return (a.length() == b.length()? -1 : n_bytes);
        }

        void on_command( const HWND window, const int id )
        {
            // MessageBox( window,
                // sb << "Button press, id " << id << ".",
                // ~p_state->basic_title,
                // MB_SETFOREGROUND | MB_ICONINFORMATION
                // );
            winapi::message_box( window, sb << "Button press, id " << id << "." );
        }

        void on_simple_notification( const HWND window, const HWND control, const int notification )
        {
            (void) window; (void) control; (void) notification;
        }

        auto on_nm_custom_draw( const HWND window, const int control_id, const_<const NMCUSTOMDRAW*> p_info )
            -> optional<LRESULT>
        {
            if( p_info->dwDrawStage == CDDS_PREERASE ) {
                RECT client_rect;
                ::GetClientRect( p_info->hdr.hwndFrom, &client_rect );
                fill_control_background( window, control_id, p_info->hdc, client_rect );
                return 1;       // !Undocumented, significant: says the erasure's done.
            }
            return {};
        }
                        
        auto is_button( const HWND ) -> bool { return true; }

        namespace message_handlers {
            // The handler signature comments are from the Visual C++ version of `<windowsx.h>`.
            // The g++ version of that header is unfortunately sans comments.
            
            // void Cls_OnClose(HWND hwnd)
            void on_wm_close( const HWND )
            {
                ::PostQuitMessage( 0 );
            }

            // "void Cls_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)"
            void on_wm_command( const HWND window, const int id, const HWND control, const int notification )
            {
                if( control == 0 ) {
                    const bool  from_menu           = (notification == 0);      (void) from_menu;
                    const bool  from_accelerator    = (notification == 1);      (void) from_accelerator;
                    on_command( window, id );
                } else if( is_button( control ) and notification == BN_CLICKED ) {
                    on_command( window, id );
                } else {
                    on_simple_notification( window, control, notification );
                }
            }

            // "BOOL Cls_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)"
            auto on_wm_create( const HWND window, CREATESTRUCT* p_params )
                -> bool
            {
                // p_params->lpszName is buggy, possibly a truncated UTF-8 back-translation, so:
                (void) p_params;
                p_state = make_unique<State>( winapi::title_of( window ) );

                using namespace winapi::option_parameter_types;
                winapi::new_child_window_of( window, "button",
                    With_position{ 10, 10 },
                    With_rect_size{ 120, winapi::good_button_height },
                    With_text{ "Click me" },
                    With_id{ Cmd::exit },
                    With_styles{ BS_DEFPUSHBUTTON | WS_VISIBLE }
                    )
                    or FAIL( "Failed to create button." );
                return true;
            }

            // "void Cls_OnDestroy(HWND hwnd)"
            void on_wm_destroy( const HWND )
            {
                p_state.reset();
            }
            
            /* BOOL Cls_OnEraseBkgnd(HWND hwnd, HDC hdc) */
            auto on_wm_erasebkgnd( const HWND window, const HDC dc )
                -> bool
            {
                RECT update_rect;
                ::GetUpdateRect( window, &update_rect, false );
                fill_background( window, dc, update_rect );
                // In Windows 11 it looks like the return value is ignored. Must call DefWindowProc
                // to get the default erasing. But per docs `true` means the job is done.
                return true;
            }

            // No message cracker.
            auto on_wm_notify( const HWND window, const int control_id, const_<const NMHDR*> p_header )
                -> optional<LRESULT>
            {
                switch( p_header->code ) {
                    case NM_CUSTOMDRAW: return on_nm_custom_draw(
                        window, control_id, reinterpret_cast<const NMCUSTOMDRAW*>( p_header )
                        );
                }
                return {};
            }
        }  // namespace message_handlers

        auto message_handler(
            const HWND      window,
            const UINT      msg_id,
            const WPARAM    w_param,
            const LPARAM    ell_param
            )
            -> LRESULT
        {
            try {
                #define CASE( m, f ) case m: return HANDLE_##m( window, w_param, ell_param, f )
                switch( msg_id ) {
                    CASE( WM_CLOSE,         message_handlers::on_wm_close );
                    CASE( WM_COMMAND,       message_handlers::on_wm_command );
                    CASE( WM_CREATE,        message_handlers::on_wm_create );
                    CASE( WM_ERASEBKGND,    message_handlers::on_wm_erasebkgnd );

                    case WM_NOTIFY:         {
                        const optional<LRESULT> retvalue = message_handlers::on_wm_notify(
                            window,
                            static_cast<int>( w_param ),                                            // Control id.
                            reinterpret_cast<const NMHDR*>( static_cast<uintptr_t>( ell_param ) )   // NMHDR
                            );
                        if( retvalue ) { return *retvalue; }
                        break;
                    }
                }
                #undef CASE
            } catch( ... ) {
                pop_exception();        // Shouldn't be necessary, but.
                push_current_exception();
            }
            return ::DefWindowProc( window, msg_id, w_param, ell_param );
        }

        auto windowclass_id()
            -> ATOM
        {
            static const ATOM the_id = invoke( []() -> ATOM
            {
                auto params = winapi::Window_class_params::dialog_colored();
                params.lpfnWndProc = message_handler;
                const ATOM result = ::RegisterClass( &params );
                hopefully( result != 0 ) or FAIL( "::RegisterClass failed." );
                return result;
            } );

            return the_id;
        }

        auto new_titled( const C_string_ptr title )
            -> HWND
        {
            using namespace winapi::option_parameter_types;
            const HWND window = winapi::new_toplevel_window( windowclass_id(),
                With_title{ title },
                With_icon{ ::LoadIcon( 0, IDI_INFORMATION ) }
                );
            return window;
        }
    }  // namespace main_window

    void run()
    {
        SM_WITH( comctl32::Library_envelope() ) {   // Initialization for modern look and feel.
            const HWND window = main_window::new_titled( "日本国 кошка 🐈" );
            ShowWindow( window, SW_SHOWDEFAULT );
            try {
                winapi::dispatch_messages();
            } catch( ... ) {
                FAIL( "Something failed, I’m terminating; really sorry!" );
            }
        }
    }
}  // namespace app


//----------------------------------------------------------------------------------

namespace sm = support_machinery;

auto main() -> int
{
    using   sm::in_, sm::messages_of, sm::zero_to, sm::int_size_of, sm::operator~;
    using   std::exception, std::string, std::vector;

    // Set the console window, if any, to UTF-8 as output encoding assumption.
    fprintf( stderr, "Original " );  fflush( stderr );  system( "chcp 1>&2" );
    system( "chcp 65001 >nul" );

    try {
        app::run();
        fprintf( stderr, "Finished!\n" );
        return EXIT_SUCCESS;
    } catch( in_<exception> x ) {
        const vector<string> messages = messages_of( x );
        for( const int i: zero_to( int_size_of( messages ) ) ) {
            if( i > 0 ) {
                fprintf( stderr, "    Because: " );
            }
            fprintf( stderr, "!%s\n", ~messages[i] );
        }
    }
    return EXIT_FAILURE;
}
