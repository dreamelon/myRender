#include "Color.h"

Color operator*(const Color& col, float t) {
	return Color(col.r * t, col.g * t, col.b * t, col.a * t);
}

Color operator+(const Color& lhs, const Color& rhs) {
	return Color(lhs.r + rhs.r, lhs.g + rhs.g, lhs.b + rhs.b, lhs.a + rhs.a);
}