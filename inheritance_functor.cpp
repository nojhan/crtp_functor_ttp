#include <iostream>
#include <vector>
#include <iterator>

#define ITEMS(c) c.begin(),c.end()

template<class T>
class Increment
{
    public:
    T arg;
    Increment( T a ) : arg(a) {}

    template<typename OutIt>
    T operator() ( T& value, OutIt out )
    {
        value+=arg;
        *out++ = value;
        return 1;
    }
};

template<class OP, typename OutIt>
class Functor
{
    public:
    OP & op;
    Functor( OP & o ) : op(o) {}

    // we cannot put the OutIt template here, because the functor is virtual
    virtual int operator() ( int& v, OutIt out ) = 0;
};


/* It is painfull to declare a Functor's derivate class with all the templates
 * because C++ can't infer templates from parameters of the constructor.
 * Thus, we rely on this helper free function, where templates can be infered.
 *
 * Instead of having to write:
 *      Assign<Increment<int>, std::back_insert_iterator<std::vector<int> > > f(add_one);
 * We can just write:
 *      auto f = make_Functor<Assign>( add_one, std::back_inserter(v));
 */
template<
    template< class, typename > class FC,
    class OP,
    typename OutIt
>
FC<OP,OutIt>& make_Functor( OP & op, OutIt /*out*/ )
{
    return *(new FC<OP,OutIt>(op));
}


template<class OP, typename OutIt>
class Assign : public Functor<OP, OutIt>
{
    public:
    Assign( OP & o ) : Functor<OP, OutIt>(o) {}

    virtual int operator() ( int& value, OutIt out )
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

        // we specify the output iterator here, just for type inference
        auto f = make_Functor<Assign>(add_one, out);

        int i = -1;
        size_t n = 0;
        for( unsigned int j=0; j<nb; ++j ) {
            n += f(i, out);
        }

        // std::cout << v.size() << std::endl;
        // std::copy( ITEMS(v), std::ostream_iterator<int>( std::cout, "\n") );
    }
}

