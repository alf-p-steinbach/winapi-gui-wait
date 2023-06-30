#pragma once    // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").

// An integer interval with inclusive limits.
// Note 1: a /time/ interval is called a “duration”, e.g. `std::chrono::duration`.
// Note 2: the `zero_to` function can alternatively be expressed via `std::views::iota`.

#include <microlib/support-machinery/type-builders.hpp>

namespace support_machinery{

    template< class Integer >
    struct Interval_
    {
        Integer         first;
        Integer         last;

        constexpr Interval_( const Integer a_first, const Integer a_last ):
            first( a_first ), last( a_last )
        {}

        constexpr auto contains( const Integer v ) const
            -> bool
        { return (first <= v and v <= last); }
            
        constexpr auto length() const -> Integer { return (last - first) + 1; }
        
        class Iterator
        {
            Integer     m_current;
            
        public:
            constexpr Iterator( const Integer first ):
                m_current( first )
            {}
            
            constexpr auto operator*() const -> Integer { return m_current; }
            
            constexpr auto operator++()
                -> Iterator&
            {
                ++m_current;
                return *this;
            }
            
            friend
            constexpr auto operator==( in_<Iterator> a, in_<Iterator> b )
                -> bool
            { return (a.m_current == b.m_current); }

            friend
            constexpr auto operator!=( in_<Iterator> a, in_<Iterator> b )
                -> bool
            { return not( a == b ); }
        };
        
        constexpr auto begin() const    -> Iterator     { return Iterator( first ); }
        constexpr auto end() const      -> Iterator     { return Iterator( last + 1 ); }
    };

    using   Interval    = Interval_<int>;


    //------------------------------- Convenience functions:
    
    template< class Integer >
    constexpr auto is_in( in_<Interval_<Integer>> interval, const Integer value )
        -> bool
    { return interval.contains( value ); }

    template< class Integer >
    constexpr auto zero_to( const Integer beyond )
        -> Interval_<Integer>
    { return Interval_<Integer>( 0, beyond - 1 ); }

    template< class Integer >
    constexpr auto one_through( const Integer last )
        -> Interval_<Integer>
    { return Interval_<Integer>( 1, last ); }

} // namespace support_machinery
