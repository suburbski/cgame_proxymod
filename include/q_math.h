#ifndef Q_MATH_H
#define Q_MATH_H

#include "cg_local.h"

#include <math.h>

/* This file was automatically generated.  Do not edit! */
#define NUMVERTEXNORMALS 162
#define DEG2RAD(a) (((a)*M_PI) / 180.0F)
#define RAD2DEG(a) (((a)*180.0f) / M_PI)
extern vec3_t vec3_origin;
int           Q_log2(int val);
void          Vector4Scale(const vec4_t in, vec_t scale, vec4_t out);
void          _VectorScale(const vec3_t in, vec_t scale, vec3_t out);
void          _VectorCopy(const vec3_t in, vec3_t out);
void          _VectorAdd(const vec3_t veca, const vec3_t vecb, vec3_t out);
void          _VectorSubtract(const vec3_t veca, const vec3_t vecb, vec3_t out);
vec_t         _DotProduct(const vec3_t v1, const vec3_t v2);
void          _VectorMA(const vec3_t veca, float scale, const vec3_t vecb, vec3_t vecc);
vec_t         VectorNormalize2(const vec3_t v, vec3_t out);
void          AddPointToBounds(const vec3_t v, vec3_t mins, vec3_t maxs);
void          ClearBounds(vec3_t mins, vec3_t maxs);
float         RadiusFromBounds(const vec3_t mins, const vec3_t maxs);
#if defined __LCC__ || defined C_ONLY || !id386 ||                                                                     \
  defined __VECTORC && !((defined __linux__ || __FreeBSD__) && (defined __i386__) && (!defined C_ONLY)) // rb010123
int       BoxOnPlaneSide(vec3_t emins, vec3_t emaxs, struct cplane_s* p);
#endif
#if !(defined __LCC__ || defined C_ONLY || !id386 || defined __VECTORC) &&                                             \
  !((defined __linux__ || __FreeBSD__) && (defined __i386__) && (!defined C_ONLY)) // rb010123
__declspec(naked) int BoxOnPlaneSide(vec3_t emins, vec3_t emaxs, struct cplane_s* p);
#endif
void  SetPlaneSignbits(cplane_t* out);
float AngleDelta(float angle1, float angle2);
float AngleNormalize180(float angle);
float AngleNormalize360(float angle);
float AngleMod(float a);
void  AnglesSubtract(vec3_t v1, vec3_t v2, vec3_t v3);
float AngleSubtract(float a1, float a2);
float LerpAngle(float from, float to, float frac);
void  VectorRotate(vec3_t in, vec3_t matrix[3], vec3_t out);
void  MakeNormalVectors(const vec3_t forward, vec3_t right, vec3_t up);
#if !idppc
float Q_fabs(float f);
#endif
void         ProjectPointOnPlane(vec3_t dst, const vec3_t p, const vec3_t normal);
void         AxisCopy(vec3_t in[3], vec3_t out[3]);
void         AxisClear(vec3_t axis[3]);
void         AngleVectors(const vec3_t angles, vec3_t forward, vec3_t right, vec3_t up);
void         AnglesToAxis(const vec3_t angles, vec3_t axis[3]);
void         vectoangles(const vec3_t value1, vec3_t angles);
void         RotateAroundDirection(vec3_t axis[3], float yaw);
void         MatrixMultiply(float in1[3][3], float in2[3][3], float out[3][3]);
void         PerpendicularVector(vec3_t dst, const vec3_t src);
void         RotatePointAroundVector(vec3_t dst, const vec3_t dir, const vec3_t point, float degrees);
vec_t        VectorNormalize(vec3_t v);
qboolean     PlaneFromPoints(vec4_t plane, const vec3_t a, const vec3_t b, const vec3_t c);
float        NormalizeColor(const vec3_t in, vec3_t out);
unsigned     ColorBytes4(float r, float g, float b, float a);
unsigned     ColorBytes3(float r, float g, float b);
void         ByteToDir(int b, vec3_t dir);
int          DirToByte(vec3_t dir);
signed short ClampShort(int i);
signed char  ClampChar(int i);
void         CrossProduct(const vec3_t v1, const vec3_t v2, vec3_t cross);
void         VectorInverse(vec3_t v);
#if !idppc
float Q_rsqrt(float number);
#endif
void  VectorNormalizeFast(vec3_t v);
vec_t DistanceSquared(const vec3_t p1, const vec3_t p2);
vec_t Distance(const vec3_t p1, const vec3_t p2);
vec_t VectorLengthSquared(const vec3_t v);
vec_t VectorLength(const vec3_t v);
int   VectorCompare(const vec3_t v1, const vec3_t v2);

#endif // Q_MATH_H
