// Unity Physics Backend System Native Plugin API Data Protocol copyright © 2025 Unity Technologies ApS
//
// Licensed under the Unity Companion License for Unity - dependent projects--see[Unity Companion License](http://www.unity3d.com/legal/licenses/Unity_Companion_License).
//
// Unless expressly provided otherwise, the Software under this license is made available strictly on an “AS IS” BASIS WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED.Please review the license for details on these and other terms and conditions.
//
// Please note that the following data protocol types represent the data used internally by Unity's Physics Module in order to communicate with the current SDK Integration. As such these types are subject to rapid change between different Unity versions.

#pragma once
#include <cstdint>
#include <type_traits>

#include "PhysicsCommands.h"
#include "PhysicsBodyTypes.h"
#include "PhysicsPose.h"
#include "PhysicsVecTypes.h"

namespace PhysicsCommands
{
    namespace BodyData
    {
        // Enables or disables body flags (e.g. Gravity, Kinematic, Simulated) on a rigid body within a scene.
        struct SetFlags : Command
        {
            static constexpr auto command = Body::SetFlags;

            Physics::BodyFlags value;
            bool enabled;
        };

        // Retrieves the current body flags for this rigid body.
        struct GetFlags : Command
        {
            static constexpr auto command = Body::GetFlags;

            Physics::BodyFlags value;
        };

        // Sets the mass of the rigid body in kilograms.
        struct SetMass : Command
        {
            static constexpr auto command = Body::SetMass;

            float value;
        };

        // Retrieves the mass of the rigid body in kilograms.
        struct GetMass : Command
        {
            static constexpr auto command = Body::GetMass;

            float value;
        };

        // Sets the linear and angular damping coefficients for the rigid body.
        // Higher values cause the body to lose velocity more quickly.
        struct SetDamping : Command
        {
            static constexpr auto command = Body::SetDamping;

            float linear;
            float angular;
        };

        // Retrieves the linear and angular damping coefficients of the rigid body.
        struct GetDamping : Command
        {
            static constexpr auto command = Body::GetDamping;

            float linear;
            float angular;
        };

        // Sets the world-space position and rotation of the rigid body.
        // When 'awake' is true, the body is woken from sleep if currently sleeping.
        struct SetPose : Command
        {
            static constexpr auto command = Body::SetPose;

            Physics::Pose value;
            bool awake;
        };

        // Retrieves the current world-space position and rotation of the rigid body.
        struct GetPose : Command
        {
            static constexpr auto command = Body::GetPose;

            Physics::Pose value;
        };

        // Attaches a collision shape to this body at the given local pose.
        // When 'forceLocalPose' is true, the pose is treated as relative to the body regardless of body type.
        struct AttachShape : Command
        {
            static constexpr auto command = Body::AttachShape;
            Physics::SDKObjectHandle shape;
            Physics::Pose pose;
            //Temporary solution until we fix local/global pose relative to rb/ab/static body
            bool forceLocalPose;
        };

        // Detaches a collision shape from this body.
        struct DetachShape : Command
        {
            static constexpr auto command = Body::DetachShape;
            Physics::SDKObjectHandle shape;
        };

        // Retrieves the number of collision shapes currently attached to this body.
        struct GetShapeCount : Command
        {
            static constexpr auto command = Body::GetShapeCount;

            size_t value;
        };

        // Retrieves the SDK handles of all shapes attached to this body into the provided buffer.
        // 'written' reports how many handles were actually written.
        struct GetShapes : Command
        {
            static constexpr auto command = Body::GetShapes;

            Physics::SDKObjectHandle* buffer;
            size_t bufferSize;

            size_t written;
        };

        // Retrieves the engine-side user data pointers for all shapes attached to this body.
        // 'written' reports how many pointers were actually written.
        struct GetShapesUserData : Command
        {
            static constexpr auto command = Body::GetShapesUserData;

            void** buffer;
            size_t bufferSize;

            size_t written;
        };

        // Retrieves the engine-side user data pointer associated with this body.
        struct GetUserData : Command
        {
            static constexpr auto command = Body::GetUserData;

            void* value;
        };

        // Sets the collision detection mode (Discrete, Continuous, ContinuousDynamic, ContinuousSpeculative) for this body.
        struct SetCollisionDetectionMode : Command
        {
            static constexpr auto command = Body::SetCollisionDetectionMode;

