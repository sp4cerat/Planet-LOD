#ifndef _vector3dD_H
#define _vector3dD_H
//------------------------------------------------------------------------------
/**
    @class _vector3d
    @ingroup Math

    Generic vector3 class. Uses 16 Byte of mem instead of 12 (!)

    (C) 2002 RadonLabs GmbH
*/
#include "mathlib/nmath.h"
#include <float.h>

//------------------------------------------------------------------------------
class _vector3d
{
public:
    /// constructor 1
    _vector3d();
    /// constructor 2
    _vector3d(const double _x, const double _y, const double _z);
    /// constructor 3
    _vector3d(const _vector3d& vec);
    /// set elements 1
    void set(const double _x, const double _y, const double _z);
    /// set elements 2 
    void set(const _vector3d& vec);
    /// return length
    double len() const;
	double length(){return len();}
	/// return cross
	_vector3d cross( const _vector3d& a , const _vector3d& b ) ;
    /// return length squared
    double lensquared() const;
    /// normalize
    _vector3d norm(double length=1.0);
	_vector3d normalize(){return norm();};
    /// inplace add
    void operator +=(const _vector3d& v0);
    /// inplace sub
    void operator -=(const _vector3d& v0);
    /// inplace scalar multiplication
    void operator *=(double s);
    /// true if any of the elements are greater
    bool operator >(const _vector3d rhs) const;
    /// true if any of the elements are smaller
    bool operator <(const _vector3d rhs) const;
    /// fuzzy compare, return true/false
    bool isequal(const _vector3d& v, double tol) const;
    /// fuzzy compare, returns -1, 0, +1
    int compare(const _vector3d& v, double tol) const;
    /// rotate around axis
    void rotate(const _vector3d& axis, double angle);
    /// inplace linear interpolation
    void lerp(const _vector3d& v0, double lerpVal);
    /// returns a vector orthogonal to self, not normalized
    _vector3d findortho() const;
    /// saturate components between 0 and 1
    void saturate();
    /// dot product
    double dot(const _vector3d& v0) const;
    /// distance between 2 vector3's
    static double distance(const _vector3d& v0, const _vector3d& v1);
	/// min
	void minimum(const _vector3d& v)
	{
		x=min(x,v.x);y=min(y,v.y);z=min(z,v.z);
	}
	/// max
	void maximum(const _vector3d& v)
	{
		x=max(x,v.x);y=max(y,v.y);z=max(z,v.z);
	}
	double angle(const _vector3d& v)
	{
		return acos(dot(v)/(v.len()*len()));
	}

	/// encode_normal sphere
	void to_angle_x_y()
	{
		double a_y=atan2(x,z);
		double a_x=acos(dot(_vector3d(0,1,0)));
		x=a_x-M_PI/2;
		y=a_y-M_PI/2;
		z=0;
	}
	/// encode_normal sphere
	void from_angle_x_y()
	{
		double ry=y+M_PI/2, rx=x+M_PI/2;
		x=sin(ry)*sin(rx);
		z=cos(ry)*sin(rx);
		y=cos(rx);
	}

	/// encode_normal sphere
	unsigned int encode_normal_sphere()
	{
		double a_y=atan2(x,z)*127.0f/M_PI+128.0f;
		double a_x=acos(dot(_vector3d(0,1,0)))*255.0/M_PI;
		//printf("ay %2.2f \n",double(atan2(x,z)));
		return int(a_y)+(int(a_x)<<8);
	}
	/// dencode_normal sphere
	void decode_normal_sphere(int pack)
	{
		double ry=double(int(pack&255)-128)*M_PI/127.0f;pack>>=8;
		double rx=double(int(pack&255))*M_PI/255.0f;
		//printf("ry %2.2f \n",ry);

		x=sin(ry)*sin(rx);
		z=cos(ry)*sin(rx);
		y=cos(rx);
	}
	/// encode_normal
	unsigned int encode_normal_uint()
	{
		int ix=x*127+128.5;
		int iy=y*127+128.5;
		int iz=z*127+128.5;
		return ix+(iy<<8)+(iz<<16);
	}
	struct char_uchar
	{
		union {char c; uchar uc;};
	};
	/// encode_normal
	unsigned int encode_normal_signed_uint()
	{
		char_uchar ix;ix.c=x*127;
		char_uchar iy;iy.c=y*127;
		char_uchar iz;iz.c=z*127;
		return ix.uc+(iy.uc<<8)+(iz.uc<<16);
	}
	/// encode_normal
	unsigned int encode_color_uint()
	{
		int ix=x*255+0.5;
		int iy=y*255+0.5;
		int iz=z*255+0.5;
		return ix+(iy<<8)+(iz<<16);
	}
	/// decode_normal
	void decode_normal_uint(int n)
	{
		x=(n&255)-128;n>>=8;
		y=(n&255)-128;n>>=8;
		z=(n&255)-128;
		norm();
	}
	/// decode_color
	void decode_color_uint(unsigned int c)
	{
		x=c&255;c>>=8;
		y=c&255;c>>=8;
		z=c&255;
		x/=255.0;
		y/=255.0;
		z/=255.0;
	}

