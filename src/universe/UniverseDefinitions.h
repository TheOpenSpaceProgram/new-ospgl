#pragma once
#include "kepler/KeplerElements.h"
#include "element/SystemElement.h"

class Trajectory;

using LightStateVector = std::vector<LightCartesianState>;
using StateVector = std::vector<CartesianState>;
using PosVector = std::vector<glm::dvec3>;
using ElementVector = std::vector<SystemElement>;
using MassVector = std::vector<double>;
using TrajectoryVector = std::vector<Trajectory*>;
