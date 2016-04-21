#ifndef N_VECTOR_H
#define N_VECTOR_H
//------------------------------------------------------------------------------
/**
    Implement 2, 3 and 4-dimensional vector classes.

    (C) 2002 RadonLabs GmbH
*/
#include "ntypes.h"
#define vec2f vector2
#define vec2d vector2d
#define vec3f vector3
#define vec3d vector3d
#define vec4f vector4
#ifndef __USE_SSE__
// generic vector classes
#include "mathlib/_vector2.h"
#include "mathlib/_vector2d.h"
#include "mathlib/_vector3.h"
#include "mathlib/_vector3d.h"
#include "mathlib/_vector4.h"
typedef _vector2 vector2;
typedef _vector2d vector2d;
typedef _vector3 vector3;
typedef _vector3d vector3d;
typedef _vector4 vector4;
#else
// sse vector classes
#include "mathlib/_vector2.h"
#include "mathlib/_vector3_sse.h"
#include "mathlib/_vector4_sse.h"
typedef _vector2 vector2; 
typedef _vector3_sse vector3;
typedef _vector4_sse vector4;
#endif

//------------------------------------------------------------------------------
#endif