	double& operator[] (int i){ return (&this->x)[i];}

	double x, y, z;
};

//------------------------------------------------------------------------------
/**
*/
inline
_vector3d::_vector3d() :
    x(0.0f),
    y(0.0f),
    z(0.0f)
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
inline
_vector3d::_vector3d(const double _x, const double _y, const double _z) :
    x(_x),
    y(_y),
    z(_z)
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
inline
_vector3d::_vector3d(const _vector3d& vec) :
    x(vec.x),
    y(vec.y),
    z(vec.z)
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
inline
void
_vector3d::set(const double _x, const double _y, const double _z)
{
    x = _x;
    y = _y;
    z = _z;
}

//------------------------------------------------------------------------------
/**
*/
inline
void
_vector3d::set(const _vector3d& vec)
{
    x = vec.x;
    y = vec.y;
    z = vec.z;
}

//------------------------------------------------------------------------------
/**
*/
inline
_vector3d
_vector3d::cross( const _vector3d& a , const _vector3d& b ) 
    { 
		x = a.y * b.z - a.z * b.y;
		y = a.z * b.x - a.x * b.z;
		z = a.x * b.y - a.y * b.x;
		return *this;
	}

//------------------------------------------------------------------------------
/**
*/
inline
double
_vector3d::len() const
{
    return (double) n_sqrt(x * x + y * y + z * z);
}

//------------------------------------------------------------------------------
/**
*/
inline
double 
_vector3d::lensquared() const
{
    return x * x + y * y + z * z;
}

//------------------------------------------------------------------------------
/**
*/
inline
_vector3d
_vector3d::norm(double length)
{
    double l = len()/length;
    //if (l > TINY) 
    {
        x /= l;
        y /= l;
        z /= l;
    }
	return *this;
}

//------------------------------------------------------------------------------
/**
*/
inline
void
_vector3d::operator +=(const _vector3d& v0)
{
    x += v0.x;
    y += v0.y;
    z += v0.z;
}

//------------------------------------------------------------------------------
/**
*/
inline
void
_vector3d::operator -=(const _vector3d& v0)
{
    x -= v0.x;
    y -= v0.y;
    z -= v0.z;
}

//------------------------------------------------------------------------------
/**
*/
inline
void
_vector3d::operator *=(double s)
{
    x *= s;
    y *= s;
    z *= s;
}

//------------------------------------------------------------------------------
/**
*/
inline
bool
_vector3d::isequal(const _vector3d& v, double tol) const
{
    if (fabs(v.x - x) > tol)      return false;
    else if (fabs(v.y - y) > tol) return false;
    else if (fabs(v.z - z) > tol) return false;
    return true;
}

//------------------------------------------------------------------------------
/**
*/
inline
int
_vector3d::compare(const _vector3d& v, double tol) const
{
    if (fabs(v.x - x) > tol)      return (v.x > x) ? +1 : -1; 
    else if (fabs(v.y - y) > tol) return (v.y > y) ? +1 : -1;
    else if (fabs(v.z - z) > tol) return (v.z > z) ? +1 : -1;
    else                          return 0;
}

//------------------------------------------------------------------------------
/**
*/
inline
void
_vector3d::rotate(const _vector3d& axis, double angle)
{
    // rotates this one around given vector. We do
    // rotation with matrices, but these aren't defined yet!
    double rotM[9];
    double sa, ca;

    sa = (double) sin(angle);
    ca = (double) cos(angle);

    // build a rotation matrix
    rotM[0] = ca + (1 - ca) * axis.x * axis.x;
    rotM[1] = (1 - ca) * axis.x * axis.y - sa * axis.z;
    rotM[2] = (1 - ca) * axis.z * axis.x + sa * axis.y;
    rotM[3] = (1 - ca) * axis.x * axis.y + sa * axis.z;
    rotM[4] = ca + (1 - ca) * axis.y * axis.y;
    rotM[5] = (1 - ca) * axis.y * axis.z - sa * axis.x;
    rotM[6] = (1 - ca) * axis.z * axis.x - sa * axis.y;
    rotM[7] = (1 - ca) * axis.y * axis.z + sa * axis.x;
    rotM[8] = ca + (1 - ca) * axis.z * axis.z;

    // "handmade" multiplication
    _vector3d help(rotM[0] * this->x + rotM[1] * this->y + rotM[2] * this->z,
                  rotM[3] * this->x + rotM[4] * this->y + rotM[5] * this->z,
                  rotM[6] * this->x + rotM[7] * this->y + rotM[8] * this->z);
    *this = help;
}

