/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* (c) Valeriy Pavlovich Lain.Added some functions and classes for working with matrices.See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef BASE_VMATH_H
#define BASE_VMATH_H

#include <math.h>
#include "math.h"	// mix

// ------------------------------------

template<typename T>
class vector2_base
{
public:
	union { T x,u; };
	union { T y,v; };

	vector2_base() {}
	vector2_base(T nx, T ny)
	{
		x = nx;
		y = ny;
	}

	vector2_base operator -() const { return vector2_base(-x, -y); }
	vector2_base operator -(const vector2_base &v) const { return vector2_base(x-v.x, y-v.y); }
	vector2_base operator +(const vector2_base &v) const { return vector2_base(x+v.x, y+v.y); }
	vector2_base operator *(const T v) const { return vector2_base(x*v, y*v); }
	vector2_base operator *(const vector2_base &v) const { return vector2_base(x*v.x, y*v.y); }
	vector2_base operator /(const T v) const { return vector2_base(x/v, y/v); }
	vector2_base operator /(const vector2_base &v) const { return vector2_base(x/v.x, y/v.y); }

	const vector2_base &operator +=(const vector2_base &v) { x += v.x; y += v.y; return *this; }
	const vector2_base &operator -=(const vector2_base &v) { x -= v.x; y -= v.y; return *this; }
	const vector2_base &operator *=(const T v) { x *= v; y *= v; return *this;	}
	const vector2_base &operator *=(const vector2_base &v) { x *= v.x; y *= v.y; return *this; }
	const vector2_base &operator /=(const T v) { x /= v; y /= v; return *this;	}
	const vector2_base &operator /=(const vector2_base &v) { x /= v.x; y /= v.y; return *this; }

	bool operator ==(const vector2_base &v) const { return x == v.x && y == v.y; } //TODO: do this with an eps instead
	bool operator !=(const vector2_base &v) const { return x != v.x || y != v.y; }

	operator const T* () { return &x; }
};

template<typename T>
inline vector2_base<T> rotate(const vector2_base<T> &a, float angle)
{
	angle = angle * pi / 180.0f;
	float s = sinf(angle);
	float c = cosf(angle);
	return vector2_base<T>((T)(c*a.x - s*a.y), (T)(s*a.x + c*a.y));
}

template<typename T>
inline T distance(const vector2_base<T> &a, const vector2_base<T> &b)
{
	return length(a-b);
}

template<typename T>
inline T dot(const vector2_base<T> &a, const vector2_base<T> &b)
{
	return a.x*b.x + a.y*b.y;
}

template<typename T>
inline vector2_base<T> closest_point_on_line(vector2_base<T> line_point0, vector2_base<T> line_point1, vector2_base<T> target_point)
{
	vector2_base<T> c = target_point - line_point0;
	vector2_base<T> v = (line_point1 - line_point0);
	v = normalize(v);
	T d = length(line_point0-line_point1);
	T t = dot(v, c)/d;
	return mix(line_point0, line_point1, clamp(t, (T)0, (T)1));
}


inline float length(const vector2_base<float> &a)
{
	return sqrtf(a.x*a.x + a.y*a.y);
}

inline float angle(const vector2_base<float> &a)
{
	return atan2f(a.y, a.x);
}

inline vector2_base<float> normalize(const vector2_base<float> &v)
{
	float l = (float)(1.0f/sqrtf(v.x*v.x + v.y*v.y));
	return vector2_base<float>(v.x*l, v.y*l);
}

inline vector2_base<float> direction(float angle)
{
	return vector2_base<float>(cosf(angle), sinf(angle));
}

typedef vector2_base<float> vec2;
typedef vector2_base<bool> bvec2;
typedef vector2_base<int> ivec2;


// ------------------------------------
template<typename T>
class vector3_base
{
public:
	union { T x,r,h; };
	union { T y,g,s; };
	union { T z,b,v,l; };

	vector3_base() {}
	vector3_base(T nx, T ny, T nz)
	{
		x = nx;
		y = ny;
		z = nz;
	}

	vector3_base operator -(const vector3_base &v) const { return vector3_base(x-v.x, y-v.y, z-v.z); }
	vector3_base operator -() const { return vector3_base(-x, -y, -z); }
	vector3_base operator +(const vector3_base &v) const { return vector3_base(x+v.x, y+v.y, z+v.z); }
	vector3_base operator *(const T v) const { return vector3_base(x*v, y*v, z*v); }
	vector3_base operator *(const vector3_base &v) const { return vector3_base(x*v.x, y*v.y, z*v.z); }
	vector3_base operator /(const T v) const { return vector3_base(x/v, y/v, z/v); }
	vector3_base operator /(const vector3_base &v) const { return vector3_base(x/v.x, y/v.y, z/v.z); }

