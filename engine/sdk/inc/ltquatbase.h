#ifndef __LTQUATBASE_H__
#define __LTQUATBASE_H__

#ifndef __MATH_H__
#include <math.h>
#define __MATH_H__
#endif


#ifndef DOXYGEN_SHOULD_SKIP_THIS

#undef HALFPI       // Max defines HALFPI as well, so we need to clean out the macro space
#define HALFPI              1.570796326794895
#define QUAT_TOLERANCE      0.00001f
#define QUAT_HIGH_TOLERANCE (1.0f - QUAT_TOLERANCE)
#define QUAT_LOW_TOLERANCE  (-QUAT_HIGH_TOLERANCE)

#define QUAT_X (0)
#define QUAT_Y (1)
#define QUAT_Z (2)
#define QUAT_W (3)


void quat_ConvertToMatrix
(
    const float*    q,
    float           M[4][4]
);

void quat_GetVectors
(
    const float*    q,
    float*          R0,
    float*          R1,
    float*          R2
);

void quat_ConvertFromMatrix
(
    float*      q,
    const float M[4][4]
);

void quat_Slerp
(
    float*          qu,
    const float*    q0,
    const float*    q1,
    float           u
);

inline void quat_Identity
(
    float*  q
)
{
    q[0] = q[1] = q[2] = 0;
    q[3] = 1;
}

inline void quat_Set
(
    float*  q,
    float   x,
    float   y,
    float   z,
    float   r
)
{
    q[0] = x;
    q[1] = y;
    q[2] = z;
    q[3] = r;
}

/*
\note This is exactly the same as converting both quaternions to
matrices, multiplying the matrices (in the same order, a*b), then
converting back to a quaternion (except it's \em faster and more
accurate).

\par This can be simplified when the quaternions are written as a
scalar and vector:

- s1 = a[QUAT_W]
- v1 = (a[QUAT_X], a[QUAT_Y], a[QUAT_Z])
- s2 = b[QUAT_W]
- v2 = (b[QUAT_X], b[QUAT_Y], b[QUAT_Z])
- a = [s1,v1]
- b = [s2,v2]

The quaternion product is then [s1*s2 - v1*v2, s1*v2 + s2*v1 + v1.Cross(v2)]
*/
inline void quat_Mul
(
    float*          q,
    const float*    a,
    const float*    b
)
{
    q[QUAT_X] = a[QUAT_W]*b[QUAT_X] + a[QUAT_X]*b[QUAT_W] + a[QUAT_Y]*b[QUAT_Z] - a[QUAT_Z]*b[QUAT_Y];
    q[QUAT_Y] = a[QUAT_W]*b[QUAT_Y] - a[QUAT_X]*b[QUAT_Z] + a[QUAT_Y]*b[QUAT_W] + a[QUAT_Z]*b[QUAT_X];
    q[QUAT_Z] = a[QUAT_W]*b[QUAT_Z] + a[QUAT_X]*b[QUAT_Y] - a[QUAT_Y]*b[QUAT_X] + a[QUAT_Z]*b[QUAT_W];
    q[QUAT_W] = a[QUAT_W]*b[QUAT_W] - a[QUAT_X]*b[QUAT_X] - a[QUAT_Y]*b[QUAT_Y] - a[QUAT_Z]*b[QUAT_Z];
}

inline void quat_Conjugate
(
    float*          c,
    const float*    q
)
{
    c[QUAT_X] = -q[QUAT_X];
    c[QUAT_Y] = -q[QUAT_Y];
    c[QUAT_Z] = -q[QUAT_Z];
    c[QUAT_W] = q[QUAT_W];
}

// ------------------------------------------------------------------------
// quat_RotVec( result-vec, quaternion, input-vector )
//
// evaluate v' = qvq-1
// q being the quat, q-1 its conjugate, and v a vector.
// we assume that the quaternion is a unit quaternion.
// this is faster and more specific than doing:
//	conv_to_quat( v, point );
//	quat_Mul( v', q, v );
//	quat_Conjugate( c, q ); 
//	quat_Mul( v'', v', c ); 
//
// ( i've left some of the operations commented out to indicate reasoning. )
// ------------------------------------------------------------------------
__inline
void quat_RotVec( float *pnt_res, const float *quat, const float *pnt_in )
{
	float  cj[4];
	float  r1[4] ;
			
	float *v = r1; // intermediate result
	const float *a = quat; // rotation
	const float *b = pnt_in;// source vector

	// evaluate quat * vector
	// bw == 0.0f
    v[QUAT_X] = a[QUAT_W]*b[QUAT_X]  /*+ax *bw */  + a[QUAT_Y]*b[QUAT_Z] - a[QUAT_Z]*b[QUAT_Y];
    v[QUAT_Y] = a[QUAT_W]*b[QUAT_Y] - a[QUAT_X]*b[QUAT_Z] /* +ay * bw*/ + a[QUAT_Z]*b[QUAT_X];
    v[QUAT_Z] = a[QUAT_W]*b[QUAT_Z] + a[QUAT_X]*b[QUAT_Y] - a[QUAT_Y]*b[QUAT_X] /* + az * bw */;
    v[QUAT_W] =/* aw * bw */- a[QUAT_X]*b[QUAT_X] - a[QUAT_Y]*b[QUAT_Y] - a[QUAT_Z]*b[QUAT_Z];

	// make the conjugate of quat.
	cj[QUAT_X] = -a[QUAT_X]; cj[QUAT_Y] = -a[QUAT_Y];  cj[QUAT_Z] = -a[QUAT_Z]; cj[QUAT_W] = a[QUAT_W];

	// reassign
	v = pnt_res ;// final result
	a = r1 ; // result from qv
	b = cj ; // conjuagate

	// evaluate vector * conj
	v[QUAT_X] = a[QUAT_W]*b[QUAT_X] + a[QUAT_X]*b[QUAT_W] + a[QUAT_Y]*b[QUAT_Z] - a[QUAT_Z]*b[QUAT_Y];
    v[QUAT_Y] = a[QUAT_W]*b[QUAT_Y] - a[QUAT_X]*b[QUAT_Z] + a[QUAT_Y]*b[QUAT_W] + a[QUAT_Z]*b[QUAT_X];
    v[QUAT_Z] = a[QUAT_W]*b[QUAT_Z] + a[QUAT_X]*b[QUAT_Y] - a[QUAT_Y]*b[QUAT_X] + a[QUAT_Z]*b[QUAT_W];
    /* v[QUAT_W] = a[QUAT_W]*b[QUAT_W] - a[QUAT_X]*b[QUAT_X] - a[QUAT_Y]*b[QUAT_Y] - a[QUAT_Z]*b[QUAT_Z]; */
}


#endif//doxygen


#endif
//EOF
