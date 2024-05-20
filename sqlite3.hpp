#ifndef SQLITE3_HPP_INCLUDED
#define SQLITE3_HPP_INCLUDED

#include <memory>
#include <string>
#include <cstdint>
#include <stdexcept>

#include <boost/config.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/utility/string_ref.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/optional/optional.hpp>

#   if defined(BOOST_NO_CXX11_VARIADIC_TEMPLATES)
#   include <boost/preprocessor/repetition/repeat.hpp>
#   include <boost/preprocessor/repetition/enum_params.hpp>
#   include <boost/preprocessor/repetition/enum_binary_params.hpp>
#   endif

#include <sqlite3.h>

namespace sqlite
{
    struct error : std::runtime_error
    {
        explicit error(int ec)
            : runtime_error(sqlite3_errstr(ec))
        {}

        int error_code() const
        {
            return 0;
        }

    private:

        int _error_code;
    };
}

namespace sqlite { namespace detail
{
    struct db_deleter
    {
        void operator()(sqlite3* db)
        {
            sqlite3_close(db);
        }
    };
    struct stmt_deleter
    {
        void operator()(sqlite3_stmt* stmt)
        {
            sqlite3_finalize(stmt);
        }
    };

    inline void throw_on_error(int ec)
    {
        if (ec)
            throw error(ec);
    }
}}

namespace sqlite
{
    struct result
    {
        explicit result(sqlite3_stmt* stmt) : _stmt(stmt) {}

        template<class T>
        T get(int idx) const
        {
            T val;
            get_result(idx, val);
            return val;
        }

    private:

        void get_result(int n, double& t) const
        {
            t = sqlite3_column_double(_stmt, n);
        }

        void get_result(int n, int& t) const
        {
            t = sqlite3_column_int(_stmt, n);
        }

        void get_result(int n, std::int64_t& t) const
        {
            t = sqlite3_column_int64(_stmt, n);
        }

        void get_result(int n, std::string& t) const
        {
            auto data = reinterpret_cast<char const*>(sqlite3_column_text(_stmt, n));
            auto size = sqlite3_column_bytes(_stmt, n);
            t.assign(data, size);
        }

        void get_result(int n, boost::string_ref& t) const
        {
            auto data = reinterpret_cast<char const*>(sqlite3_column_text(_stmt, n));
            auto size = sqlite3_column_bytes(_stmt, n);
            t = boost::string_ref(data, size);
        }

        template<class T>
        void get_result(int n, boost::optional<T>& o) const
        {
            if (sqlite3_column_type(_stmt, n) != SQLITE_NULL)
            {
                T t;
                get_result(n, t);
                o = t;
            }
        }

        sqlite3_stmt* _stmt;
    };

    struct iterator
        : boost::iterator_facade<iterator, result, std::input_iterator_tag, result>
    {
        iterator() : _stmt() {}

        explicit iterator(sqlite3_stmt* stmt) : _stmt(stmt)
        {
            increment();
        }

    private:

        friend class boost::iterator_core_access;

        void increment()
        {
            int ec = sqlite3_step(_stmt);
            if (ec == SQLITE_DONE)
                _stmt = nullptr;
            else if (ec != SQLITE_ROW)
                throw error(ec);
        }

        bool equal(iterator const& other) const
        {
            return _stmt == other._stmt;
        }

        result dereference() const
        {
            return result(_stmt);
        }

        sqlite3_stmt* _stmt;
    };

    struct statement
    {
        statement() {}

        explicit statement(sqlite3_stmt* stmt) : _handle(stmt) {}

#   if defined(BOOST_NO_CXX11_DEFAULTED_FUNCTIONS)
        statement(statement&& other) : _handle(std::move(other._handle)) {}

        statement& operator=(statement other)
        {
            this->~statement();
            return *new(this) statement(std::move(other));
        }
#   endif

        bool valid() const
        {
            return static_cast<bool>(_handle);
        }

#   if defined(BOOST_NO_CXX11_VARIADIC_TEMPLATES)
        boost::iterator_range<iterator> query() const
        {
            auto stmt = _handle.get();
            sqlite3_reset(stmt);
            return boost::iterator_range<iterator>(iterator(stmt), iterator());
        }