            Physics::CollisionDetectionMode value;
        };

        // Applies a force to the rigid body using the specified force mode (Force, Impulse, VelocityChange, Acceleration).
        // When 'awake' is true, the body is woken from sleep.
        struct AddForce : Command
        {
            static constexpr auto command = Body::AddForce;

            Physics::Vec3 value;
            Physics::ForceMode mode;
            bool awake;
        };

        // Applies a torque to the rigid body using the specified force mode.
        // When 'awake' is true, the body is woken from sleep.
        struct AddTorque : Command
        {
            static constexpr auto command = Body::AddTorque;

            Physics::Vec3 value;
            Physics::ForceMode mode;
            bool awake;
        };

        // Sets the linear velocity of the rigid body in world space.
        // When 'awake' is true, the body is woken from sleep.
        struct SetLinearVelocity : Command
        {
            static constexpr auto command = Body::SetLinearVelocity;

            Physics::Vec3 value;
            bool awake;
        };

        // Retrieves the current linear velocity of the rigid body in world space.
        struct GetLinearVelocity : Command
        {
            static constexpr auto command = Body::GetLinearVelocity;

            Physics::Vec3 value;
        };

        // Sets the angular velocity of the rigid body in world space (radians per second).
        // When 'awake' is true, the body is woken from sleep.
        struct SetAngularVelocity : Command
        {
            static constexpr auto command = Body::SetAngularVelocity;

            Physics::Vec3 value;
            bool awake;
        };

        // Retrieves the current angular velocity of the rigid body in world space.
        struct GetAngularVelocity : Command
        {
            static constexpr auto command = Body::GetAngularVelocity;

            Physics::Vec3 value;
        };

        // Sets the maximum allowed linear velocity for this body, clamping movement speed.
        struct SetMaxLinearVelocity : Command
        {
            static constexpr auto command = Body::SetMaxLinearVelocity;

            float value;
        };

        // Retrieves the maximum allowed linear velocity for this body.
        struct GetMaxLinearVelocity : Command
        {
            static constexpr auto command = Body::GetMaxLinearVelocity;

            float value;
        };

        // Sets the maximum allowed angular velocity for this body, clamping rotation speed.
        struct SetMaxAngularVelocity : Command
        {
            static constexpr auto command = Body::SetMaxAngularVelocity;

            float value;
        };

        // Retrieves the maximum allowed angular velocity for this body.
        struct GetMaxAngularVelocity : Command
        {
            static constexpr auto command = Body::GetMaxAngularVelocity;

            float value;
        };

        // Sets the maximum depenetration velocity used when resolving overlapping bodies.
        struct SetMaxDepenetrationVelocity : Command
        {
            static constexpr auto command = Body::SetMaxDepenetrationVelocity;

            float value;
        };

        // Retrieves the maximum depenetration velocity for this body.
        struct GetMaxDepenetrationVelocity : Command
        {
            static constexpr auto command = Body::GetMaxDepenetrationVelocity;

            float value;
        };

        // Sets the diagonal elements of the inertia tensor in the body's local space.
        struct SetInertiaTensor : Command
        {
            static constexpr auto command = Body::SetInertiaTensor;

            Physics::Vec3 value;
        };

        // Retrieves the diagonal elements of the inertia tensor in the body's local space.
        struct GetInertiaTensor : Command
        {
            static constexpr auto command = Body::GetInertiaTensor;

            Physics::Vec3 value;
        };

        // Sets the rotation that diagonalizes the inertia tensor (aligning principal axes with local axes).
        struct SetInertiaTensorRotation : Command
        {
            static constexpr auto command = Body::SetInertiaTensorRotation;

            Physics::Vec4 value;
        };

        // Retrieves the rotation that diagonalizes the inertia tensor.
        struct GetInertiaTensorRotation : Command
        {
            static constexpr auto command = Body::GetInertiaTensorRotation;

            Physics::Vec4 value;
        };

        // Sets the center of mass position in the body's local space.
        struct SetLocalCenterOfMass : Command
        {
            static constexpr auto command = Body::SetLocalCenterOfMass;

            Physics::Vec3 value;
        };

        // Retrieves the center of mass position in the body's local space.
        struct GetLocalCenterOfMass : Command
        {
            static constexpr auto command = Body::GetLocalCenterOfMass;

