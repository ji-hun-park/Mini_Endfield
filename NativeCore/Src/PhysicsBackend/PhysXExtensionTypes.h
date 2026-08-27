// Unity Physics Backend System Native Plugin API Data Protocol copyright © 2025 Unity Technologies ApS
//
// Licensed under the Unity Companion License for Unity - dependent projects--see[Unity Companion License](http://www.unity3d.com/legal/licenses/Unity_Companion_License).
//
// Unless expressly provided otherwise, the Software under this license is made available strictly on an “AS IS” BASIS WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED.Please review the license for details on these and other terms and conditions.
//
// Please note that the following data protocol types represent the data used internally by Unity's Physics Module in order to communicate with the current SDK Integration. As such these types are subject to rapid change between different Unity versions.
#pragma once
#include <cstdint>
#include "PhysicsVecTypes.h"

namespace Physics
{
    namespace PhysXExt
    {
        //matches PX_MAX_NB_WHEELS (20) defined in PxVehicleSdk.h
        constexpr uint32_t kMaxVehicleWheelsCount = 20;

        //default number of vehicles to be allocated in a scene
        constexpr uint32_t kDefaultVehicleStorage = 256;

        //invalid wheel x vehicle ids
        constexpr uint32_t kInvalidVehicleId = 0xffffffff;
        constexpr uint32_t kInvalidWheelId = 0xffffffff;

        // Opaque wrapper around physx::PxGeometryHolder. Stores the PhysX geometry type as the first int
        // and the remaining bytes are opaque SDK data. Used by immediate mode collision queries.
        // The memory layout matches PxConvexMeshGeometry (the largest sub-type) to ensure all
        // geometry types fit within the 48-byte blob.
        //
        // PxTriangleMeshLayout (64bit):
        // [00...03] -- PxGeometryType
        // [04...31] -- PxMeshScale
        // [32...39] -- PxConvexMesh ptr
        // [40...43] -- PxConvexMeshGeometryFlag + 3 byte padding
        // [44...47] -- 4 byte padding
        struct GeometryHolder
        {
            constexpr static int kInvalidType = -1;
            int data[12];

            GeometryHolder() { data[0] = kInvalidType; };
        };

        // Opaque 28-byte blob matching the layout of physx::PxTransform (quaternion + position).
        // Used as input for immediate mode contact generation.
        struct ImmediateTransform { char blob[28]; };     // physx::PxTransform

        // Opaque 64-byte blob matching the layout of Gu::ContactPoint from PhysX internals.
        // Used as output buffer for immediate mode contact generation.
        struct ImmediateContact { char blob[64]; };     // Gu::ContactPoint

        // Describes the tire friction curve for a vehicle wheel in one direction (forward or sideways).
        // The curve is a piecewise function defined by an extremum point and an asymptote point,
        // scaled by a stiffness factor. Used by PhysX vehicle SDK tire simulation.
        class WheelFrictionCurve
        {
        private:
            static constexpr float kDefaultForwardCurveExtremumSlip = 0.4f;
            static constexpr float kDefaultForwardCurveExtremumValue = 1.0f;
            static constexpr float kDefaultForwardCurveAsymptoteSlip = 0.8f;
            static constexpr float kDefaultForwardCurveAsymptoteValue = 0.5f;
            static constexpr float kDefaultForwardCurveStiffness = 1.0f;

            static constexpr float kDefaultSidewaysCurveExtremumSlip = 0.2f;
            static constexpr float kDefaultSidewaysCurveExtremumValue = 1.0f;
            static constexpr float kDefaultSidewaysCurveAsymptoteSlip = 0.5f;
            static constexpr float kDefaultSidewaysCurveAsymptoteValue = 0.75f;
            static constexpr float kDefaultSidewaysCurveStiffness = 1.0f;

        public:

            WheelFrictionCurve() : WheelFrictionCurve(true) {}