	const vector3_base &operator +=(const vector3_base &v) { x += v.x; y += v.y; z += v.z; return *this; }
	const vector3_base &operator -=(const vector3_base &v) { x -= v.x; y -= v.y; z -= v.z; return *this; }
	const vector3_base &operator *=(const T v) { x *= v; y *= v; z *= v; return *this;	}
	const vector3_base &operator *=(const vector3_base &v) { x *= v.x; y *= v.y; z *= v.z; return *this; }
	const vector3_base &operator /=(const T v) { x /= v; y /= v; z /= v; return *this;	}
	const vector3_base &operator /=(const vector3_base &v) { x /= v.x; y /= v.y; z /= v.z; return *this; }

	bool operator ==(const vector3_base &v) const { return x == v.x && y == v.y && z == v.z; } //TODO: do this with an eps instead
	bool operator !=(const vector3_base &v) const { return x != v.x || y != v.y || z != v.z; }

	operator const T* () { return &x; }
};

template<typename T>
inline T distance(const vector3_base<T> &a, const vector3_base<T> &b)
{
	return length(a-b);
}

template<typename T>
inline T dot(const vector3_base<T> &a, const vector3_base<T> &b)
{
	return a.x*b.x + a.y*b.y + a.z*b.z;
}

template<typename T>
inline vector3_base<T> cross(const vector3_base<T> &a, const vector3_base<T> &b)
{
	return vector3_base<T>(
		a.y*b.z - a.z*b.y,
		a.z*b.x - a.x*b.z,
		a.x*b.y - a.y*b.x);
}

//
inline float length(const vector3_base<float> &a)
{
	return sqrtf(a.x*a.x + a.y*a.y + a.z*a.z);
}

inline vector3_base<float> normalize(const vector3_base<float> &v)
{
	float l = (float)(1.0f/sqrtf(v.x*v.x + v.y*v.y + v.z*v.z));
	return vector3_base<float>(v.x*l, v.y*l, v.z*l);
}

typedef vector3_base<float> vec3;
typedef vector3_base<bool> bvec3;
typedef vector3_base<int> ivec3;

// ------------------------------------

template<typename T>
class vector4_base
{
public:
	union { T x,r; };
	union { T y,g; };
	union { T z,b; };
	union { T w,a; };

	vector4_base() {}
	vector4_base(T nx, T ny, T nz, T nw)
	{
		x = nx;
		y = ny;
		z = nz;
		w = nw;
	}

	vector4_base operator +(const vector4_base& v) const { return vector4_base(x + v.x, y + v.y, z + v.z, w + v.w); }
	vector4_base operator -(const vector4_base& v) const { return vector4_base(x - v.x, y - v.y, z - v.z, w - v.w); }
	vector4_base operator -() const { return vector4_base(-x, -y, -z, -e); }
	vector4_base operator *(const vector4_base& v) const { return vector4_base(x * v.x, y * v.y, z * v.z, w * v.z); }
	vector4_base operator *(const T v) const { return vector4_base(x * v, y * v, z * v, w * v); }
	vector4_base operator /(const vector4_base& v) const { return vector4_base(x/v.x, y/v.y, z/v.w, w/v.h); }
	vector4_base operator /(const T v) const { return vector4_base(x/v, y/v, z/v, w/v); }

	const vector4_base &operator =(const vector4_base& v) { x = v.x; y = v.y; z = v.z; w = v.z; return *this; }

	const vector4_base& operator +=(const vector4_base& v) { x += v.x; y += v.y; z += v.z; w += v.w; return *this; }
	const vector4_base& operator -=(const vector4_base& v) { x -= v.x; y -= v.y; z -= v.z; w -= v.w; return *this; }
	const vector4_base& operator *=(const T v) { x *= v; y *= v; z *= v; w *= v; return *this; }
	const vector4_base& operator *=(const vector4_base& v) { x *= v.x; y *= v.y; z *= v.z; w *= v.w; return *this; }
	const vector4_base& operator /=(const T v) { x /= v; y /= v; w /= v; h /= v; return *this; }
	const vector4_base& operator /=(const vector4_base& v) { x /= v.x; y /= v.y; z /= v.z; w /= v.w; return *this; }

