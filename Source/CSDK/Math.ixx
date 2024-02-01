export module Math;

export import <cassert>;
export import <cmath>;

export import <algorithm>;
export import <numbers>;

export import vector;

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
export inline auto quat_slerp(Quaternion const &qSrc, Quaternion qDest, double t) noexcept
{
	static constexpr double epsilon = 0.0001;

	double p{}, q{};

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

export template <typename T> constexpr
T linear_interpolation(T const& lhs, T const& rhs, double const t) noexcept
{
	assert(t <= 1 && t >= 0);
	return (lhs * t) + (rhs * (1.0 - t));
}

export template <typename T> constexpr
T smoothstep_interpolation(T const& lhs, T const& rhs, double t) noexcept
{
	assert(t <= 1 && t >= 0);
	t = (t * t * t * (t * (t * 6 - 15) + 10));
	return (lhs * t) + (rhs * (1.0 - t));
}

export template <typename T> constexpr
T accelerated_interpolation(T const& lhs, T const& rhs, double t) noexcept
{
	assert(t <= 1 && t >= 0);
	t = t * t;
	return (lhs * t) + (rhs * (1.0 - t));
}

export template <typename T> constexpr
T decelerated_interpolation(T const& lhs, T const& rhs, double t) noexcept
{
	assert(t <= 1 && t >= 0);
	t = 1 - (1 - t) * (1 - t);
	return (lhs * t) + (rhs * (1.0 - t));
}
