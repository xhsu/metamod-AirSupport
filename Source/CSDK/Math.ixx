export module Math;

import <algorithm>;
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
