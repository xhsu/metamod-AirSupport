module;

#include <cassert>

export module Math;

export import hlsdk;
export import std.compat;


export inline auto get_spherical_coord(const Vector &vecOrigin, const Quaternion &qRotation, double radius, double inclination, double azimuth) noexcept
{
	radius = std::clamp(radius, 0.0, 8192.0);	// r ∈ [0, ∞)
	inclination = std::clamp(inclination * std::numbers::pi / 180.0, 0.0, std::numbers::pi);	// θ ∈ [0, π]
	azimuth = std::clamp(azimuth * std::numbers::pi / 180.0, 0.0, std::numbers::pi * 2.0);	// φ ∈ [0, 2π)

	auto const length = radius * sin(inclination);

	return vecOrigin + qRotation * Vector(
		length * cos(azimuth),
		length * sin(azimuth),
		radius * cos(inclination)
	);
};

export inline auto get_spherical_coord(double radius, double inclination, double azimuth) noexcept
{
	radius = std::clamp(radius, 0.0, 8192.0);	// r ∈ [0, ∞)
	inclination = std::clamp(inclination * std::numbers::pi / 180.0, 0.0, std::numbers::pi);	// θ ∈ [0, π]
	azimuth = std::clamp(azimuth * std::numbers::pi / 180.0, 0.0, std::numbers::pi * 2.0);	// φ ∈ [0, 2π)

	auto const length = radius * sin(inclination);

	return Vector(
		length * cos(azimuth),
		length * sin(azimuth),
		radius * cos(inclination)
	);
};

export inline auto get_cylindrical_coord(double radius, double azimuth, double height) noexcept
{
	radius = std::clamp(radius, 0.0, 8192.0);	// r ∈ [0, ∞)
	azimuth = std::clamp(azimuth * std::numbers::pi / 180.0, 0.0, std::numbers::pi * 2.0);	// φ ∈ [0, 2π)
	// z ∈ (-∞, +∞)

	return Vector(
		radius * cos(azimuth),
		radius * sin(azimuth),
		height
	);
}

// https://www.euclideanspace.com/maths/algebra/realNormedAlgebra/quaternions/slerp/index.htm
export template <typename Proj = std::identity> inline
Quaternion quat_slerp(Quaternion const &qSrc, Quaternion qDest, double t, Proj&& proj = {}) noexcept
{
	static constexpr double epsilon = 0.0001;

	double p{}, q{};
	t = std::invoke(proj, t);

	auto cosTheta = qSrc.a * qDest.a + qSrc.b * qDest.b + qSrc.c * qDest.c + qSrc.d * qDest.d;

	if (cosTheta < 0.0)
	{
		qDest = Quaternion(-qDest.a, -qDest.b, -qDest.c, -qDest.d);
		cosTheta = -cosTheta;
	}

	if ((1.0 - abs(cosTheta)) > epsilon)
	{
		auto const theta = acos(cosTheta);
		auto const sinTheta = sin(theta);

		q = sin((1 - t) * theta) / sinTheta;
		p = sin(t * theta) / sinTheta;
	}
	else
	{
		q = 1 - t;
		p = t;
	}

	return Quaternion(
		(q * qSrc.a) + (p * qDest.a),
		(q * qSrc.b) + (p * qDest.b),
		(q * qSrc.c) + (p * qDest.c),
		(q * qSrc.d) + (p * qDest.d)
	);
}

export template <typename T, typename Proj = std::identity> constexpr
T arithmetic_lerp(T const& lhs, T const& rhs, double t, Proj&& proj = {}) noexcept
{
	assert(t <= 1 && t >= 0);
	t = std::invoke(proj, t);
	return (lhs * t) + (rhs * (1.0 - t));
}

// Both of them must be normalized
export template <typename Proj = std::identity> inline
Vector vector_slerp(Vector const& vStart, Vector const& vEnd, double t, Proj&& proj = {}) noexcept
{
	assert(t <= 1 && t >= 0);
	t = std::invoke(proj, t);

	auto const omega = std::acos(DotProduct(vStart, vEnd));
	auto const sin_omega = std::sin(omega);

	auto const term_1 = vStart * (std::sin(omega * (1.0 - t)) / sin_omega);
	auto const term_2 = vEnd * (std::sin(omega * t) / sin_omega);

	return term_1 + term_2;
}

// https://inloop.github.io/interpolator/
export namespace Interpolation
{
	// Basic

	constexpr double smooth_step(double const t) noexcept
	{
		return (t * t * t * (t * (t * 6 - 15) + 10));
	}

	template <double factor> inline
	double spring(double const t) noexcept
	{
		return std::pow(2.0, -10.0 * t) * std::sin((t - factor / 4.0) * (2.0 * std::numbers::pi) / factor) + 1;
	}

	// Android

	inline double acce_then_dece(double const t) noexcept
	{
		return (std::cos((t + 1) * std::numbers::pi) / 2.0) + 0.5;
	}

	constexpr double bounce(double const t) noexcept
	{
		constexpr auto l = [](double const t) /*#UPDATE_AT_CPP23 static*/ noexcept { return t * t * 8; };
		
		if (t < 0.3535)
			return l(t);
		else if (t < 0.7408)
			return l(t - 0.54719) + 0.7;
		else if (t < 0.9644)
			return l(t - 0.8526) + 0.9;
		else
			return l(t - 1.0435) + 0.95;
	}

	template <double factor = 1.0> inline
	double accelerated(double const t) noexcept
	{
		if constexpr (factor == 1.0)
			return t * t;
		else
			return std::pow(t, 2 * factor);
	}

	template <double tension = 2.0> constexpr
	double anticipate(double const t) noexcept
	{
		return t * t * ((tension + 1) * t - tension);
	}

	template <double tension = 2.0 * 1.5> constexpr
	double antic_then_overshoot(double const t) noexcept
	{
		constexpr auto a = [](double const t) /*#UPDATE_AT_CPP23 static*/ noexcept
			{ return t * t * ((tension + 1) * t - tension); };
		constexpr auto o = [](double const t) /*#UPDATE_AT_CPP23 static*/ noexcept
			{ return t * t * ((tension + 1) * t + tension); };

		if (t < 0.5)
			return 0.5 * a(t * 2.0);
		else
			return 0.5 * (o(t * 2.0 - 2.0) + 2.0);
	}

	template <double cyc_count = 1.0> inline
	double cycle(double const t) noexcept
	{
		return std::sin(2 * cyc_count * std::numbers::pi * t);
	}

	template <double factor = 1.0> inline
	double decelerated(double const t) noexcept
	{
		if constexpr (factor == 1.0)
			return 1 - (1 - t) * (1 - t);
		else
			return (1.0 - std::pow((1.0 - t), 2 * factor));
	}

	template <double tension = 2.0> constexpr
	double overshoot(double t) noexcept
	{
		t -= 1.0;
		return t * t * ((tension + 1) * t + tension) + 1.0;
	}

	// Advanced

	template <double tangent0 = 4.0, double tangent1 = 4.0> constexpr
	double cubic_hermite(double const t) noexcept
	{
		constexpr auto l =
			[](double t, double p0, double p1) noexcept
			{
				auto const t2 = t * t;
				auto const t3 = t2 * t;
				return (2 * t3 - 3 * t2 + 1) * p0 + (t3 - 2 * t2 + t) * tangent0 + (-2 * t3 + 3 * t2) * p1 + (t3 - t2) * tangent1;
			};

		return l(t, 0, 1);
	}
}
