    template< class Handle, void(*destroy)(Handle) >
    class Unique_handle_
    {
        Handle      m_handle;
        
    public:
        ~Unique_handle_() { destroy( m_handle ); }
        Unique_handle_( const Handle handle ): m_handle( handle ) {}
        
        auto value() const -> Handle { return m_handle; }
        operator Handle() const -> Handle { return value(); }
    };