	bool operator ==(const vector4_base& v) const { return x == v.x && y == v.y && z == v.z && w == v.w; } //TODO: do this with an eps instead
	bool operator !=(const vector4_base &v) const { return x != v.x || y != v.y || z != v.z || w != v.w; }
	
	operator const T* () { return &x; }
};

template<typename T>
inline T dot(const vector3_base<T>& a, const vector4_base<T>& b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

template<typename T>
inline vector4_base<T> cross(const vector4_base<T>& a, const vector4_base<T>& b)
{
	return vector4_base<T>(
		a.y * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z,
		a.x * b.y - a.y * b.x);
}

template<typename T>
inline float length(const vector4_base<T>& a)
{
	return sqrtf(a.x * a.x + a.y * a.y + a.z * a.z);
}
template<typename T>
inline vector4_base<T> normalize(const vector4_base<T> & v)
{
	float l = (float)(1.0f / sqrtf(v.x * v.x + v.y * v.y + v.z * v.z));
	return vector4_base<T>(v.x * l, v.y * l, v.z * l);
}
typedef vector4_base<float> vec4;
typedef vector4_base<bool> bvec4;
typedef vector4_base<int> ivec4;


class matrix_base
{
protected:
	short rows=0;
	short columns=0;
public:
	virtual void createIdentity();
	virtual ~matrix_base();
    short getSize(){ return rows * columns; }
	short getColumns(){ return  columns; }
	short getRows(){ return  rows; }
	
};
template<typename T>
class matrix2x2_base : matrix_base
{
public:
	vector2_base<T>* values = nullptr;
	void createIdentity() override
	{
		values[0].x = 1;
		values[1].y = 1;
	}
	matrix2x2_base()
	{
		rows = 2;
		columns = 2;
		values = new vector2_base<T>[rows];
		for (int i = 0; i < rows; i++)
		{
			values[i].x = 0;
			values[i].y = 0;
		}
	}
	~matrix2x2_base() override
	{
		delete values;
		values = nullptr;
	}
	bool operator ==(const matrix2x2_base& mat) const
	{
		return  mat.values[0] == values[0] && mat.values[1] == values[1];
	}
	bool operator !=(const matrix2x2_base& mat) const
	{
		return  mat.values[0] != values[0] && mat.values[1] != values[1];
	}
	matrix2x2_base operator + (const matrix2x2_base& mat) const
	{
		matrix2x2_base result;
		for (int i = 0; i < rows; i++)
		{
			result.values[i] = mat.values[i] + values[i];
		}
		return result;
	}
	matrix2x2_base operator - (const matrix2x2_base& mat) const
	{
		matrix2x2_base result;
		for (int i = 0; i < rows; i++)
		{
			result.values[i] = values[i] - mat.values[i];
		}
		return result;
	}
	vector2_base<T> operator *(const vector2_base<T>& vec) const
	{
		vector2_base<T> result;
		result.x = (values[0].x * vec.x) + (values[0].y * vec.y);
		result.y = (values[1].x * vec.x) + (values[1].y * vec.y);
		return result;
	}
	matrix2x2_base operator *(const matrix2x2_base<T>& mat) const
	{
		matrix2x2_base<T> result;
		for (int i = 0; i < rows; i++)
		{
			result.values[i].x = (values[i].x * mat.values[0].x) + (values[i].y * mat.values[1].x);
			result.values[i].y = (values[i].x * mat.values[0].y) + (values[i].y * mat.values[1].y);
		}
		return result;
	}
};
typedef matrix2x2_base<int> imat2;
typedef matrix2x2_base<float> mat2;
typedef matrix2x2_base<bool> bmat2;

template<typename T>
class matrix3x3_base : matrix_base
{
public:
	vector3_base<T>* values = nullptr;
	void createIdentity() override
	{
		values[0].x = 1;
		values[1].y = 1;
		values[2].z = 1;
	}
	matrix3x3_base()
	{
		rows = 3;
		columns = 3;
		values = new vector3_base<T>[rows];
		for (int i = 0; i < rows; i++)
		{
				values[i].x = 0;
				values[i].y = 0;
				values[i].z = 0;
		}
	}
	~matrix3x3_base() override
	{
		delete values;
		values = nullptr;
	}
	bool operator ==(const matrix3x3_base& mat) const
	{
		return  mat.values[0] == values[0] && mat.values[1] == values[1] &&  mat.values[2] == values[2];
	}
	bool operator !=(const matrix3x3_base& mat) const
	{
		return  mat.values[0] != values[0] && mat.values[1] != values[1] && mat.values[2] != values[2];
	}
	matrix3x3_base operator + (const matrix3x3_base& mat) const
	{
		matrix3x3_base result;
		for (int i = 0; i < rows; i++)
		{
			result.values[i] = mat.values[i] + values[i];
		}
		return result;
	}
	matrix3x3_base operator - (const matrix3x3_base& mat) const
	{
		matrix3x3_base result;
		for (int i = 0; i < rows; i++)
		{
			result.values[i] = values[i] - mat.values[i];
		}
		return result;
	}
	vector3_base<T> operator *(const vector2_base<T>& vec) const
	{
		vector3_base<T> result;
		result.x = (values[0].x * vec.x) + (values[0].y * vec.y) + (values[0].z * vec.z);
		result.y = (values[1].x * vec.x) + (values[1].y * vec.y) + (values[1].z * vec.z);
		result.z = (values[2].x * vec.x) + (values[2].y * vec.y) + (values[2].z * vec.z);
		return result;
	}
	matrix3x3_base operator *(const matrix2x2_base<T>& mat) const
	{
		matrix3x3_base<T> result;
		for (int i = 0; i < rows; i++)
		{
			result.values[i].x = (values[i].x * mat.values[0].x) + (values[i].y * mat.values[1].x) + (values[i].z * mat.values[2].x);
			result.values[i].y = (values[i].x * mat.values[0].y) + (values[i].y * mat.values[1].y) + (values[i].z * mat.values[2].y);
			result.values[i].z = (values[i].x * mat.values[0].z) + (values[i].y * mat.values[1].z) + (values[i].z * mat.values[2].z);
		}
		return result;
	}
};
typedef matrix3x3_base<int> imat3;
typedef matrix3x3_base<float> mat3;
typedef matrix3x3_base<bool> bmat3;
template<typename T>
class matrix4x4_base : matrix_base
{
public:
	vector4_base<T>* values = nullptr;
	void createIdentity() override
	{
		values[0].x = 1;
		values[1].y = 1;
		values[2].z = 1;
		values[3].w = 1;
	}
	matrix4x4_base()
	{
		rows = 4;
		columns = 4;
		values = new vector4_base<T>[rows];
		for (int i = 0; i < rows; i++)
		{
			values[i].x = 0;
			values[i].y = 0;
			values[i].z = 0;
			values[i].w = 0;
		}
	}
	~matrix4x4_base() override
	{
		delete values;
		values = nullptr;
	}
	bool operator ==(const matrix4x4_base& mat) const
	{
		return  mat.values[0] == values[0] && mat.values[1] == values[1] && mat.values[2] == values[2] && mat.values[3] == values[3];
	}
	bool operator !=(const matrix4x4_base& mat) const
	{
		return  mat.values[0] != values[0] && mat.values[1] != values[1] && mat.values[2] != values[2] != mat.values[3] != values[3] ;
	}
	matrix4x4_base operator + (const matrix4x4_base& mat) const
	{
		matrix4x4_base result;
		for (int i = 0; i < rows; i++)
		{
			result.values[i] = mat.values[i] + values[i];
		}
		return result;
	}
	matrix4x4_base operator - (const matrix4x4_base& mat) const
	{
		matrix4x4_base result;
		for (int i = 0; i < rows; i++)
		{
			result.values[i] = values[i] - mat.values[i];
		}
		return result;
	}
	vector4_base<T> operator * (const vector2_base<T>& vec) const
	{
		vector4_base<T> result;
		result.x = (values[0].x * vec.x) + (values[0].y * vec.y) + (values[0].z * vec.z) + (values[0].w * vec.w);
		result.y = (values[1].x * vec.x) + (values[1].y * vec.y) + (values[1].z * vec.z) + (values[1].w * vec.w);
		result.z = (values[2].x * vec.x) + (values[2].y * vec.y) + (values[2].z * vec.z) + (values[2].w * vec.w);
		result.w = (values[3].x * vec.x) + (values[3].y * vec.y) + (values[3].z * vec.z) + (values[3].w * vec.w);
		return result;
	}
	matrix4x4_base operator *(const matrix2x2_base<T>& mat) const
	{
		matrix4x4_base<T> result;
		for (int i = 0; i < rows; i++)
		{
			result.values[i].x = (values[i].x * mat.values[0].x) + (values[i].y * mat.values[1].x) + (values[i].z * mat.values[2].x) + (values[i].w * mat.values[3].x);
			result.values[i].y = (values[i].x * mat.values[0].y) + (values[i].y * mat.values[1].y) + (values[i].z * mat.values[2].y) + (values[i].w * mat.values[3].y);
			result.values[i].z = (values[i].x * mat.values[0].z) + (values[i].y * mat.values[1].z) + (values[i].z * mat.values[2].z) + (values[i].w * mat.values[3].z);
			result.values[i].w = (values[i].x * mat.values[0].w) + (values[i].y * mat.values[1].w) + (values[i].z * mat.values[2].w) + (values[i].w * mat.values[3].w);
		}
		return result;
	}
};
template<typename  T>
inline matrix4x4_base<T> translate(vector4_base<T> &position )
{
	matrix4x4_base<T> translateMatrix;
	translateMatrix.createIdentity();
	translateMatrix.values[0].w = position.x;
	translateMatrix.values[1].w = position.y;
	translateMatrix.values[2].w = position.z;
	return translateMatrix ;
	
}
template<typename T>
inline  matrix4x4_base<T> rotateX(T angle)
{
	angle = angle * pi / 180.0f;
	matrix4x4_base<T> rotateMatrixX;
	rotateMatrixX.createIdentity();
	rotateMatrixX.values[1].y = cosf(angle);
	rotateMatrixX.values[1].z = -sinf(angle);
	rotateMatrixX.values[2].y = sin(angle);
	rotateMatrixX.values[2].z = cosf(angle);
	return rotateMatrixX;
}
template<typename T>
inline  matrix4x4_base<T> rotateY(T angle)
{
	angle = angle * pi / 180.0f;
	matrix4x4_base<T> rotateMatrixY;
	rotateMatrixY.createIdentity();
	rotateMatrixY.values[0].x = cosf(angle);
	rotateMatrixY.values[0].y = -sinf(angle);
	rotateMatrixY.values[2].x = sin(angle);
	rotateMatrixY.values[2].y = cosf(angle);
	return   rotateMatrixY;
}
template<typename T>
inline  matrix4x4_base<T> rotateZ(T angle)
{
	angle = angle * pi / 180.0f;
	matrix4x4_base<T> rotateMatrixZ;
	rotateMatrixZ.values[0].x = cosf(angle);
	rotateMatrixZ.values[0].y = -sinf(angle);
	rotateMatrixZ.values[1].x = sin(angle);
	rotateMatrixZ.values[1].y = cosf(angle);
	return rotateMatrixZ;
}
template<typename T>
inline  matrix4x4_base<T> rotate(vector3_base<T>& angles)
{
	return rotateZ(angles.z) * rotateY(angles.y) * rotateX(angles.x);
}
template<typename T>
inline  matrix4x4_base<T> scale(vector3_base<T>& size)
{
	matrix4x4_base<T> scaleMatrix;
	scaleMatrix.values[0].x = size.x;
	scaleMatrix.values[1].y = size.y;
	scaleMatrix.values[2].z = size.z;
	scaleMatrix.values[3].w = 1;
	return scaleMatrix;
}
template<typename T>
inline  matrix4x4_base<T> perspective(float FOV,float zNear,float zFar ,int windowWidth,int windowHeight)
{
	matrix4x4_base<T> perspectiveMatrix;
	float ar = windowWidth / windowHeight;
	float zRange = zNear - zFar;
	FOV = FOV * pi / 180.0f;
	float halfFOV = tanf(FOV / 2);
	perspectiveMatrix.values[0].x = 1.0f / (halfFOV*ar);
	perspectiveMatrix.values[1].y = 1.0f / halfFOV;
	perspectiveMatrix.values[2].z = (-zNear - zFar) / zRange;
	perspectiveMatrix.values[2].w = 2.0f * zFar * zNear / zRange;
	perspectiveMatrix.values[3].z = 1.0f;
	return perspectiveMatrix;
}
template<typename T>
inline  matrix4x4_base<T> lookAt(vector4_base<T> &position ,vector4_base<T> &up,vector4_base<T> &target)
{
	matrix4x4_base<T> viewMatrix,lookAtMatrix;
	viewMatrix = translate(-position);
	vector4_base<T> n = normalize(target);
	vector4_base<T> u = cross(normalize(up), n);
	vector4_base<T> v = normalize(n, u);
	n.w = 0;
	v.w = 0;
	u.w = 0;
	lookAtMatrix.values[0] = u;
	lookAtMatrix.values[1] = v;
	lookAtMatrix.values[2] = n;
	lookAtMatrix.values[3].w = 0;
	return viewMatrix * lookAtMatrix;
}
#endif