            //Wheel friction curve direction if 'isForward' is false then the curve is sideways
            WheelFrictionCurve(bool isForward)
            {
                if(isForward)
                {
                    m_ExtremumSlip = kDefaultForwardCurveExtremumSlip;
                    m_ExtremumValue = kDefaultForwardCurveExtremumValue;
                    m_AsymptoteSlip = kDefaultForwardCurveAsymptoteSlip;
                    m_AsymptoteValue = kDefaultForwardCurveAsymptoteValue;
                    m_Stiffness = kDefaultForwardCurveStiffness;
                }
                else
                {
                    m_ExtremumSlip = kDefaultSidewaysCurveExtremumSlip;
                    m_ExtremumValue = kDefaultSidewaysCurveExtremumValue;
                    m_AsymptoteSlip = kDefaultSidewaysCurveAsymptoteSlip;
                    m_AsymptoteValue = kDefaultSidewaysCurveAsymptoteValue;
                    m_Stiffness = kDefaultSidewaysCurveStiffness;
                }
            }

            inline bool IsValid() const
            {
                return m_ExtremumSlip > 0.0f
                    && m_ExtremumSlip < m_AsymptoteSlip
                    && m_ExtremumValue > 0.0f
                    && m_AsymptoteValue > 0.0f
                    && m_Stiffness > 0.0f;
            }

            inline bool operator!=(const WheelFrictionCurve& other) const
            {
                return m_ExtremumSlip != other.m_ExtremumSlip
                    || m_ExtremumValue != other.m_ExtremumValue
                    || m_AsymptoteSlip != other.m_AsymptoteSlip
                    || m_AsymptoteValue != other.m_AsymptoteValue
                    || m_Stiffness != other.m_Stiffness;
            }

            float m_ExtremumSlip; ///<Extremum Slip. range { 0.001, infinity }
            float m_ExtremumValue; ///<Extremum Value. range { 0.001, infinity }
            float m_AsymptoteSlip; ///<Asymptote Slip. range { 0.001, infinity }
            float m_AsymptoteValue; ///<Asymptote Value. range { 0.001, infinity }
            float m_Stiffness;      ///<Stiffness Factor. range { 0, infinity }
        };

        // Result of a per-wheel raycast/sweep query during vehicle simulation.
        // Contains the ground contact point, surface normal, forward/sideways directions,
        // contact force, slip ratios, suspension compression, steering angle, and user data.
        struct WheelQueryResult
        {
            Physics::Vec3 point;
            Physics::Vec3 normal;
            Physics::Vec3 forwardDir;
            Physics::Vec3 sidewaysDir;
            float force;
            float forwardSlip;
            float sidewaysSlip;
            float suspensionCompression;
            float steerAngle;
            void* userData;
        };

        // Suspension spring parameters for a vehicle wheel.
        struct WheelSuspensionData
        {
            float springStrength;    // Spring force coefficient (N/m)
            float springDamperRate;  // Damping force coefficient (Ns/m)
            float maxCompression;    // Maximum compression distance from rest length
            float maxDroop;          // Maximum extension distance from rest length
            float sprungMass;        // Mass supported by this suspension spring (kg)
        };

        // Physical properties of a vehicle wheel.
        struct WheelData
        {
            float radius;           // Wheel radius in world units
            float mass;             // Wheel mass in kilograms
            float momentOfInertia;  // Moment of inertia around the spin axis (kg*m^2)
            float dampingRate;      // Rotational damping rate (Nm*s/rad)
        };

        // Records a single collision hit between a character controller and a collider during a move.
        // Contains the collider pointer, contact point, surface normal, and the motion vector
        // (direction and distance) of the controller at the time of the hit.
        struct RecordedControllerColliderHit
        {
            void*            collider;
            Physics::Vec3    point;
            Physics::Vec3    normal;
            Physics::Vec3    motionDirection;
            float            motionLength;
        };
    }
}
