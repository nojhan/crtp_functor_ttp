#include <iostream>
#include <vector>
#include <iterator>

#define ITEMS(c) c.begin(),c.end()

// remains inchanged
template<class T>
class Increment
{
    public:
    T arg;
    Increment( T a ) : arg(a) {}

    template<typename OutIt>
    T operator() ( T & value, OutIt out )
    {
        value+=arg;
        *out++ = value;
        return 1;
    }
};

/* We want to be able to use the functor interface as a templated class.
 * But, it takes the derived class as a template parameter,
 * along with the type of an operator class, which is templated:
 * both of which are templated themselves.
 *
 * We want to avoid having to use declarations of the form:
 *      Functor<Assign<Increment<T>,T>,Increment<T>,T>
 * Thus the complicated template of template (of template) structures below, which permits to write:
 *      Functor<Assign,Increment,T>
 *
 * Below, "typename" is used where a standard type (being fundamentals or in the STL) is expected,
 * and "class" where a class from the framework is expected.
 */
template<
    // The templates structure of the derived class
    template<
        // An operator, i.e. a class with a template <=> a template of template
        template<typename> class,
        // A type
        typename
    // used as a type called CRTP
    > class CRTP,

    // The template structure of the operator <=> a template of template called OP
    template<typename> class OP,

    // the simple value type
    typename T
>
class Functor
{
    public:
    OP<T> & op;
    Functor( OP<T> & o ) : op(o) {}

    // the function is not virtual, we can put a template on it
    template<typename OutIt>
    int operator() ( int& v, OutIt out )
    {
        return static_cast<CRTP<OP,T>*>(this)->functor(v,out);
    }
};

/*
 * Instead of having to write:
 *      Assign<Increment,int> f(op);
 * We can just write:
 *      auto f = make<Assign>(op);
 */
template<
    template< template<typename> class, typename > class FC,
    template <typename> class OP,
    typename T
>
FC<OP,T>& make_Functor( OP<T> & op )
{
    return *(new FC<OP,T>(op));
}

// Inherits from Functor via the CRTP
template<template <typename> class OP, class T>
class Assign : public Functor<Assign,OP,T>
{
    public:
    Assign( OP<T> & o ) : Functor<Assign,OP,T>(o) {}

    template<typename OutIt>
    inline int functor( int& value, OutIt out )
    {
        // std::clog << value << " + " << this->op.arg << std::endl;
        return this->op(value,out);
    }
};


int main(void)
{
    size_t nb = 10000;

    for( unsigned int k=0; k<nb; ++k) {
        std::vector<int> v;
        auto out = std::back_inserter(v);

        Increment<int> add_one(1);

        auto f = make_Functor<Assign>(add_one);

        int i = -1;
        size_t n = 0;
        for( unsigned int j=0; j<nb; ++j ) {
            n += f(i, out);
        }

        // std::cout << v.size() << std::endl;
        // std::copy( ITEMS(v), std::ostream_iterator<int>( std::cout, "\n") );
    }
}

