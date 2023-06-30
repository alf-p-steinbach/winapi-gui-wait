#pragma once    // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").

#include <exception>            // std::(exception_ptr, 
#include <stdexcept>
#include <functional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include <microlib/support-machinery/basic-types.hpp>       // C_string_ptr, Index, as_signed
#include <microlib/support-machinery/type-builders.hpp>     // in_

#include <assert.h>         // assert
#include <stdlib.h>         // EXIT_FAILURE as a generic well known failure code.

// Can't test directly for existence of the pseudo-macros, so must test compiler ids:
#if     defined( _MSC_VER )      // Visual C++
#   define SM_FUNC_DECL                                 __FUNCSIG__
#elif   defined( __GNUC__ ) or defined( __clang__ )
#   define SM_FUNC_DECL                                 __PRETTY_FUNCTION__
#else
#   define SM_FUNC_DECL                                 __func__
#endif

#define SM_FUNC_ID          support_machinery::func_id_from( SM_FUNC_DECL )
#define SM_FAIL_( X, ... )  support_machinery::fail_<X>( std::string( SM_FUNC_ID ) + " - " + (__VA_ARGS__) )
#define SM_FAIL( ... )      SM_FAIL_( std::runtime_error, __VA_ARGS__ )
    
namespace support_machinery {
    using   std::current_exception, std::exception_ptr, std::rethrow_exception, // <exception>
            std::reference_wrapper, std::function,                              // <functional>
            std::exception, std::runtime_error,                                 // <stdexcept>
            std::string,
            std::string_view,
            std::is_same_v,                                                     // <type_traits>
            std::exchange, std::forward, std::move, std::enable_if_t,           // <utility>
            std::vector;

    namespace impl {
        // A max 1 item stack of exception pointers.
        inline auto ref_deferred_exception()
            -> ref_<exception_ptr>
        {
            thread_local exception_ptr the_deferred_x;
            return the_deferred_x;
        }
    }  // namespace impl
    
    struct Bool_with_error_code
    {
        int     error_code = 0;

        Bool_with_error_code( const bool success ):
            error_code( success? 0 : EXIT_FAILURE )
        {}
        
        template< class Integer, bool enabled_ = enable_if_t<is_same_v<Integer, int>, bool>() >
        Bool_with_error_code( const Integer code ):
            error_code( code )
        {}

        auto is_success() const -> bool { return (error_code == 0); }
        operator bool() const { return is_success(); }
    };

    inline auto n_pushed_exceptions()
        -> int
    { return !!impl::ref_deferred_exception(); }

    inline auto popped_exception()
        -> exception_ptr
    { return exchange( impl::ref_deferred_exception(), nullptr ); }

    inline void pop_exception() { (void) popped_exception(); }

    inline auto rethrow_popped_exception()
        -> bool     // Mostly for convenience in invoking this in an expression.
    {
        const exception_ptr xp = popped_exception();
        if( !!xp ) { rethrow_exception( xp ); }
        return false;
    }

    inline auto push_current_exception()
        -> Bool_with_error_code
    {
        auto& dx = impl::ref_deferred_exception();
        if( !!dx ) { return {1}; }
        dx = current_exception();
        if( not dx ) { return {2}; }
        return true;
    }

    constexpr auto func_id_from( in_<string_view> func_decl )
        -> string_view
    {
        const auto i_parens = as_signed( func_decl.find( '(' ) );
        if( i_parens < 0 ) {
            return func_decl;       // Probably a `__func__` as argument.
        }
        const auto i_space = as_signed( func_decl.rfind( ' ', i_parens ) );
        const auto i_first = (i_space < 0? 0 : i_space + 1);
        return func_decl.substr( i_first, i_parens - i_first );
    }

    template< class X, class... Args >
    [[noreturn]] inline auto fail_( Args&&... args )
        -> bool
    {
        assert( n_pushed_exceptions() == 0 );
        auto x = X( forward<Args>( args )... );
        if( current_exception() ) {
            throw_with_nested( move( x ) );
        }
        throw move( x );
    }

    [[noreturn]] inline auto fail( in_<string> message )
        -> bool
    { fail_<runtime_error>( message ); }

    constexpr auto hopefully( const bool condition ) -> bool { return condition; }

    inline void rethrow_any_nested_x_of( in_<exception> x )
    {
        std::rethrow_if_nested( x );
    }

    inline auto with_messages_of( in_<exception> x, function<void(C_string_ptr)> process )
        -> bool
    {
        // Uses inefficient nested exception throwing in order to avoid stack overflow on recursion.
        exception_ptr px;
        for( ;; ) {
            try {
                if( not px ) {
                    process( x.what() );
                    rethrow_any_nested_x_of( x );
                } else {
                    try{
                        rethrow_exception( px );
                    } catch( in_<exception> current_x ) {
                        process( current_x.what() );
                        rethrow_any_nested_x_of( current_x );
                    }
                    return false;   // Non-standard exception type.
                }
                return true;    // No more nested exceptions.
            } catch( ... ) {
                px = current_exception();
            }
        }
        for( ;; ){}         // Can't get here.
    }

    inline auto messages_of( in_<exception> x )
    {
        vector<string>  messages;
        with_messages_of( x, [&]( C_string_ptr s ){ messages.push_back( s ); } );
        return messages;
    }
}  // namespace support_machinery
