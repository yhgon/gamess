#ifndef _RYSQ_CORE_HPP_
#define _RYSQ_CORE_HPP_

#include <vector>
#include <utility>
#include <boost/array.hpp>
#include <boost/utility.hpp>
#include <iostream>
#include <stdio.h>

#include "rysq/config.hpp"

static const int RYSQ_NORMALIZE = 1;

namespace rysq {

    static const double SQRT_4PI5 = 34.986836655249725;

    enum type { SP = -1, S, P, D, F, G, H, I };

    static const type types[] = { SP, S, P, D, F };

    extern const int LX[];
    extern const int LY[];
    extern const int LZ[];
    extern const double NORMALIZE[];


    typedef boost::array<double,3> Center;
    typedef boost::array<int,4> Int4;


    struct shell {
	struct const_iterator {
	    typedef std::forward_iterator_tag iterator_category;
	    typedef int value_type;
	    typedef int reference;
	    typedef int difference_type;
	    typedef const int* pointer;
	    explicit const_iterator(int value = 0) : value(value) {}
	    const const_iterator& operator++() const { ++ value; return *this ; }
	    reference operator[](size_t i) const { return value + i; }
	    reference operator*() const { return value; }
	    operator int() const { return value ; }
	    // operator size_t() const { return size_t(value); }
	    mutable int value;
	};
	const rysq::type type;
	const int L;
	const int index;
	const int size;
	const int nc;
	const int K;
	// shell() : type(rysq::SP), L(0), index(0), size(0), nc(0), K(0) {}
	shell(int type, int K = 0)
	    : type(rysq:: type(type)),
	      L(std::abs(type)), 
	      index(begin(type)),
	      size(end(type) - index),
	      nc(1 + (type < 0)*L),
	      K(K)
	{}
	operator rysq::type() const { return type; }
	bool is_hybrid() const { return nc > 1; }
	const_iterator begin() const { return const_iterator(index); }
	const_iterator end() const { return const_iterator(index + size); }
    private:
	static int begin(int a) { return (a >= 0)*((a*(a+1)*(a+2))/6); }
	static int end(int a) { return begin(std::abs(a)+1); }
    };


    namespace detail {

	template<class F, typename Quartet>
	typename F::value_type add(const Quartet &q) {
	    return (F::value(q[0])+F::value(q[1])+F::value(q[2])+F::value(q[3]));
	}

	template<class F, typename Quartet>
	typename F::value_type multiply(const Quartet &q) {
	    return (F::value(q[0])*F::value(q[1])*F::value(q[2])*F::value(q[3]));
	}

	struct L {
	    typedef size_t value_type;
	    static size_t value(const rysq::type &type) { return shell(type).L; }
	    static size_t value(const rysq::shell &shell) { return shell.L; }
	    template<typename Quartet>
	    static value_type add(const Quartet &q) {
		return detail::add<L>(q);
	    }
	};

	struct size {
	    typedef size_t value_type;
	    static size_t value(const rysq::type &type) { return shell(type).size; }
	    static size_t value(const rysq::shell &shell) { return shell.size; }
	    template<typename Quartet>
	    static value_type multiply(const Quartet &q) {
		return detail::multiply<size>(q);
	    }
	};

	struct K {
	    typedef size_t value_type;
	    static size_t value(const rysq::shell &shell) { return shell.K; }
	    template<typename Quartet>
	    static value_type multiply(const Quartet &q) {
		return detail::multiply<K>(q);
	    }
	};

    }


    class Shell : public shell {
    public:
	Shell(rysq::type type, int K, const double *a, const double* C[])
	    : shell(type, K)
	{
	    initialize(a, C);
	}
	Shell(const Shell &shell) : rysq::shell(shell) {
	    initialize(shell.a_, shell.C_);
	}
	~Shell() { if (this->data_) delete[] this->data_; }
	double& operator()(size_t i) const { return a_[i]; }
	double& operator()(size_t i, size_t k) const { return C_[k][i]; }
	const char* symbol() const { return symbol_; }
    private:
	char symbol_[4];
	double *a_, *C_[2];
	double *data_;
	void initialize(const double *a, const double* const C[]);
	void operator=(const Shell&);
    };
    inline std::ostream& operator<<(std::ostream &os, const Shell &shell) {
      return os << shell.symbol();
    }

    template<typename C, typename T>
    boost::array<C,4> make_array(const T &t0, const T &t1, const T &t2, const T &t3) {
	boost::array<C,4> q = {{ C(t0), C(t1), C(t2), C(t3) }};
	return q;
    }


