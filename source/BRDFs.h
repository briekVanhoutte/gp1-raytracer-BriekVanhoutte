#pragma once
#include <cassert>
#include "Math.h"

namespace dae
{
	namespace BRDF
	{
		/**
		 * \param kd Diffuse Reflection Coefficient
		 * \param cd Diffuse Color
		 * \return Lambert Diffuse Color
		 */
		static ColorRGB Lambert(float kd, const ColorRGB& cd)
		{
			ColorRGB rho = cd * kd;
			return rho/M_PI;
		}

		static ColorRGB Lambert(const ColorRGB& kd, const ColorRGB& cd)
		{
			// Calculate the reflection coefficient for each channel separately
			ColorRGB rho;
			rho.r = kd.r * cd.r;
			rho.g = kd.g * cd.g;
			rho.b = kd.b * cd.b;

			// Normalize the reflection coefficient by dividing by pi
			rho = rho / float(M_PI);

			return rho;
		}


		/**
		 * \brief todo
		 * \param ks Specular Reflection Coefficient
		 * \param exp Phong Exponent
		 * \param l Incoming (incident) Light Direction
		 * \param v View Direction
		 * \param n Normal of the Surface
		 * \return Phong Specular Color
		 */
		static ColorRGB Phong(float ks, float exp, const Vector3& l, const Vector3& v, const Vector3& n)
		{	
			Vector3 r = l - 2 * Vector3::Dot(n, l) * n;
			float cosAlpha = Vector3::Dot(r, v);

			float specularColor = ks * powf(cosAlpha, exp);

			return ColorRGB(specularColor,specularColor,specularColor);
		}

		/**
		 * \brief BRDF Fresnel Function >> Schlick
		 * \param h Normalized Halfvector between View and Light directions
		 * \param v Normalized View direction
		 * \param f0 Base reflectivity of a surface based on IOR (Indices Of Refrection), this is different for Dielectrics (Non-Metal) and Conductors (Metal)
		 * \return
		 */
		static ColorRGB FresnelFunction_Schlick(const Vector3& h, const Vector3& v, const ColorRGB& f0)
		{
			return f0 + (ColorRGB(1.f,1.f,1.f) - f0) * powf(1 - Vector3::Dot(h, v), 5.f);
		}

		/**
		 * \brief BRDF NormalDistribution >> Trowbridge-Reitz GGX (UE4 implemetation - squared(roughness))
		 * \param n Surface normal
		 * \param h Normalized half vector
		 * \param roughness Roughness of the material
		 * \return BRDF Normal Distribution Term using Trowbridge-Reitz GGX
		 */
		static float NormalDistribution_GGX(const Vector3& n, const Vector3& h, float roughness)
		{
			float aSqr = powf(roughness,2.f);
			float aSqrSqr = powf(aSqr,2.f);

			return aSqrSqr / (M_PI * powf(powf(Vector3::Dot(n, h),2.f) * (aSqrSqr - 1) + 1 , 2.f));
		}


		/**
		 * \brief BRDF Geometry Function >> Schlick GGX (Direct Lighting + UE4 implementation - squared(roughness))
		 * \param n Normal of the surface
		 * \param v Normalized view direction
		 * \param roughness Roughness of the material
		 * \return BRDF Geometry Term using SchlickGGX
		 */
		static float GeometryFunction_SchlickGGX(const Vector3& n, const Vector3& v, float roughness)
		{
			float dotPrNV = Vector3::Dot(n, v);
			float kDirect = powf(powf(roughness,2.f) + 1, 2.f) / 8.f;

			return  dotPrNV / ((dotPrNV * (1 - kDirect)) + kDirect);

		}

		/**
		 * \brief BRDF Geometry Function >> Smith (Direct Lighting)
		 * \param n Normal of the surface
		 * \param v Normalized view direction
		 * \param l Normalized light direction
		 * \param roughness Roughness of the material
		 * \return BRDF Geometry Term using Smith (> SchlickGGX(n,v,roughness) * SchlickGGX(n,l,roughness))
		 */
		static float GeometryFunction_Smith(const Vector3& n, const Vector3& v, const Vector3& l, float roughness)
		{
			
			return  GeometryFunction_SchlickGGX(n, v, roughness) * GeometryFunction_SchlickGGX(n, l, roughness);
		}

	}
}