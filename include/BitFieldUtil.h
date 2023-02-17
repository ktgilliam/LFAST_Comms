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
/// @file BitFieldUtil.cpp
///
#pragma once
#include <cinttypes>

typedef union
{
    uint32_t U32;
    int32_t I32;
    uint16_t U16[2];
    union
    {
        struct
        {
            uint16_t LOWER;
            uint16_t UPPER;
        } UNSIGNED;

        int16_t I16[2];
        struct
        {
            int16_t LOWER;
            int16_t UPPER;
        } SIGNED;

    } PARTS;
} ConversionBuffer32;


template <typename T>
union ConversionBuffer
{
    T WHOLE;
    uint16_t U16_PARTS[sizeof(T)/sizeof(uint16_t)];
    uint8_t U8_PARTS[sizeof(T)/sizeof(uint8_t)];
};

template <typename T>
using ConversionBuffer_t = union ConversionBuffer<T>;


#define M32_LOWER 0x0000FFFF
#define M32_UPPER 0xFFFF0000