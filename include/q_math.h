#ifndef Q_MATH_H
#define Q_MATH_H

#include "cg_local.h"

#include <math.h>

/* This file was automatically generated.  Do not edit! */
int         Q_log2(int val);
void        Vector4Scale(vec4_t const in, vec_t scale, vec4_t out);
void        _VectorScale(vec3_t const in, vec_t scale, vec3_t out);
void        _VectorCopy(vec3_t const in, vec3_t out);
void        _VectorAdd(vec3_t const veca, vec3_t const vecb, vec3_t out);
void        _VectorSubtract(vec3_t const veca, vec3_t const vecb, vec3_t out);
vec_t       _DotProduct(vec3_t const v1, vec3_t const v2);
void        _VectorMA(vec3_t const veca, float scale, vec3_t const vecb, vec3_t vecc);
vec_t       VectorNormalize2(vec3_t const v, vec3_t out);
void        AddPointToBounds(vec3_t const v, vec3_t mins, vec3_t maxs);
void        ClearBounds(vec3_t mins, vec3_t maxs);
float       RadiusFromBounds(vec3_t const mins, vec3_t const maxs);
#if defined __LCC__ || defined C_ONLY || !id386 ||                                                                     \
  defined __VECTORC && !((defined __linux__ || __FreeBSD__) && (defined __i386__) && (!defined C_ONLY)) // rb010123
int       BoxOnPlaneSide(vec3_t const emins, vec3_t const emaxs, struct cplane_s* p);
#endif
#if !(defined __LCC__ || defined C_ONLY || !id386 || defined __VECTORC) &&                                             \
  !((defined __linux__ || __FreeBSD__) && (defined __i386__) && (!defined C_ONLY)) // rb010123
__declspec(naked) int BoxOnPlaneSide(vec3_t const emins, vec3_t const emaxs, struct cplane_s* p);
#endif
void  SetPlaneSignbits(cplane_t* out);
float AngleDelta(float angle1, float angle2);
float AngleNormalize180(float angle);
float AngleNormalize360(float angle);
float AngleMod(float a);
void  AnglesSubtract(vec3_t const v1, vec3_t const v2, vec3_t v3);
float AngleSubtract(float a1, float a2);
float LerpAngle(float from, float to, float frac);
void  VectorRotate(vec3_t const in, vec3_t matrix[3], vec3_t out);
void  MakeNormalVectors(vec3_t const forward, vec3_t right, vec3_t up);
#if !idppc
float Q_fabs(float f);
#endif
void         ProjectPointOnPlane(vec3_t dst, vec3_t const p, vec3_t const normal);
void         AxisCopy(vec3_t in[3], vec3_t out[3]);
void         AxisClear(vec3_t axis[3]);
void         AngleVectors(vec3_t const angles, vec3_t forward, vec3_t right, vec3_t up);
void         AnglesToAxis(vec3_t const angles, vec3_t axis[3]);
void         vectoangles(vec3_t const value1, vec3_t angles);
void         RotateAroundDirection(vec3_t axis[3], float yaw);
void         MatrixMultiply(float in1[3][3], float in2[3][3], float out[3][3]);
void         PerpendicularVector(vec3_t dst, vec3_t const src);
void         RotatePointAroundVector(vec3_t dst, vec3_t const dir, vec3_t const point, float degrees);
vec_t        VectorNormalize(vec3_t v);
qboolean     PlaneFromPoints(vec4_t plane, vec3_t const a, vec3_t const b, vec3_t const c);
float        NormalizeColor(vec3_t const in, vec3_t out);
unsigned     ColorBytes4(float r, float g, float b, float a);
unsigned     ColorBytes3(float r, float g, float b);
void         ByteToDir(int b, vec3_t dir);
int          DirToByte(vec3_t const dir);
signed short ClampShort(int i);
signed char  ClampChar(int i);
void         CrossProduct(vec3_t const v1, vec3_t const v2, vec3_t cross);
void         VectorInverse(vec3_t v);
#if !idppc
float Q_rsqrt(float number);
#endif
void  VectorNormalizeFast(vec3_t v);
vec_t DistanceSquared(vec3_t const p1, vec3_t const p2);
vec_t Distance(vec3_t const p1, vec3_t const p2);
vec_t VectorLengthSquared(vec3_t const v);
vec_t VectorLength(vec3_t const v);
int   VectorCompare(vec3_t const v1, vec3_t const v2);

#endif // Q_MATH_H
