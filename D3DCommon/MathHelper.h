//***************************************************************************************
// MathHelper.h 
//
// Helper math class.
//***************************************************************************************

#pragma once

#include <Windows.h>
#include <DirectXMath.h>
#include <cstdint>

class MathHelper
{
public:
	// Returns random float in [0, 1).
	static float RandF()
	{
		return (float)(rand()) / (float)RAND_MAX;
	}

	// Returns random float in [a, b).
	static float RandF(float a, float b)
	{
		return a + RandF()*(b-a);
	}

    static int Rand(int a, int b)
    {
        return a + rand() % ((b - a) + 1);
    }

	template<typename T>
	static T Min(const T& a, const T& b)
	{
		return a < b ? a : b;
	}

	template<typename T>
	static T Max(const T& a, const T& b)
	{
		return a > b ? a : b;
	}
	 
	template<typename T>
	static T Lerp(const T& a, const T& b, float t)
	{
		return a + (b-a)*t;
	}

	template<typename T>
	static T Clamp(const T& x, const T& low, const T& high)
	{
		return x < low ? low : (x > high ? high : x); 
	}

	// Returns the polar angle of the point (x,y) in [0, 2*PI).
	static float AngleFromXY(float x, float y);

	static DirectX::XMVECTOR SphericalToCartesian(float radius, float theta, float phi)
	{
		return DirectX::XMVectorSet(
			radius*sinf(phi)*cosf(theta),
			radius*cosf(phi),
			radius*sinf(phi)*sinf(theta),
			1.0f);
	}

    static DirectX::XMMATRIX InverseTranspose(DirectX::CXMMATRIX M)
	{
		// Inverse-transpose is just applied to normals.  So zero out 
		// translation row so that it doesn't get into our inverse-transpose
		// calculation--we don't want the inverse-transpose of the translation.
        DirectX::XMMATRIX A = M;
        A.r[3] = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

        DirectX::XMVECTOR det = DirectX::XMMatrixDeterminant(A);
        return DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(&det, A));
	}

    static DirectX::XMFLOAT4X4 Identity4x4()
    {
        static DirectX::XMFLOAT4X4 I(
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f);

        return I;
    }

    static DirectX::XMVECTOR RandUnitVec3();
    static DirectX::XMVECTOR RandHemisphereUnitVec3(DirectX::XMVECTOR n);

	static const float Infinity;
	static const float Pi;

	static DirectX::XMFLOAT3 GetPosition(const DirectX::XMFLOAT4X4 transform)
	{
		return DirectX::XMFLOAT3(transform._41, transform._42, transform._43);
	}

	static void UpdatePosition(DirectX::XMFLOAT4X4& transform, const DirectX::XMFLOAT3 pos)
	{
		transform._41 += pos.x;
		transform._42 += pos.y;
		transform._43 += pos.z;
	}

	static DirectX::XMFLOAT4X4 MultiplyXMFLOAT4x4(DirectX::XMFLOAT4X4 m1, DirectX::XMFLOAT4X4 m2)
	{
		float v11 = m1._11;
		float v12 = m1._12;
		float v13 = m1._13;
		float v14 = m1._14;

		float v21 = m1._21;
		float v22 = m1._22;
		float v23 = m1._23;
		float v24 = m1._24;

		float v31 = m1._31;
		float v32 = m1._32;
		float v33 = m1._33;
		float v34 = m1._34;

		float v41 = m1._41;
		float v42 = m1._42;
		float v43 = m1._43;
		float v44 = m1._44;

		float q11 = m2._11;
		float q12 = m2._12;
		float q13 = m2._13;
		float q14 = m2._14;
			  		 
		float q21 = m2._21;
		float q22 = m2._22;
		float q23 = m2._23;
		float q24 = m2._24;
			  		 
		float q31 = m2._31;
		float q32 = m2._32;
		float q33 = m2._33;
		float q34 = m2._34;
			  		 
		float q41 = m2._41;
		float q42 = m2._42;
		float q43 = m2._43;
		float q44 = m2._44;

		float a11 = v11 * q11 + v12 * q21 + v13 * q31 + v14 * q41;
		float a12 = v11 * q12 + v12 * q22 + v13 * q32 + v14 * q42;
		float a13 = v11 * q13 + v12 * q23 + v13 * q33 + v14 * q43;
		float a14 = v11 * q14 + v12 * q24 + v13 * q34 + v14 * q44;

		float a21 = v21 * q11 + v22 * q21 + v23 * q31 + v24 * q41;
		float a22 = v21 * q12 + v22 * q22 + v23 * q32 + v24 * q42;
		float a23 = v21 * q13 + v22 * q23 + v23 * q33 + v24 * q43;
		float a24 = v21 * q14 + v22 * q24 + v23 * q34 + v24 * q44;

		float a31 = v31 * q11 + v32 * q21 + v33 * q31 + v34 * q41;
		float a32 = v31 * q12 + v32 * q22 + v33 * q32 + v34 * q42;
		float a33 = v31 * q13 + v32 * q23 + v33 * q33 + v34 * q43;
		float a34 = v31 * q14 + v32 * q24 + v33 * q34 + v34 * q44;

		float a41 = v41 * q11 + v42 * q21 + v43 * q31 + v44 * q41;
		float a42 = v41 * q12 + v42 * q22 + v43 * q32 + v44 * q42;
		float a43 = v41 * q13 + v42 * q23 + v43 * q33 + v44 * q43;
		float a44 = v41 * q14 + v42 * q24 + v43 * q34 + v44 * q44;

		return DirectX::XMFLOAT4X4(a11, a12, a13, a14, a21, a22, a23, a24, a31, a32, a33, a34, a41, a42, a43, a44);
	}
};