    struct State {
	State(const Shell &A, const Shell &B) : A(A), B(B) {}
	size_t K() const { return A.K*B.K; }
	size_t L() const { return A.L + B.L; }
	size_t size() const {return A.size*B.size; }
	size_t hybrid() const { return  A.is_hybrid() + B.is_hybrid(); }
	const Shell& operator[](bool i) const { return ((i == 0) ? A : B); }
    private:
	const Shell &A, &B;
    };

    template<class  C>
    struct quartet_base : boost::array<C,4> {
	typedef boost::array<C,4> base;
	typedef C c_type[4];
	quartet_base() {}
	template<typename T>
	quartet_base(const T &t0, const T &t1, const T &t2, const T &t3)
	    : base(make(t0, t1, t2, t3)) {}
	template<typename T>
	quartet_base(const boost::array<T,4> &q)
	    : base(make(q[0], q[1], q[2], q[3])) {}
	operator const c_type&() const { return this->elems; }
    protected:
	template<typename T>
	static base make(const T &t0, const T &t1, const T &t2, const T &t3) {
	    boost::array<C,4> array = {{ t0, t1, t2, t3 }};
	    return array;
	}
	size_t size() const;// { return base::size(); }
    };

    template<class C>
    struct Quartet : quartet_base<C> {
	typedef quartet_base<C> base;
	Quartet() {}
	template<typename T>
	Quartet(const T &t0, const T &t1, const T &t2, const T &t3)
	    : base(t0, t1, t2, t3) {}
	template<typename T>
	Quartet(const T (&q)[4]) : base(q[0], q[1], q[2], q[3]) {}
	template<typename T>
   	Quartet(const boost::array<T,4> &q) : base(q[0], q[1], q[2], q[3]) {}
    };

    template<class C>
    inline std::ostream& operator<<(std::ostream &os, const Quartet<C> &quartet) {
      return os << "{"
		<< quartet[0] << "," << quartet[1] << ","
		<< quartet[2] << "," << quartet[3]
		<< "}";
    }


    template<>
    struct Quartet<rysq::type> : quartet_base<rysq::type> {
	typedef quartet_base<rysq::type> base;
	Quartet(const type &a, const type &b, const type &c, const type &d)
	    : base(a,b,c,d) {}
	template<typename T>
   	Quartet(const boost::array<T,4> &quartet) : base(quartet) {}
	size_t L() const { return detail::L::add(*this); }
	size_t size() const { return detail::size::multiply(*this); }
    };

    template<>
    struct Quartet<rysq::shell> : quartet_base<rysq::shell> {
	typedef quartet_base<rysq::shell> base;
	Quartet(const shell &a, const shell &b, const shell &c, const shell &d)
	    : base(a,b,c,d) {}
	template<typename T>
   	Quartet(const boost::array<T,4> &quartet) : base(quartet) {}
	size_t L() const { return detail::L::add(*this); }
	size_t size() const { return detail::size::multiply(*this); }
	size_t K() const { return detail::K::multiply(*this); }
    };

    template<>
    class Quartet<Shell> {
    public:
	typedef State Bra, Ket;

	static int size(int a, int b, int c, int d) {
	    return (shell(a).size*shell(b).size*shell(c).size*shell(d).size);
	}

	static size_t size(const rysq::type (&quartet)[4]) {
	    return size(quartet[0], quartet[1], quartet[2], quartet[3]);
	}

#ifdef __CUDACC__
	__host__ __device__
#endif
	static int symmetry(int i, int j, int k, int l) {
	    return 1 << ((i == j) + (k == l) + ((i == k) && (j == l)));
	}

	template<typename T>
#ifdef __CUDACC__
	__host__ __device__
#endif
	static int symmetry(const T (&quartet)[4]) {
	    return symmetry(quartet[0], quartet[1], quartet[2], quartet[3]);
	}

