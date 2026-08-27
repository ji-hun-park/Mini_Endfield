// Unity Physics Backend System Native Plugin API Data Protocol copyright © 2025 Unity Technologies ApS
//
// Licensed under the Unity Companion License for Unity - dependent projects--see[Unity Companion License](http://www.unity3d.com/legal/licenses/Unity_Companion_License).
//
// Unless expressly provided otherwise, the Software under this license is made available strictly on an “AS IS” BASIS WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED.Please review the license for details on these and other terms and conditions.
//
// Please note that the following data protocol types represent the data used internally by Unity's Physics Module in order to communicate with the current SDK Integration. As such these types are subject to rapid change between different Unity versions.

#pragma once
#include "PhysXPhysicsExtensionCommands.h"
#include "PhysXExtensionTypes.h"

#include "Modules/Physics/CommandLayer/PhysicsFilterData.h"
#include "Modules/Physics/CommandLayer/PhysicsPose.h"

namespace PhysicsCommands
{
    namespace PhysXExt
    {
        namespace VehicleData
        {
            // Creates a new vehicle with the given wheel count. If 'sourceVehicleId' references an existing vehicle,
            // tire friction data is copied from it. Returns the vehicle ID and updates user data pointers for wheels.
            struct CreateVehicle : Command
            {
                static constexpr auto command = PhysXExtension::CreateVehicle;

                // vehicle to copy tire data from if provided
                uint32_t sourceVehicleId;
                uint32_t wheelCount;

                // userData object in need of update after the vehicle got recreated
                void** wheelUserDatasBuffer;

                uint32_t outWrittenUserDatas;
                uint32_t outVehicleId;
            };

            // Destroys a vehicle and releases all of its wheel and suspension resources.
            struct DestroyVehicle : Command
            {
                static constexpr auto command = PhysXExtension::DestroyVehicle;

                uint32_t vehicleId;
            };

            // Enables or disables a specific wheel on the vehicle. Updates the wheel's friction curve and user data.
            // 'outAllWheelsDisabled' is set to true if all wheels are now disabled.
            struct SetVehicleWheelEnabled : Command
            {
                static constexpr auto command = PhysXExtension::SetVehicleWheelEnabled;

                Physics::PhysXExt::WheelFrictionCurve* wheelFrictionData;
                void* wheelUserData;
                uint32_t wheelId;
                bool enabled;
                bool outAllWheelsDisabled;
            };

            // Retrieves the index and total count of the first disabled wheel on this vehicle.
            struct GetFirstDisabledWheelFromVehicle : Command
            {
                static constexpr auto command = PhysXExtension::GetFirstDisabledWheelFromVehicle;

                uint32_t outWheelIndex;
                uint32_t outWheelCount;
            };

            // Sets whether the vehicle uses implicitly computed sprung masses based on wheel positions.
            struct SetVehicleHasImplicitSprungMasses : Command
            {
                static constexpr auto command = PhysXExtension::SetVehicleHasImplicitSprungMasses;

                bool hasSprungMasses;
            };

            // Queries whether the vehicle uses implicitly computed sprung masses.
            struct GetVehicleHasImplicitSprungMasses : Command
            {
                static constexpr auto command = PhysXExtension::GetVehicleHasImplicitSprungMasses;

                bool outHasSprungMasses;
            };

            // Retrieves the raycast/sweep query result for a specific wheel, including ground contact point, normal, and slip data.
            struct GetVehicleWheelQueryResult : Command
            {
                static constexpr auto command = PhysXExtension::GetVehicleWheelQueryResult;

                uint32_t wheelId;

                Physics::PhysXExt::WheelQueryResult outHit;
            };

            // Sets the collision filter data for a specific vehicle wheel, controlling what it can collide with.
            struct SetVehicleWheelFilterData : Command
            {
                static constexpr auto command = PhysXExtension::SetVehicleWheelFilterData;

                uint32_t wheelId;

                Physics::FilterData filterData;
            };

            // Retrieves the collision filter data for a specific vehicle wheel.
            struct GetVehicleWheelFilterData : Command
            {
                static constexpr auto command = PhysXExtension::GetVehicleWheelFilterData;

                uint32_t wheelId;

                Physics::FilterData outFilterData;
            };

            // Retrieves the number of wheels on this vehicle.
            struct GetVehicleWheelCount : Command
            {
                static constexpr auto command = PhysXExtension::GetVehicleWheelCount;

                uint32_t outCount;
            };

            // Retrieves the engine-side user data pointers for all wheels on this vehicle into the provided buffer.
            struct GetVehicleWheelsUserData : Command
            {
                static constexpr auto command = PhysXExtension::GetVehicleWheelsUserData;

                void** buffer;
                uint32_t bufferSize;
                uint32_t outWritten;
            };

            // Resets the cached raycast query results for a shape that has been modified or destroyed,
            // ensuring stale wheel contact data is not used.
            struct ResetVehicleQueryResultsForShape : Command
            {
                static constexpr auto command = PhysXExtension::ResetVehicleQueryResultsForShape;

                Physics::SDKObjectHandle shape;
            };

            // Sets the vehicle simulation sub-step counts based on longitudinal speed.
            // More sub-steps at low speed improves accuracy; fewer at high speed improves performance.
            struct SetVehicleLongitudinalSpeedSubSteps : Command
            {
                static constexpr auto command = PhysXExtension::SetVehicleLongitudinalSpeedSubSteps;

                float longitudinalSpeed;
                uint32_t belowSpeedSubsteps;
                uint32_t aboveSpeedSubsteps;
            };

