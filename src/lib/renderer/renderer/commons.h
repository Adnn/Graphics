#pragma once

#include <math/Rectangle.h>
#include <math/Vector.h>

namespace ad {

template <class T_number>
using Rectangle=math::Rectangle<T_number>;

template <class T_number>
using Vec2=math::Vec<2, T_number>;

template <class T_number>
using Vec3=math::Vec<3, T_number>;

template <class T_number>
using Vec4=math::Vec<4, T_number>;

template <class T_number>
using Size2=math::Size<2, T_number>;

template <class T_number>
using Position2=math::Position<2, T_number>;

static const std::array<float, 4> gBlack = {0.f, 0.f, 0.f, 1.f};

/// \todo Replace with a proper path class, as standardised in C++17
//typedef std::string path;

} // namespace ad
