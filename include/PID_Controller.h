/*******************************************************************************
Copyright 2022
Steward Observatory Engineering & Technical Services, University of Arizona
This program is free software: you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with
this program. If not, see <https://www.gnu.org/licenses/>.
*******************************************************************************/
///
/// @author Kevin Gilliam
/// @date February 16th, 2023
/// @file PID_Controller.h
///
#pragma once


#include <cinttypes>
#include <limits>

namespace DIGITAL_CONTROL
{
    constexpr double pos_inf = std::numeric_limits<double>::infinity();
    constexpr double neg_inf = -1 * pos_inf;

    const uint8_t P_BIT = 0b001;
    const uint8_t I_BIT = 0b010;
    const uint8_t D_BIT = 0b100;

    enum
    {
        P = P_BIT,
        I = I_BIT,
        PI = P_BIT | I_BIT,
        D = D_BIT,
        PD = P_BIT | D_BIT,
        ID = I_BIT | D_BIT,
        PID = P_BIT | I_BIT | D_BIT
    };
};

class PID_Controller
{
public:
    PID_Controller(double _kp = 0.0, double _ki = 0.0, double _kd = 0.0);
    virtual ~PID_Controller() {}
    void configureGains(double _kp, double _ki, double _kd);
    void configureIntegratorSaturation(double ulim, double llim);
    void configureOutputSaturation(double ulim, double llim);
    void resetIntegrator();
    void reset();
    void update(double e, double dt, double *uC);
    bool integratorIsSaturated();
    bool outputIsSaturated();

private:

    double Kp;
    double Ki;
    double Kd;

    bool limit_integrator;
    bool limit_output;
    double integratorState;
    bool outputSaturatedFlag;
    double e_prev;
    bool firstTime;

    struct limits
    {
        double ulim;
        double llim;
    };
    struct limits integrator_limits
    {
        DIGITAL_CONTROL::pos_inf, DIGITAL_CONTROL::neg_inf
    };
    struct limits output_limits
    {
        DIGITAL_CONTROL::pos_inf, DIGITAL_CONTROL::neg_inf
    };

    uint8_t compensationMode;
    void configureCompMode();
    // bool anti_windup;
};