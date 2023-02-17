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
/// @file teensy41_device.cpp
///


#pragma once
#define CONCAT_(A, B) A ## B
#define CONCAT(A, B) CONCAT_(A, B)

#define EVAL(M) M


#if defined (STR)
#undef STR
#endif
#define STR(s) #s

#if defined (XSTR)
#undef XSTR
#endif
#define XSTR(s) STR(s)