        void operator()() const
        {
            auto stmt = _handle.get();
            sqlite3_reset(stmt);
            int ec = sqlite3_step(stmt);
            if (ec != SQLITE_DONE)
                throw error(ec);
        }
#   define BIND_PARAM_(z, n, data) bind_param(n + 1, std::forward<BOOST_PP_CAT(T, n)>(BOOST_PP_CAT(t, n)));
#   define QUERY_(n)                                                            \
        template<BOOST_PP_ENUM_PARAMS(n, class T)>                              \
        boost::iterator_range<iterator> query(BOOST_PP_ENUM_BINARY_PARAMS(n, T, &&t)) const\
        {                                                                       \
            auto stmt = _handle.get();                                          \
            sqlite3_reset(stmt);                                                \
            BOOST_PP_REPEAT(n, BIND_PARAM_, ~)                                  \
            return boost::iterator_range<iterator>(iterator(stmt), iterator()); \
        }                                                                       \
/***/
#   define OPERATOR_CALL_(n)                                                    \
        template<BOOST_PP_ENUM_PARAMS(n, class T)>                              \
        void operator()(BOOST_PP_ENUM_BINARY_PARAMS(n, T, &&t)) const           \
        {                                                                       \
            auto stmt = _handle.get();                                          \
            sqlite3_reset(stmt);                                                \
            BOOST_PP_REPEAT(n, BIND_PARAM_, ~)                                  \
            int ec = sqlite3_step(stmt);                                        \
            if (ec != SQLITE_DONE)                                              \
                throw error(ec);                                                \
        }                                                                       \
/***/
        QUERY_(1)
        QUERY_(2)
        QUERY_(3)
        QUERY_(4)
        QUERY_(5)
        OPERATOR_CALL_(1)
        OPERATOR_CALL_(2)
        OPERATOR_CALL_(3)
        OPERATOR_CALL_(4)
        OPERATOR_CALL_(5)
#   undef BIND_PARAM_
#   undef BIND_PARAM_
#   undef OPERATOR_CALL_
#   else
        template<class... T>
        boost::iterator_range<iterator> query(T&&... t) const
        {
            auto stmt = _handle.get();
            sqlite3_reset(stmt);
            bind_impl(std::make_integer_sequence<int, sizeof...(T)>{}, std::forward<T>(t)...);
            return boost::iterator_range<iterator>(iterator(stmt), iterator());
        }

        template<class... T>
        void operator()(T&&... t) const
        {
            auto stmt = _handle.get();
            sqlite3_reset(stmt);
            bind_impl(std::make_integer_sequence<int, sizeof...(T)>{}, std::forward<T>(t)...);
            int ec = sqlite3_step(stmt);
            if (ec != SQLITE_DONE)
                throw error(ec);
        }
#   endif

        template<class T>
        void bind(char const* name, T&& t) const
        {
            bind_param(sqlite3_bind_parameter_index(_handle.get(), name), std::forward<T>(t));
        }

    private:

#   if !defined(BOOST_NO_CXX11_VARIADIC_TEMPLATES)
        template<int... N, class... T>
        void bind_impl(std::integer_sequence<int, N...>, T&&... t) const
        {
            std::initializer_list<bool>
            {
                (bind_param(N + 1, std::forward<T>(t)), true)...
            };
        }
#   endif

        void bind_param(int n, double t) const
        {
            detail::throw_on_error(sqlite3_bind_double(_handle.get(), n, t));
        }

        void bind_param(int n, int t) const
        {
            detail::throw_on_error(sqlite3_bind_int(_handle.get(), n, t));
        }

        void bind_param(int n, std::int64_t t) const
        {
            detail::throw_on_error(sqlite3_bind_int64(_handle.get(), n, t));
        }

        void bind_param(int n, boost::string_ref const& t) const
        {
            detail::throw_on_error(sqlite3_bind_text(_handle.get(), n, t.data(), int(t.size()), SQLITE_TRANSIENT));
        }

        void bind_param(int n, std::nullptr_t) const
        {
            detail::throw_on_error(sqlite3_bind_null(_handle.get(), n));
        }

        template<class T>
        void bind_param(int n, boost::optional<T> const& o) const
        {
            if (o)
                bind_param(n, o.get());
            else
                bind_param(n, nullptr);
        }

        typedef std::unique_ptr<sqlite3_stmt, detail::stmt_deleter> handle_t;

        handle_t _handle;
    };

    struct database
    {
        database() {}

        explicit database(char const* filename)
            : _handle(make_handle(filename))
        {}

        database(char const* filename, int flags, char const* vfs = nullptr)
            : _handle(make_handle(filename, flags, vfs))
        {}

#   if defined(BOOST_NO_CXX11_DEFAULTED_FUNCTIONS)
        database(database&& other) : _handle(std::move(other._handle)) {}

        database& operator=(database other)
        {
            this->~database();
            return *new(this) database(std::move(other));
        }
#   endif

        void open(char const* filename)
        {
            _handle = make_handle(filename);
        }

        void open(char const* filename, int flags, char const* vfs = nullptr)
        {
            _handle = make_handle(filename, flags, vfs);
        }

        void close()
        {
            _handle.reset();
        }

        bool valid() const
        {
            return static_cast<bool>(_handle);
        }

        void exec(char const* stmts) const
        {
            detail::throw_on_error(sqlite3_exec(_handle.get(), stmts, nullptr, nullptr, nullptr));
        }

        statement prepare(boost::string_ref const& stmt) const
        {
            sqlite3_stmt* p;
            detail::throw_on_error(sqlite3_prepare_v2(_handle.get(), stmt.data(), int(stmt.size()), &p, nullptr));
            return statement(p);
        }

    private:

        typedef std::unique_ptr<sqlite3, detail::db_deleter> handle_t;

        static handle_t make_handle(char const* filename)
        {
            sqlite3* db;
            detail::throw_on_error(sqlite3_open(filename, &db));
            return handle_t(db);
        }

        static handle_t make_handle(char const* filename, int flags, char const* vfs = nullptr)
        {
            sqlite3* db;
            detail::throw_on_error(sqlite3_open_v2(filename, &db, flags, vfs));
            return handle_t(db);
        }

        handle_t _handle;
    };
}

#endif