            // Sets the wheel center offset and force application point offset relative to the vehicle's center of mass.
            struct SetVehicleWheelComOffset : Command
            {
                static constexpr auto command = PhysXExtension::SetVehicleWheelComOffset;

                Physics::Vec3 wheelCentreOffset;
                Physics::Vec3 forceAppPointOffset;

                uint32_t wheelId;
            };

            // Retrieves the wheel center offset and force application point offset for a specific wheel.
            struct GetVehicleWheelComOffset : Command
            {
                static constexpr auto command = PhysXExtension::GetVehicleWheelComOffset;

                Physics::Vec3 outWheelCentreOffset;
                Physics::Vec3 outForceAppPointOffset;

                uint32_t wheelId;
            };

            // Enables or disables the suspension expansion limit, which prevents suspension from extending beyond its rest length.
            struct SetVehicleSuspensionExpansionLimitEnabled : Command
            {
                static constexpr auto command = PhysXExtension::SetVehicleSuspensionExpansionLimitEnabled;

                bool enabled;
            };

            // Queries whether the suspension expansion limit is currently enabled.
            struct GetVehicleSuspensionExpansionLimitEnabled : Command
            {
                static constexpr auto command = PhysXExtension::GetVehicleSuspensionExpansionLimitEnabled;

                bool outEnabled;
            };

            // Sets the suspension parameters (spring strength, damper rate, max compression/droop, sprung mass) for a wheel.
            struct SetVehicleWheelSuspensionData : Command
            {
                static constexpr auto command = PhysXExtension::SetVehicleWheelSuspensionData;

                Physics::PhysXExt::WheelSuspensionData suspension;
                uint32_t wheelId;
            };

            // Retrieves the suspension parameters for a specific wheel.
            struct GetVehicleWheelSuspensionData : Command
            {
                static constexpr auto command = PhysXExtension::GetVehicleWheelSuspensionData;

                Physics::PhysXExt::WheelSuspensionData outSuspension;
                uint32_t wheelId;
            };

            // Sets the physical properties (radius, mass, moment of inertia, damping rate) for a wheel.
            struct SetVehicleWheelData : Command
            {
                static constexpr auto command = PhysXExtension::SetVehicleWheelData;

                Physics::PhysXExt::WheelData wheelData;
                uint32_t wheelId;
            };

            // Retrieves the physical properties for a specific wheel.
            struct GetVehicleWheelData : Command
            {
                static constexpr auto command = PhysXExtension::GetVehicleWheelData;

                Physics::PhysXExt::WheelData outWheelData;
                uint32_t wheelId;
            };

            // Retrieves the local-space pose (position and rotation including steering and spin) of a specific wheel.
            struct GetVehicleWheelLocalPose : Command
            {
                static constexpr auto command = PhysXExtension::GetVehicleWheelLocalPose;

                Physics::Pose pose;
                uint32_t wheelId;
            };

            // Sets the rotation speed (radians per second) of a specific wheel.
            struct SetVehicleWheelRotationSpeed : Command
            {
                static constexpr auto command = PhysXExtension::SetVehicleWheelRotationSpeed;

                uint32_t wheelId;
                float speed;
            };

            // Retrieves the current rotation speed (radians per second) of a specific wheel.
            struct GetVehicleWheelRotationSpeed : Command
            {
                static constexpr auto command = PhysXExtension::GetVehicleWheelRotationSpeed;

                uint32_t wheelId;
                float outSpeed;
            };

            // Sets the motor torque applied to a specific wheel.
            struct SetVehicleWheelMotorTorque : Command
            {
                static constexpr auto command = PhysXExtension::SetVehicleWheelMotorTorque;

                uint32_t wheelId;
                float torque;
            };

            // Retrieves the motor torque currently applied to a specific wheel.
            struct GetVehicleWheelMotorTorque : Command
            {
                static constexpr auto command = PhysXExtension::GetVehicleWheelMotorTorque;

                uint32_t wheelId;
                float outTorque;
            };

            // Sets the brake torque applied to a specific wheel.
            struct SetVehicleWheelBrakeTorque : Command
            {
                static constexpr auto command = PhysXExtension::SetVehicleWheelBrakeTorque;

                uint32_t wheelId;
                float brkTorque;
            };

            // Retrieves the brake torque currently applied to a specific wheel.
            struct GetVehicleWheelBrakeTorque : Command
            {
                static constexpr auto command = PhysXExtension::GetVehicleWheelBrakeTorque;

                uint32_t wheelId;
                float outBrkTorque;
            };

            // Sets the steering angle (in radians) for a specific wheel.
            struct SetVehicleWheelSteerAngle : Command
            {
                static constexpr auto command = PhysXExtension::SetVehicleWheelSteerAngle;

                uint32_t wheelId;
                float angle;
            };

            // Retrieves the current steering angle (in radians) for a specific wheel.
            struct GetVehicleWheelSteerAngle : Command
            {
                static constexpr auto command = PhysXExtension::GetVehicleWheelSteerAngle;

                uint32_t wheelId;
                float outAngle;
            };

            // Computes the sprung masses for a set of wheels based on their offsets from the vehicle's center of mass.
            // This distributes the vehicle's weight across the wheels for stable suspension simulation.
            struct CalculateVehicleSprungMasses : Command
            {
                static constexpr auto command = PhysXExtension::CalculateVehicleSprungMasses;

                uint32_t* wheelIds;
                //array of vector3 provided as float stream
                float* wheelOffsets;
                uint32_t wheelCount;
            };
        }
    }
}