            Physics::Vec3 value;
        };

        // Retrieves the 3x3 world-space inertia tensor matrix for this body, stored as row-major.
        struct GetWorldInertiaTensorMatrix : Command
        {
            static constexpr auto command = Body::GetWorldInertiaTensorMatrix;

            //stored as row major
            float value[9];
        };

        // Recomputes the body's mass, center of mass, and inertia tensor from its attached shapes,
        // applying the provided overrides for mass and inertia values.
        struct RecomputeMassProperties : Command
        {
            static constexpr auto command = Body::RecomputeMassProperties;

            Physics::MassPropertiesOverride overrides;
        };

        // Retrieves the total force accumulated on this body during the current simulation step,
        // scaled by 'deltaTime' for integration. Returns the force vector via 'value'.
        struct GetAccumulatedForce : Command
        {
            static constexpr auto command = Body::GetAccumulatedForce;

            float deltaTime;

            Physics::Vec3 value;
        };

        // Retrieves the total torque accumulated on this body during the current simulation step,
        // scaled by 'deltaTime' for integration.
        struct GetAccumulatedTorque : Command
        {
            static constexpr auto command = Body::GetAccumulatedTorque;

            float deltaTime;

            Physics::Vec3 value;
        };

        // Computes the world-space velocity of a point specified in the body's local space.
        // Accounts for both linear and angular velocity contributions.
        struct GetLocalPointVelocity : Command
        {
            static constexpr auto command = Body::GetLocalPointVelocity;

            Physics::Vec3 localPoint;

            Physics::Vec3 value;
        };

        // Computes the world-space velocity at a given world-space point on the body.
        // Accounts for both linear and angular velocity contributions.
        struct GetWorldPointVelocity : Command
        {
            static constexpr auto command = Body::GetWorldPointVelocity;

            Physics::Vec3 worldPoint;

            Physics::Vec3 value;
        };

        // Finds the closest point on the body's bounding box to a given world-space position.
        // Returns the closest point and squared distance via output fields.
        struct GetClosestWorldPointOnBounds : Command
        {
            static constexpr auto command = Body::GetClosestWorldPointOnBounds;

            Physics::Vec3 worldPosition;

            Physics::Vec3 outClosestWorldPositionOnBounds;
            float outSqrDistance;
        };

        // Sets the target pose for a kinematic body. The body will interpolate toward this target during the next simulation step.
        struct SetKinematicTarget : Command
        {
            static constexpr auto command = Body::SetKinematicTarget;

            Physics::Pose value;
        };

        // Retrieves the current kinematic target pose previously set via SetKinematicTarget.
        struct GetKinematicTarget : Command
        {
            static constexpr auto command = Body::GetKinematicTarget;

            Physics::Pose value;
        };

        // Queries whether this body is currently in a sleeping state (not being simulated due to inactivity).
        struct GetIsSleeping : Command
        {
            static constexpr auto command = Body::GetIsSleeping;

            bool value;
        };

        // Forces this body into the sleeping state, halting simulation until it is woken.
        struct Sleep : Command
        {
            static constexpr auto command = Body::Sleep;
        };

        // Wakes this body from sleep so it resumes active simulation.
        struct WakeUp : Command
        {
            static constexpr auto command = Body::WakeUp;
        };

        // Sets the energy threshold below which this body is allowed to go to sleep.
        struct SetSleepThresholdOverride : Command
        {
            static constexpr auto command = Body::SetSleepThresholdOverride;

            float value;
        };

        // Retrieves the current sleep energy threshold for this body.
        struct GetSleepThresholdOverride : Command
        {
            static constexpr auto command = Body::GetSleepThresholdOverride;

            float value;
        };

        // Sets the number of position and velocity solver iterations used for this body's constraints.
        // Higher values improve accuracy at the cost of performance.
        struct SetSolverIterationsOverride : Command
        {
            static constexpr auto command = Body::SetSolverIterationsOverride;

            uint32_t positionIterations;
            uint32_t velocityIterations;
        };

        // Retrieves the number of position and velocity solver iterations configured for this body.
        struct GetSolverIterationsOverride : Command
        {
            static constexpr auto command = Body::GetSolverIterationsOverride;

            uint32_t positionIterations;
            uint32_t velocityIterations;
        };
    }
}
