#pragma once    // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").

#include <microlib/support-machinery.hpp>                           // SM_FAIL
#include <microlib/winapi-header-wrappers/windows-h.for-utf8.hpp>
#include <microlib/winapi++/resource-handling.hpp>                  // h_instance

#include <assert.h>         // assert
#include <stddef.h>         // offsetof
#include <stdint.h>         // uintptr_t

#include <stdexcept>
#include <string>
#include <variant>

namespace winapi {
    namespace sm = support_machinery;
    using   std::exception,                     // <stdexcept>
            std::string, std::to_string,        // <string>
            std::variant, std::get;             // <variant>
    using   sm::const_, sm::ref_, sm::in_,
            sm::rethrow_popped_exception,
            sm::hopefully,
            sm::C_string_ptr,
            sm::Option_,
            sm::Option_refs_,
            sm::Type_list_;

    using   Point       = POINT;
    using   Rect        = RECT;
    using   Rect_size   = SIZE;

    constexpr auto width_of( in_<Rect> r )  -> long         { return r.right - r.left; }
    constexpr auto height_of( in_<Rect> r ) -> long         { return r.bottom - r.top; }
    constexpr auto size_of( in_<Rect> r )   -> Rect_size    { return { width_of( r ), height_of( r ) }; }

    auto operator+( in_<Point> pt, in_<Rect_size> offset )
        -> Point
    { return {pt.x + offset.cx, pt.y + offset.cy}; }
    
    auto operator-( in_<Point> pt, in_<Rect_size> offset )
        -> Point
    { return {pt.x - offset.cx, pt.y - offset.cy}; }

    constexpr int good_button_height = 23;          // pixels

    inline auto std_cursor( const_<const char*> id = IDC_ARROW )
        -> HCURSOR
    { return ::LoadCursor( 0, id ); }

    // This could possibly be just a call to `::GetSysColorBrush`. Not sure of difference, if any,
    // except “An application must not register a window class for a window using a system brush.”.
    // A pseudo-brush from this function is OK, though, per the `RegisterClass` documentation.
    inline auto std_pseudobrush( const int id = COLOR_WINDOW )  // For type see GetSysColor
        -> HBRUSH
    { return reinterpret_cast<HBRUSH>( static_cast<uintptr_t>( id + 1 ) ); }
    
    inline auto set_icon_of( const HWND window, const HICON icon )
        -> HICON    // The old now replaced icon.
    {
        return reinterpret_cast<HICON>(
            ::SendMessage( window, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>( icon ) )
            );
    }

    inline auto icon_of( const HWND window )
        -> HICON
    {
        return reinterpret_cast<HICON>(
            ::SendMessage( window, WM_GETICON, ICON_SMALL, {} )
            );
    }
            
    inline auto set_app_icon_of( const HWND window, const HICON icon )
        -> HICON    // The old now replaced icon.
    {
        return reinterpret_cast<HICON>(
            ::SendMessage( window, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>( icon ) )
            );
    }

    inline auto app_icon_of( const HWND window )
        -> HICON
    {
        return reinterpret_cast<HICON>(
            ::SendMessage( window, WM_GETICON, ICON_BIG, {} )
            );
    }
            
    inline void set_font_of( const HWND window, const HFONT font )
    {
        ::SendMessage( window, WM_SETFONT, reinterpret_cast<WPARAM>( font ), true );
    }

    inline auto get_ui_font_spec()   // Costly
        -> LOGFONT
    {
        NONCLIENTMETRICS info = {sizeof( NONCLIENTMETRICS )};
        
        ::SystemParametersInfo( SPI_GETNONCLIENTMETRICS, info.cbSize, &info, {} )
            or SM_FAIL( "::SystemParametersInfo failed" );
        return info.lfMessageFont;
    }
    
    inline auto ui_font_spec()
        -> const LOGFONT&
    {
        static const LOGFONT the_spec = get_ui_font_spec();
        return the_spec;
    }

    inline auto ui_font()
        -> HFONT
    {
        static const HFONT the_font = ::CreateFontIndirect( &ui_font_spec() );
        hopefully( the_font != 0 )
            or SM_FAIL( "::CreateFontIndirect failed" );
        return the_font;
    }

    class Name
    {
        variant<C_string_ptr, ATOM>     m_value;

    public:
        Name( const C_string_ptr s ): m_value( s ) {}
        Name( const ATOM id ): m_value( id ) {}

        auto as_pointer() const
            -> const char*              // Note: can be a pseudo-pointer.
        {
            switch( m_value.index() ) {
                case 0:     return get<0>( m_value );
                case 1:     return as_pseudo_ptr( get<1>( m_value ) );
            }
            for( ;; ) {}                // Can't get here.
        }

        auto str() const
            -> string
        {
            switch( m_value.index() ) {
                case 0:     return get<0>( m_value );
                case 1:     return atom_id_string( get<1>( m_value ) );
            }
            for( ;; ) {}                // Can't get here.
        }
    };

