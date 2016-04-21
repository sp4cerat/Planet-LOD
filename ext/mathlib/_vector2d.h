#ifndef _vector2DD_H
#define _vector2DD_H
//------------------------------------------------------------------------------
/**
    @class _vector2d
    @ingroup Math

    Generic vector2 class.

    (C) 2002 RadonLabs GmbH
*/
#include "mathlib/nmath.h"
#include <float.h>

//------------------------------------------------------------------------------
class _vector2d {
public:
    /// constructor 1
    _vector2d();
    /// constructor 2
    _vector2d(const double _x, const double _y);
    /// constructor 3
    _vector2d(const _vector2d& vec);
    /// constructor 4
    _vector2d(const double* p);
    /// set elements 1
    void set(const double _x, const double _y);
    /// set elements 2 
    void set(const _vector2d& vec);
    /// set elements 3
    void set(const double* p);
    /// return length
    double len() const;
    /// normalize
    void norm();
    /// in place add
    void operator+=(const _vector2d& v0);
    /// in place sub
    void operator-=(const _vector2d& v0);
    /// in place scalar mul
    void operator*=(const double s);
    /// in place scalar div
    void operator/=(const double s);
    /// fuzzy compare operator
    bool isequal(const _vector2d& v, const double tol) const;
    /// fuzzy compare, returns -1, 0, +1
    int compare(const _vector2d& v, double tol) const;
    /// rotate around P(0,0)
    void rotate(double angle);

    double x, y;
};

//------------------------------------------------------------------------------
/**
*/
inline
_vector2d::_vector2d() :
    x(0.0f),
    y(0.0f)
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
inline
_vector2d::_vector2d(const double _x, const double _y) :
    x(_x),
    y(_y)
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
inline
_vector2d::_vector2d(const _vector2d& vec) :
    x(vec.x),
    y(vec.y)
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
inline
_vector2d::_vector2d(const double* p) :
    x(p[0]),
    y(p[1])
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
inline
void
_vector2d::set(const double _x, const double _y)
{
    x = _x;
    y = _y;
}

//------------------------------------------------------------------------------
/**
*/
inline
void
_vector2d::set(const _vector2d& v)
{
    x = v.x;
    y = v.y;
}

//------------------------------------------------------------------------------
/**
*/
inline
void
_vector2d::set(const double* p)
{
    x = p[0];
    y = p[1];
}

//------------------------------------------------------------------------------
/**
*/
inline
double
_vector2d::len() const
{
    return (double) sqrt(x * x + y * y);
}

//------------------------------------------------------------------------------
/**
*/
inline
void
_vector2d::norm()
{
    double l = len();
    if (l > TINY)
    {
        x /= l;
        y /= l;
    }
}

//------------------------------------------------------------------------------
/**
*/
inline
void
_vector2d::operator +=(const _vector2d& v0) 
{
    x += v0.x;
    y += v0.y;
}

//------------------------------------------------------------------------------
/**
*/
inline
void
_vector2d::operator -=(const _vector2d& v0) 
{
    x -= v0.x;
    y -= v0.y;
}

//------------------------------------------------------------------------------
/**
*/
inline
void
_vector2d::operator *=(const double s) 
{
    x *= s;
    y *= s;
}

//------------------------------------------------------------------------------
/**
*/
inline
void
_vector2d::operator /=(const double s) 
{
    x /= s;
    y /= s;
}

//------------------------------------------------------------------------------
/**
*/
inline
bool 
_vector2d::isequal(const _vector2d& v, const double tol) const
{
    if (fabs(v.x - x) > tol)      return false;
    else if (fabs(v.y - y) > tol) return false;
    return true;
}

//------------------------------------------------------------------------------
/**
*/
inline
int
_vector2d::compare(const _vector2d& v, double tol) const
{
    if (fabs(v.x - x) > tol)      return (v.x > x) ? +1 : -1; 
    else if (fabs(v.y - y) > tol) return (v.y > y) ? +1 : -1;
    else                          return 0;
}

//------------------------------------------------------------------------------
/**
*/
inline
void
_vector2d::rotate(double angle)
{
    // rotates this one around P(0,0).
    double sa, ca;

    sa = (double) sin(angle);
    ca = (double) cos(angle);

    // "handmade" multiplication
    _vector2d help(ca * this->x - sa * this->y,
                  sa * this->x + ca * this->y);  

    *this = help;
}

//------------------------------------------------------------------------------
/**
*/
static 
inline 
_vector2d operator +(const _vector2d& v0, const _vector2d& v1) 
{
    return _vector2d(v0.x + v1.x, v0.y + v1.y); 
}

//------------------------------------------------------------------------------
/**
*/
static 
inline 
_vector2d operator -(const _vector2d& v0, const _vector2d& v1) 
{
    return _vector2d(v0.x - v1.x, v0.y - v1.y);
}

//------------------------------------------------------------------------------
/**
*/
static 
inline 
_vector2d operator *(const _vector2d& v0, const double s) 
{
    return _vector2d(v0.x * s, v0.y * s);
}

//------------------------------------------------------------------------------
/**
*/
static 
inline 
_vector2d operator -(const _vector2d& v) 
{
    return _vector2d(-v.x, -v.y);
}

//------------------------------------------------------------------------------
#endif

