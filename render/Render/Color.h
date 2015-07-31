#pragma once

#include <Eigen/Dense>

Eigen::Vector3f LabToRgb(Eigen::Vector3f lab);
Eigen::Vector3f RgbToLab(Eigen::Vector3f rgb);

bool IsValidLab(Eigen::Vector3f rgb);