    struct Window_class_params: WNDCLASS
    {
        static const int    id_window_color = COLOR_WINDOW;
        static const int    id_dialog_color = COLOR_BTNFACE;

        explicit Window_class_params( const int color_id = id_dialog_color ):
            WNDCLASS()
        {
            style           = CS_DBLCLKS;
            lpfnWndProc     = ::DefWindowProc;
            hInstance       = h_instance;
            hCursor         = std_cursor();
            hbrBackground   = std_pseudobrush( color_id );
            lpszClassName   = "A General Window Class";
        }
        
        static auto window_colored() -> Window_class_params { return Window_class_params( id_window_color ); }
        static auto dialog_colored() -> Window_class_params { return Window_class_params( id_dialog_color ); }
    };

    inline namespace option_parameter_types {
        struct With_title           : Option_<C_string_ptr>{};
        struct With_text            : Option_<C_string_ptr>{};
        struct With_styles          : Option_<DWORD>{};
        struct With_position        : Option_<Point>{};
        struct With_rect_size       : Option_<Rect_size>{};
        struct With_parent          : Option_<HWND>{};
        struct With_menu            : Option_<HMENU>{};
        struct With_id              : Option_<int>{};
        struct With_icon            : Option_<HICON>{};
        struct With_custom_param    : Option_<void*>{};
    }  // namespace option_parameter_types

    using Common_window_option_types = Type_list_<
        With_styles,
        With_position,
        With_rect_size,
        With_custom_param
    >;
    
    using Toplevel_window_option_types  = Common_window_option_types::Plus_<
        With_title, With_parent, With_icon, With_menu
    >;
    using Toplevel_window_options       = Toplevel_window_option_types::Specialization_of_<Option_refs_>;

    template< class... Options >
    inline auto new_toplevel_window( const Name class_id, in_<Options>... options )
        -> HWND
    {
        const Toplevel_window_options option_refs( options... );
        const DWORD     styles      = option_refs | With_styles{ WS_OVERLAPPEDWINDOW };
        const Point     position    = option_refs | With_position{ CW_USEDEFAULT, CW_USEDEFAULT };
        const Rect_size rect_size   = option_refs | With_rect_size{ 640, 400 };

        static_assert( WS_OVERLAPPED == 0 );        // I.e. this style is the default.
        assert( (styles & WS_CHILD) == 0 );

        const HWND window = ::CreateWindow(
            class_id.as_pointer(),                                                  // lpClassName,
            option_refs | With_title{ "<unspecified title>" },                      // lpWindowName
            styles,                                                                 // dwStyle
            position.x, position.y,                                                 // x, y
            rect_size.cx, rect_size.cy,                                             // w, h
            option_refs | With_parent{},                                            // hWndParent
            option_refs | With_menu{},                                              // hMenu
            h_instance,                                                             // hInstance
            option_refs | With_custom_param{}                                       // lpParam
            );
        hopefully( window != 0 ) or SM_FAIL( "::CreateWindow failed." );

        set_font_of( window, ui_font() );
        if( const HICON icon = option_refs | With_icon{} ) {
            set_icon_of( window, icon );
        }
        return window;
    }

    using Child_window_option_types     = Common_window_option_types::Plus_< With_text, With_id >;
    using Child_window_options          = Child_window_option_types::Specialization_of_<Option_refs_>;

    template< class... Options >
    inline auto new_child_window_of( const HWND parent, const Name class_id, in_<Options>... options )
        -> HWND
    {
        const Child_window_options option_refs( options... );
        const DWORD     styles      = (option_refs | With_styles{ WS_CHILD | WS_VISIBLE }) | WS_CHILD;
        const Point     position    = option_refs | With_position{ CW_USEDEFAULT, CW_USEDEFAULT };
        const Rect_size rect_size   = option_refs | With_rect_size{ 64, 40 };

        static_assert( WS_OVERLAPPED == 0 );        // I.e. this style is the default.
        assert( (styles & WS_POPUP) == 0 );
        assert( (styles & WS_CHILD) != 0 );

        const HWND window = ::CreateWindow(
            class_id.as_pointer(),                                                  // lpClassName,
            option_refs | With_text{ "" },                                          // lpWindowName
            styles,                                                                 // dwStyle
            position.x, position.y,                                                 // x, y
            rect_size.cx, rect_size.cy,                                             // w, h
            parent,                                                                 // hWndParent
            reinterpret_cast<HMENU>( (option_refs | With_id{}).value ),             // Id
            h_instance,                                                             // hInstance
            option_refs | With_custom_param{}                                       // lpParam
            );
        hopefully( window != 0 ) or SM_FAIL( "::CreateWindow failed." );
        set_font_of( window, ui_font() );
        return window;
    }