//------------------------------------------------------------------------------
/**
*/
static 
inline 
_vector3d operator +(const _vector3d& v0, const _vector3d& v1) 
{
    return _vector3d(v0.x + v1.x, v0.y + v1.y, v0.z + v1.z);
}

//------------------------------------------------------------------------------
/**
*/
static 
inline 
_vector3d operator -(const _vector3d& v0, const _vector3d& v1) 
{
    return _vector3d(v0.x - v1.x, v0.y - v1.y, v0.z - v1.z);
}

//------------------------------------------------------------------------------
/**
*/
static 
inline 
_vector3d operator *(const _vector3d& v0, const double s) 
{
    return _vector3d(v0.x * s, v0.y * s, v0.z * s);
}

//------------------------------------------------------------------------------
/**
*/
static 
inline 
_vector3d operator -(const _vector3d& v) 
{
    return _vector3d(-v.x, -v.y, -v.z);
}

//------------------------------------------------------------------------------
/**
*/
static 
inline 
_vector3d operator /(const _vector3d& v0, const double s)
{
    double one_over_s = 1.0f/s;
    return _vector3d(v0.x*one_over_s, v0.y*one_over_s, v0.z*one_over_s);
}

//------------------------------------------------------------------------------
/**
    Dot product.
*/
static
inline
double operator %(const _vector3d& v0, const _vector3d& v1)
{
    return v0.x * v1.x + v0.y * v1.y + v0.z * v1.z;
}

//------------------------------------------------------------------------------
/**
    multiply
*/
static 
inline 
_vector3d operator *(const _vector3d& v0, const _vector3d& v1) 
{
//    return _vector3d(v0.y * v1.z - v0.z * v1.y,
 //                   v0.z * v1.x - v0.x * v1.z,
   //                 v0.x * v1.y - v0.y * v1.x);
    return _vector3d(v0.x*v1.x,v0.y*v1.y,v0.z*v1.z);
}

//------------------------------------------------------------------------------
/**
*/
inline
void
_vector3d::lerp(const _vector3d& v0, double lerpVal)
{
    x = v0.x + ((x - v0.x) * lerpVal);
    y = v0.y + ((y - v0.y) * lerpVal);
    z = v0.z + ((z - v0.z) * lerpVal);
}

//------------------------------------------------------------------------------
/**
*/
inline
void
_vector3d::saturate()
{
    x = n_saturate(x);
    y = n_saturate(y);
    z = n_saturate(z);
}

//------------------------------------------------------------------------------
/**
    Find a vector that is orthogonal to self. Self should not be (0,0,0).
    Return value is not normalized.
*/
inline
_vector3d
_vector3d::findortho() const
{
    if (0.0 != x)
    {
        return _vector3d((-y - z) / x, 1.0, 1.0);
    } else
    if (0.0 != y)
    {
        return _vector3d(1.0, (-x - z) / y, 1.0);
    } else
    if (0.0 != z)
    {
        return _vector3d(1.0, 1.0, (-x - y) / z);
    } else
    {
        return _vector3d(0.0, 0.0, 0.0);
    }
}

//------------------------------------------------------------------------------
/**
    Dot product for vector3
*/
inline
double
_vector3d::dot(const _vector3d& v0) const
{
    return ( x * v0.x + y * v0.y + z * v0.z );
}

//------------------------------------------------------------------------------
/**
*/
inline
bool
_vector3d::operator >(const _vector3d rhs) const
{
	if(fabs(x-rhs.x)>0.0001){if(x>rhs.x)return true;else return false;}
	if(fabs(y-rhs.y)>0.0001){if(y>rhs.y)return true;else return false;}
	if(fabs(z-rhs.z)>0.0001){if(z>rhs.z)return true;else return false;}
	return false;

	if(x+y+z>rhs.x+rhs.y+rhs.z) return true;else return false;

    if ((this->x > rhs.x) || (this->y > rhs.y) || (this->z > rhs.z))
    {
        return true;
    }
    else
    {
        return false;
    }
}

//------------------------------------------------------------------------------
/**
*/
inline
bool
_vector3d::operator <(const _vector3d rhs) const
{
	if(fabs(x-rhs.x)>0.0001){if(x<rhs.x)return true;else return false;}
	if(fabs(y-rhs.y)>0.0001){if(y<rhs.y)return true;else return false;}
	if(fabs(z-rhs.z)>0.0001){if(z<rhs.z)return true;else return false;}
	return false;

	if(x+y+z<rhs.x+rhs.y+rhs.z) return true;else return false;
		/*
    if ((this->x < rhs.x) || (this->y < rhs.y) || (this->z < rhs.z))
    {
        return true;
    }
    else
    {
        return false;
    }*/
}

//------------------------------------------------------------------------------
/**
*/
inline
double
_vector3d::distance(const _vector3d& v0, const _vector3d& v1)
{
    _vector3d v(v1 - v0);
    return (double) n_sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

//------------------------------------------------------------------------------
#endif
