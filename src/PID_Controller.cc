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
/// @file PID_Controller.cc
///

#include "../include/PID_Controller.h"
#include "../include/math_util.h"
#include "../include/df2_filter.h"

//////////////////////////////////////////////////////////////////////////////////////////////////
///
//////////////////////////////////////////////////////////////////////////////////////////////////
PID_Controller::PID_Controller(double _kp, double _ki, double _kd) : Kp(_kp),
                                                                     Ki(_ki),
                                                                     Kd(_kd),
                                                                     limit_integrator(false),
                                                                     limit_output(false),
                                                                     outputSaturatedFlag(false),
                                                                     e_prev(0)
{
    configureCompMode();
}
//////////////////////////////////////////////////////////////////////////////////////////////////
///
//////////////////////////////////////////////////////////////////////////////////////////////////
void PID_Controller::configureCompMode()
{
    uint8_t compMode = 0;
    compMode |= Kp != 0 ? DIGITAL_CONTROL::P_BIT : 0;
    compMode |= Ki != 0 ? DIGITAL_CONTROL::I_BIT : 0;
    compMode |= Kd != 0 ? DIGITAL_CONTROL::D_BIT : 0;
    compensationMode = compMode;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
///
//////////////////////////////////////////////////////////////////////////////////////////////////
void PID_Controller::configureGains(double _kp, double _ki, double _kd)
{
    Kp = _kp;
    Ki = _ki;
    Kd = _kd;
    configureCompMode();
}

//////////////////////////////////////////////////////////////////////////////////////////////////
///
//////////////////////////////////////////////////////////////////////////////////////////////////
void PID_Controller::configureIntegratorSaturation(double ulim, double llim)
{
    integrator_limits.llim = llim;
    integrator_limits.ulim = ulim;
    limit_integrator = true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
///
//////////////////////////////////////////////////////////////////////////////////////////////////
void PID_Controller::configureOutputSaturation(double ulim, double llim)
{
    output_limits.llim = llim;
    output_limits.ulim = ulim;
    limit_output = true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
///
//////////////////////////////////////////////////////////////////////////////////////////////////
void PID_Controller::reset()
{
    firstTime = true;
    e_prev = 0;
    integratorState = 0.0;
    resetIntegrator();
}

//////////////////////////////////////////////////////////////////////////////////////////////////
///
//////////////////////////////////////////////////////////////////////////////////////////////////
void PID_Controller::resetIntegrator()
{
    integratorState = 0.0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
///
//////////////////////////////////////////////////////////////////////////////////////////////////
bool PID_Controller::integratorIsSaturated()
{
    bool satFlag = (integratorState == integrator_limits.ulim) || (integratorState == integrator_limits.llim);
    return satFlag;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
///
//////////////////////////////////////////////////////////////////////////////////////////////////
bool PID_Controller::outputIsSaturated()
{
    return outputSaturatedFlag;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
///
//////////////////////////////////////////////////////////////////////////////////////////////////
void PID_Controller::update(double e, double dt, double *uC)
{
    double prop_term{0};
    double int_term{0};
    double diff_term{0};
    // double output_pre_sat{0};
    // double output_post_sat{0};
    double output{0};

    if (compensationMode & DIGITAL_CONTROL::P_BIT)
    {
        prop_term = e * Kp;
        output += prop_term;
    }

    if (compensationMode & DIGITAL_CONTROL::I_BIT)
    {
        integratorState += e * Ki * dt;
        if (limit_integrator)
            int_term = saturate(integratorState, integrator_limits.llim, integrator_limits.ulim);
        else
            int_term = integratorState;
        output += int_term;
    }

    if (compensationMode & DIGITAL_CONTROL::D_BIT)
    {
        double diff = 0;
        if (!firstTime)
        {
            diff = (e - e_prev)/dt;
        }
        e_prev = e;
        firstTime = false;
        diff_term = diff * Kd;
        output += diff_term;
    }

    *uC = output;
}