	Quartet(const Shell &a, const Shell &b, const Shell &c, const Shell &d)
	    : a_(a), b_(b), c_(c), d_(d)
	{
	    initialize();
	}
	Quartet(const Quartet &quartet)
	    : a_(quartet[0]), b_(quartet[1]), c_(quartet[2]), d_(quartet[3])
	{
	    initialize();
	}
	~Quartet() {}
	int L() const { return L_; }
	int size() const { return size_; }
	int size(int i, int j) const { return (*this)[i].size*(*this)[j].size; }
	int hybrid() const { return hybrid_; }
	int nc() const { return nc_; }
	int K() const { return K_; }
	const Shell& operator[](size_t i) const { return *shells_[i]; }
	const Bra bra() const { return Bra((*this)[0], (*this)[1]); }
	const Ket ket() const { return Ket((*this)[2], (*this)[3]); }
	operator Quartet<rysq::type>() const { return cast<rysq::type>(); }
	operator Quartet<rysq::shell>() const  { return cast<rysq::shell>(); }
	boost::array<Shell*,4> data() const { return shells_; }
    private:
	boost::array<Shell*,4> shells_;
	Shell a_, b_, c_, d_;
	int L_, size_, hybrid_, nc_, K_;
	void initialize();
	void operator=(const Quartet&);
	template<typename T>
	Quartet<T> cast() const {
	    const Quartet &q = *this;
	    return Quartet<T>(T(q[0]), T(q[1]), T(q[2]), T(q[3]));
	}
    };

    // template<typename T>
    // std::ostream& operator<<(std::ostream &os, const Quartet<T> &quartet) {
    // 	return (os << "{ " << quartet[0] << quartet[1] << 
    // 		quartet[2] << quartet[3] << "}");
    // }


    struct Transpose {
	enum { BRA = 1, KET = 2, BRAKET = 4 };
	bool bra, ket, braket;
	size_t value, index[4];
	Transpose() : bra(0), ket(0), braket(0), value(0) { initialize(); }
	Transpose(bool bra, bool ket, bool braket)
	    : bra(bra), ket(ket), braket(braket)
	{
	    this->value = ((int(bra) << 0) | (int(ket) << 1) | (int(braket) << 2));
	    using std::swap;
	    initialize();
	    if (bra) swap(index[0], index[1]);
	    if (ket) swap(index[2], index[3]);
	    if (braket) {
		swap(index[0], index[2]);
		swap(index[1], index[3]);
	    }
	}
	template<class Q> Q operator()(const Q &quartet);
    private:
	void initialize() {
	    for (int i = 0; i < 4; ++i) index[i] = i;
	}
    };


    int initialize();

    int finalize();

#ifdef __CUDACC__
#define rysq_kernel_function__ __host__ __device__
#else
#define rysq_kernel_function__
#endif

    struct index_list {
	typedef size_t type[2];
	static const type list_[6];
	typedef type* iterator;
	typedef const type* const_iterator;
	static size_t size() { return 6; }
	static const type& at(size_t i) { return list_[i]; }
	static const_iterator begin() { return list_; }
	static const_iterator end() { return list_ + size(); }

	rysq_kernel_function__
	static inline size_t index1(size_t index){
	    // if ( index == 0)  return 0;
	    // if ( index == 1)  return 2;
	    // return (index > 3);
	    return (index == 1) ? 2 : (index > 3);
	}

	rysq_kernel_function__
	static inline size_t index2(size_t index){
	    // if ( index == 0) return 1;
	    // // if ( index == 5) return 3;
	    // return 2 + index%2;
	    return (index == 0) ? 1 : (2 + index%2);
	}

	rysq_kernel_function__
	static inline size_t index3(size_t I,size_t J) {
	    return ((I > 0) ? (0) :
		    ((J - I > 1) ? (I + 1) : (J + 1)));
	}

	rysq_kernel_function__
	static inline size_t index4(size_t I, size_t J) {
	    return 6 - (I +  J + index3(I, J));
	}

	template<typename T>
	rysq_kernel_function__
	static size_t find(const T (&index)[2]) {
	    return find(index[0], index[1]);
	}

	rysq_kernel_function__
	static size_t find(size_t i,size_t j) {
	    const size_t transpose = 6*(i > j);
	    size_t index;
	    if (i + j == 1) index =  0 + transpose;
	    else if (i + j == 5) index =  1 + transpose;
	    else if (transpose) index =  2*j + i + transpose;
	    else index =  2*i + j;
	    // std::cout << i << " " << j << " " << index << std::endl;
	    return index;
	}
    };

    template<typename T, class L = index_list>
    struct index_set {
	static L indices(){ return L(); }
	rysq_kernel_function__
	T& get(size_t i, size_t j) {
	    return data_[L::find(i,j)];
	}
	rysq_kernel_function__
	const T& get(size_t i, size_t j) const {
	    return data_[L::find(i,j)];
	}
	template<typename U>
	rysq_kernel_function__
	T& get(const U (&index)[2]) {
	    return get(index[0], index[1]);
	}
	T& operator[](size_t i) { return data_[i]; }
	const T& operator[](size_t i) const { return data_[i]; }
    private:
	T data_[12];
    };

}

// #ifdef RYSQ_CUDA
// #include "rysq-cuda.hpp"
// #endif

#endif
