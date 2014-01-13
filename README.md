Functor pattern in C++: CRTP VS Inheritance
===========================================

Some examples to compare the performances of several implementations of the
Functor pattern:

* Inheritance (the canonical way),
* Curiously Recurring Template Pattern (CRTP).


Abstract
--------

There is several way of implementing the functors in C++. This set of code
demonstrate two of them: an inheritance pattern and the CRTP.

With this very rough test, the CRTP implementation seems to be the fastest. On
my machine, CRTP is approximately 1.3 times faster than Inheritance.


Introduction
------------

### Functor's definition

The *Functor pattern* seems to have many possible definitions. Here, I will
consider the simpler one: a class type functor. This is basically an object that
is callable: a function object.

```C++
/* For example, this simple class uses the functor pattern to attach a state to
a function. */
struct A:
{
    int _state;
    A ( int state ) : _state(state) {}
    int operator() ( int arg ) const {
        return _state + arg;
    }
};
```

### Composition of function objects

I really like this functor which is a good compromise between composition and
inheritance on one side, and object-oriented and functional programming on the
other side.

I use it to assemble algorithms as a set of functions applied to a state. This
is a pattern heavily used within the [ParadisEO framework](http://paradiseo.gforge.inria.fr),
for instance.

The targeted use case needs several function objects (called operators) to build
an algorithm (which is itself an operator), but here I will do the tests with a
single operator that embed an operator itself.


```C++
/* An example of the general form of an operator and its usage */
struct Algo
{
    OperatorInit _opi;
    OperatorIter _opt;
    Algo( OperatorInit & opi, OperatorIter opt ) : _opi(opi), _opt(opt) {}
    int operator() ( int & state ) {
        _opi(state);
        for( int i=-1; i<9; ++i ) {
            _opt(state);
        }
        return i;
    }
};

int main(  )
{
    // Operators instanciation
    OperatorInit init;
    // They may take arguments
    OperatorIter iter(1);
    Algo algo( init, iter );
    int state = 0;
    std::cout << algo( state ) << std::endl;
}
```

### Interfaces

Of course, to permits the user of a framework to add new operators, we can use
interfaces as a way to force him to avoid mistakes.

```C++
/* With an inheritance pattern, we can use abstract classes as interfaces */
struct Operator:
{
    virtual void operator() ( int & arg ) = 0;
};

struct OperatorInit : public Operator
{
    virtual void operator() ( int & arg ) { arg = 0; }
};
```

### Generic programming

As operators are used deep inside the call tree, they may need different
interfaces at each level, and not always the one defined by the framework.
For example, if there is interaction between user-defined operators somewhere
within a framework-defined one (remember operators are composed with operators).

```C++
/* Thus, we need templates, most probably for the manipulated state */
template< class T>
struct OperatorIter
{
    // for example because this particular operator needs an access to "length" */
    int operator() ( T & state ) { state++; return state.length(); }
};
```


Putting it all together
-----------------------

For the comparison, I will use here a simple example that will increment a
counter with the help of two functors. One is a generic `Assign` function that
is supposed to call an `Increment` function that alter the given state and push
it in a container. The other is the function that actually do the job.

For simplicity, only the first one will use an interface called `Functor`.

Keep in mind that this is just a simplified example that implements a hierarchy
of operators, it is not supposed to be elegant.

```C++
/* A simplified view of the logical articulation of the classes */
template<class T>
struc Increment
{
    template<class OutIt>
    T operator() ( T & arg, OutIt out )
    {
        // do something with the `arg` and push it to `out`
    }
    /* ... */
};

template<class T>
struct Functor
{
    Increment op;
    // the interface
    template<class OutIt>
    int operator() ( int & arg, OutIt out ) /* ... */
};

template<class T>
struct Assign : public Functor<T>
{
    // Assemble operator[s]
    Assign( Increment & op ) : Functor(op) {}
    // This kind of interface
    int operator() ( int & arg, OutIt out )
    {
        return this->op(arg,out);
    }
    /* ... */
};
```


Note about the implented versions
---------------------------------

The implementations are located in the following files:
* Inheritance version: `inheritance_functor.cpp`
* CRTP version: `crtp_functor_ttp.cpp`

Notes about the code are written as comments in those files.

### Inheritance code

This version use overloading of the `Assign::operator()` function, over an abstract
base class that provide the interface.

In the `Increment` functor, the output iterator template is defined over the
call function, which permits to ease the declaration of the functor. But in the
`Assign` class, as we want to overload a virtual function, it is not possible to
define such a template, it should be defined over the class itself.

This is a limitation of this pattern, because I would have wanted to let the
compiler infer the output iterator type at call and thus avoid the need to
specify it at the instanciation.

```C++
    // we cannot do that:
    Assign<Increment> f(op);

    // we must explictly declare the output iterator type at instanciation
    Assign<Increment,std::back_insert_iterator<std::vector<int> > > f(op);

    // or use a helper that does not have a virtual function
    auto f = mafe_Functor<Assign>(op,out);
```


### Curiously Recurring Template Pattern

Has there is no virtual functions involved in the CRTP, it is possible to
declare the output iterator template over the callable function, and the
compiler can infer its type.

The use of the CRTP makes hard to design more than one level of "inheritance"
for the operator classes, and the template declarations are a pain.


Performance test
----------------

The performance test is done with two nested loops, the outer one instanciating
the functors and the inner one calling it several times. Both are iterating
10 000 times.

To compile and run the performance tests, use the `./test.sh` script.

```
$ ./test.sh
inheritance_functor

real	0m0.350s
user	0m0.344s
sys	0m0.004s

crtp_functor_ttp

real	0m0.272s
user	0m0.268s
sys	0m0.000s
```