    inline auto styles_of( const HWND window )
        -> DWORD
    { return ::GetWindowLongPtr( window, GWL_STYLE ); }
    
    inline auto title_of( const HWND window )
        -> string
    {
        if( (styles_of( window ) & WS_CAPTION) == 0 ) {
            return "";
        }
        const int buffer_size = 1 + ::GetWindowTextLength( window );
        auto result = string( buffer_size, '\0' );
        const int string_length = ::GetWindowText( window, result.data(), result.size() );
        result.resize( string_length );
        return result;
    }

    inline auto rect_of( const HWND window )
        -> Rect
    {
        Rect r;
        ::GetWindowRect( window, &r ) or SM_FAIL( "GetWindowRect failed." );
        return r;
    }

    inline auto get_screen_size()
        -> Rect_size
    { return size_of( rect_of( ::GetDesktopWindow() ) ); }

    inline auto exe_name()
        -> string
    {
        auto result = string( MAX_PATH + 1, '\0' );
        const auto string_length = static_cast<int>( ::GetModuleFileName( 0, result.data(), result.size() ) );
        result.resize( string_length );
        return result;
    }

    void move_window_near_position( const HWND window, in_<Point> pos )
    {
        const Rect          wr                  = rect_of( window );
        const Rect_size     wr_size             = size_of( wr );
        const Rect_size     down_right_move     = {16, 16};
        const Point         down_right_pos      = pos + down_right_move;
        const Point         down_right_beyond   = down_right_pos + wr_size;
        const Rect_size     up_left_move        = {8, 8};
        const Point         up_left_pos         = pos - up_left_move - wr_size;
        const Rect_size     screen_size         = get_screen_size();

        const bool          use_move_right      = (down_right_beyond.x <= screen_size.cx);
        const bool          use_move_down       = (down_right_beyond.y <= screen_size.cy);

        const auto new_pos = Point
        {
            (use_move_right?    down_right_pos.x : up_left_pos.x),
            (use_move_down?     down_right_pos.y : up_left_pos.y)
        };
        ::SetWindowPos(
            window, {},
            new_pos.x, new_pos.y, {}, {},
            SWP_NOOWNERZORDER | SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER      // TODO: SWP_NOCOPYBITS
            )
            or SM_FAIL( "SetWindowPos failed" );
    }

    void move_window_near_mouse( const HWND window )
    {
        Point mouse_position;
        ::GetCursorPos( &mouse_position ) or SM_FAIL( "GetCursorPos failed" );
        move_window_near_position( window, mouse_position );
    }

    inline auto message_box(
        const HWND              parent,
        const C_string_ptr      text,
        const DWORD             flags   = MB_ICONINFORMATION | MB_SETFOREGROUND
        ) -> int
    {
        struct Cbt_hook
        {
            HHOOK   handle;

            static auto is_active() -> bool&
            {
                static bool the_is_active_flag;
                return the_is_active_flag;
            }

            static auto CALLBACK proc( const int code, const WPARAM w_param, const LPARAM ell_param ) -> LRESULT
            {
                if( is_active() and code == HCBT_ACTIVATE ) {
                    is_active() = false;    // Just prevent more processing, b/c difficult to unhook here.
                    ref_<const CBTACTIVATESTRUCT> info = *reinterpret_cast<const CBTACTIVATESTRUCT*>( ell_param );
                    try {
                        move_window_near_mouse( info.hWndActive );
                    } catch( in_<exception> x ) {
                        fprintf( stderr, "!%s [ignored].\n", x.what() );
                    }
                    
                }
                return ::CallNextHookEx( {}, code, w_param, ell_param );
            }

            ~Cbt_hook() { ::UnhookWindowsHookEx( handle ); }
        
            Cbt_hook():
                handle( ::SetWindowsHookEx( WH_CBT, &proc, nullptr, ::GetCurrentThreadId() ) )
            {
                hopefully( handle != 0 ) or SM_FAIL( "SetWindowsHook failed." );
            }
        };

        SM_WITH( Cbt_hook() ) {
            const string title = (parent? title_of( parent ) : exe_name());
            Cbt_hook::is_active() = true;
            return ::MessageBox( parent, text, title.c_str(), flags | (parent? 0 : MB_TASKMODAL) );
        }
    }

    inline void dispatch_messages()
    {
        for( ;; ) {
            MSG msg = {};
            if( ::GetMessage( &msg, 0, 0, 0 ) < 0 ) {
                SM_FAIL( "::GetMessage failed" );
            } else if( msg.message == WM_QUIT ) {
                hopefully( msg.wParam == 0 )
                    or SM_FAIL( "Something failed, error code " + to_string( msg.wParam ) + "." );
                return;
            }
            ::TranslateMessage( &msg );
            ::DispatchMessage( &msg );
            rethrow_popped_exception();     // If any.
        }
    }
}  // namespace winapi
