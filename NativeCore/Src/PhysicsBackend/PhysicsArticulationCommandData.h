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
#include "PhysicsJointTypes.h"
#include "PhysicsPose.h"
#include "PhysicsVecTypes.h"

namespace Physics
{
    struct FloatBuffer
    {
        const float* data;
        size_t size;
        bool sdkOwnedMemory;
    };

    enum class ArticulationMotorType
    {
        Force = 0,
        Acceleration = 1,
        Target = 2,
        Velocity = 3
    };

    struct ArticulationMotorConfiguration
    {
        float lowerLimit;
        float upperLimit;
        float stiffness;
        float damping;
        float forceLimit;
        float target;
        float targetVelocity;
        ArticulationMotorType type;

        constexpr ArticulationMotorConfiguration()
            : lowerLimit(0),
            upperLimit(0),
            stiffness(0),
            damping(0),
            forceLimit(std::numeric_limits<float>::max()),
            target(0),
            targetVelocity(0),
            type(ArticulationMotorType::Force) {}

        bool operator==(const ArticulationMotorConfiguration& rhs) const
        {
            return lowerLimit == rhs.lowerLimit &&
                   upperLimit == rhs.upperLimit &&
                   stiffness == rhs.stiffness &&
                   damping == rhs.damping &&
                   forceLimit == rhs.forceLimit &&
                   target == rhs.target &&
                   targetVelocity == rhs.targetVelocity &&
                   type == rhs.type;
        }
    };

    struct ReducedSpaceCoordinateStorage
    {
        Physics::Vec3 coordinate;
        int dofCount;

        constexpr ReducedSpaceCoordinateStorage()
            : coordinate{ 0,0,0 }
            , dofCount(0){}
    };
}

namespace PhysicsCommands
{
    // Commands for configuring the joint that connects an articulation link to its parent link.
    // These operate on the joint associated with the link identified by Context::body.
    namespace ArticulationJointData
    {
        // Sets the joint type (Fixed, Prismatic, Hinge, BallAndSocket) for an articulation link's joint.
        struct SetType : Command
        {
            static constexpr auto command = Articulation::SetLinkJointType;

            Physics::JointType value;
        };

        // Retrieves the joint type of an articulation link's joint.
        struct GetType : Command
        {
            static constexpr auto command = Articulation::GetLinkJointType;

            Physics::JointType value;
        };

        // Sets the lock state (Locked, Limited, or Free) for a specific axis on the articulation link joint.
        struct SetAxisLock : Command
        {
            static constexpr auto command = Articulation::SetLinkJointAxisLock;

            Physics::ConstraintAxis axis;
            Physics::JointDofLock value;
        };

        // Sets the angular or linear limit range for a specific axis on the articulation link joint.
        struct SetLimit : Command
        {
            static constexpr auto command = Articulation::SetLinkJointLimit;

            //when using a rotational axis the values are in radians
            Physics::ConstraintAxis axis;
            float min;
            float max;
        };

        // Sets the local-space anchor pose on the parent or child side of the articulation link joint.
        struct SetActorLocalPose : Command
        {
            static constexpr auto command = Articulation::SetLinkJointActorLocalPose;

            Physics::PoseTarget target;
            Physics::Pose value;
        };

        // Configures the motor drive for a specific axis on the articulation link joint,
        // including stiffness, damping, force limit, target, target velocity, and drive type.
        struct SetAxisMotorConfiguration : Command
        {
            static constexpr auto command = Articulation::SetLinkJointAxisMotorConfiguration;

            Physics::ConstraintAxis axis;
            Physics::ArticulationMotorConfiguration value;
        };

        // Sets the friction coefficient for the articulation link joint.
        struct SetFriction : Command
        {
            static constexpr auto command = Articulation::SetLinkJointFriction;

            float value;
        };

        // Sets the joint position in reduced-space coordinates (generalized coordinates for each DOF).
        struct SetPosition : Command
        {
            static constexpr auto command = Articulation::SetLinkJointPosition;

            Physics::ReducedSpaceCoordinateStorage value;
        };

        // Retrieves the joint position in reduced-space coordinates.
        struct GetPosition : Command
        {
            static constexpr auto command = Articulation::GetLinkJointPosition;

            Physics::ReducedSpaceCoordinateStorage value;
        };

        // Sets the joint velocity in reduced-space coordinates.
        struct SetVelocity : Command
        {
            static constexpr auto command = Articulation::SetLinkJointVelocity;

            Physics::ReducedSpaceCoordinateStorage value;
        };

        // Retrieves the joint velocity in reduced-space coordinates.
        struct GetVelocity : Command
        {
            static constexpr auto command = Articulation::GetLinkJointVelocity;

            Physics::ReducedSpaceCoordinateStorage value;
        };

        // Sets the joint acceleration in reduced-space coordinates.
        struct SetAcceleration : Command
        {
            static constexpr auto command = Articulation::SetLinkJointAcceleration;

            Physics::ReducedSpaceCoordinateStorage value;
        };

        // Retrieves the joint acceleration in reduced-space coordinates.
        struct GetAcceleration : Command
        {
            static constexpr auto command = Articulation::GetLinkJointAcceleration;

            Physics::ReducedSpaceCoordinateStorage value;
        };

        // Sets the joint force in reduced-space coordinates.
        struct SetForce : Command
        {
            static constexpr auto command = Articulation::SetLinkJointForce;

            Physics::ReducedSpaceCoordinateStorage value;
        };

        // Retrieves the joint force in reduced-space coordinates.
        struct GetForce : Command
        {
            static constexpr auto command = Articulation::GetLinkJointForce;

            Physics::ReducedSpaceCoordinateStorage value;
        };

        // Retrieves the force applied by the joint motor drive in reduced-space coordinates.
        struct GetMotorForce : Command
        {
            static constexpr auto command = Articulation::GetLinkJointMotorForce;

            Physics::ReducedSpaceCoordinateStorage value;

        };

        // Computes the joint forces required to produce the given acceleration in reduced-space coordinates.
        // 'acceleration' is the desired acceleration; 'value' receives the computed forces.
        struct GetForcesForAcceleration : Command
        {
            static constexpr auto command = Articulation::GetLinkJointForcesForAcceleration;

            Physics::ReducedSpaceCoordinateStorage acceleration;
            Physics::ReducedSpaceCoordinateStorage value;
        };

        // Sets the maximum velocity for the articulation link joint.
        struct SetMaxVelocity : Command
        {
            static constexpr auto command = Articulation::SetLinkJointMaxVelocity;

            float value;
        };

        // Retrieves the maximum velocity for the articulation link joint.
        struct GetMaxVelocity : Command
        {
            static constexpr auto command = Articulation::GetLinkJointMaxVelocity;

            float value;
        };

        // Retrieves the number of degrees of freedom for the articulation link joint.
        struct GetDofCount : Command
        {
            static constexpr auto command = Articulation::GetLinkJointDofCount;

            uint32_t value;
        };
    }

    // Commands for manipulating the rigid body properties of an individual articulation link.
    // These mirror the BodyData commands but operate on a link within an articulation hierarchy.
    // The link is identified by Context::body.
    namespace ArticulationLinkData
    {
        // Retrieves the index of this link within the articulation's link array.
        struct GetLinkIndex : Command
        {
            static constexpr auto command = Articulation::GetLinkIndex;

            int value;
        };

        // Retrieves the SDK handle for the joint connecting this link to its parent link.
        struct GetLinkJoint : Command
        {
            static constexpr auto command = Articulation::GetLinkJoint;

            Physics::SDKObjectHandle value;
        };

        // Enables or disables body flags (only the Gravity flag is valid for this operation) on this articulation link.
        struct SetFlags : Command
        {
            static constexpr auto command = Articulation::SetLinkFlags;

            Physics::BodyFlags value;
            bool enabled;
        };

        // Retrieves the current body flags for this articulation link.
        struct GetFlags : Command
        {
            static constexpr auto command = Articulation::GetLinkFlags;

            Physics::BodyFlags value;
        };

        // Sets the mass of this articulation link in kilograms.
        struct SetMass : Command
        {
            static constexpr auto command = Articulation::SetLinkMass;

            float value;
        };

        // Retrieves the mass of this articulation link in kilograms.
        struct GetMass : Command
        {
            static constexpr auto command = Articulation::GetLinkMass;

            float value;
        };

        // Sets the linear and angular damping coefficients for this articulation link.
        struct SetDamping : Command
        {
            static constexpr auto command = Articulation::SetLinkDamping;

            float linear;
            float angular;
        };

        // Retrieves the linear and angular damping coefficients for this articulation link.
        struct GetDamping : Command
        {
            static constexpr auto command = Articulation::GetLinkDamping;

            float linear;
            float angular;
        };

        // Sets the world-space position and rotation of this articulation link.
        // When 'awake' is true, the articulation is woken from sleep.
        struct SetPose : Command
        {
            static constexpr auto command = Articulation::SetLinkPose;

            Physics::Pose value;
            bool awake;
        };

        // Retrieves the current world-space position and rotation of this articulation link.
        struct GetPose : Command
        {
            static constexpr auto command = Articulation::GetLinkPose;

            Physics::Pose value;
        };

        // Attaches a collision shape to this articulation link at the given local pose.
        struct AttachShape : Command
        {
            static constexpr auto command = Articulation::AttachShapeToLink;

            Physics::SDKObjectHandle shape;
            Physics::Pose pose;
        };

        // Detaches a collision shape from this articulation link.
        struct DetachShape : Command
        {
            static constexpr auto command = Articulation::DetachShapeFromLink;

            Physics::SDKObjectHandle shape;
        };

        // Retrieves the number of collision shapes attached to this articulation link.
        struct GetShapeCount : Command
        {
            static constexpr auto command = Articulation::GetLinkShapeCount;

            size_t value;
        };

        // Retrieves the engine-side user data pointers for all shapes attached to this articulation link.
        // 'written' reports how many pointers were actually written.
        struct GetShapesUserData : Command
        {
            static constexpr auto command = Articulation::GetLinkShapesUserData;

            void** buffer;
            size_t bufferSize;

            size_t written;
        };

        // Retrieves the engine-side user data pointer associated with this articulation link.
        struct GetUserData : Command
        {
            static constexpr auto command = Articulation::GetLinkUserData;

            void* value;
        };

        // Sets the collision detection mode for this articulation link.
        struct SetCollisionDetectionMode : Command
        {
            static constexpr auto command = Articulation::SetLinkCollisionDetectionMode;

            Physics::CollisionDetectionMode value;
        };

        // Applies a force to this articulation link using the specified force mode.
        // When 'awake' is true, the articulation is woken from sleep.
        struct AddForce : Command
        {
            static constexpr auto command = Articulation::AddForceToLink;

            Physics::Vec3 value;
            Physics::ForceMode mode;
            bool awake;
        };

        // Applies a torque to this articulation link using the specified force mode.
        // When 'awake' is true, the articulation is woken from sleep.
        struct AddTorque : Command
        {
            static constexpr auto command = Articulation::AddTorqueToLink;

            Physics::Vec3 value;
            Physics::ForceMode mode;
            bool awake;
        };

        // Sets the linear velocity of this articulation link in world space.
        // When 'awake' is true, the articulation is woken from sleep.
        struct SetLinearVelocity : Command
        {
            static constexpr auto command = Articulation::SetLinkLinearVelocity;

            Physics::Vec3 value;
            bool awake;
        };

        // Retrieves the current linear velocity of this articulation link in world space.
        struct GetLinearVelocity : Command
        {
            static constexpr auto command = Articulation::GetLinkLinearVelocity;

            Physics::Vec3 value;
        };

        // Sets the angular velocity of this articulation link in world space (radians per second).
        // When 'awake' is true, the articulation is woken from sleep.
        struct SetAngularVelocity : Command
        {
            static constexpr auto command = Articulation::SetLinkAngularVelocity;

            Physics::Vec3 value;
            bool awake;
        };

        // Retrieves the current angular velocity of this articulation link in world space.
        struct GetAngularVelocity : Command
        {
            static constexpr auto command = Articulation::GetLinkAngularVelocity;

            Physics::Vec3 value;
        };

        // Sets the maximum allowed linear velocity for this articulation link.
        struct SetMaxLinearVelocity : Command
        {
            static constexpr auto command = Articulation::SetLinkMaxLinearVelocity;

            float value;
        };

        // Retrieves the maximum allowed linear velocity for this articulation link.
        struct GetMaxLinearVelocity : Command
        {
            static constexpr auto command = Articulation::GetLinkMaxLinearVelocity;

            float value;
        };

        // Sets the maximum allowed angular velocity for this articulation link.
        struct SetMaxAngularVelocity : Command
        {
            static constexpr auto command = Articulation::SetLinkMaxAngularVelocity;

            float value;
        };

        // Retrieves the maximum allowed angular velocity for this articulation link.
        struct GetMaxAngularVelocity : Command
        {
            static constexpr auto command = Articulation::GetLinkMaxAngularVelocity;

            float value;
        };

        // Sets the maximum depenetration velocity for this articulation link.
        struct SetMaxDepenetrationVelocity : Command
        {
            static constexpr auto command = Articulation::SetLinkMaxDepenetrationVelocity;

            float value;
        };

        // Retrieves the maximum depenetration velocity for this articulation link.
        struct GetMaxDepenetrationVelocity : Command
        {
            static constexpr auto command = Articulation::GetLinkMaxDepenetrationVelocity;

            float value;
        };

        // Sets the diagonal elements of the inertia tensor for this articulation link in local space.
        struct SetInertiaTensor : Command
        {
            static constexpr auto command = Articulation::SetLinkInertiaTensor;

            Physics::Vec3 value;
        };

        // Retrieves the diagonal elements of the inertia tensor for this articulation link in local space.
        struct GetInertiaTensor : Command
        {
            static constexpr auto command = Articulation::GetLinkInertiaTensor;

            Physics::Vec3 value;
        };

        // Sets the rotation that diagonalizes the inertia tensor for this articulation link.
        struct SetInertiaTensorRotation : Command
        {
            static constexpr auto command = Articulation::SetLinkInertiaTensorRotation;

            Physics::Vec4 value;
        };

        // Retrieves the rotation that diagonalizes the inertia tensor for this articulation link.
        struct GetInertiaTensorRotation : Command
        {
            static constexpr auto command = Articulation::GetLinkInertiaTensorRotation;

            Physics::Vec4 value;
        };

        // Sets the center of mass position in this articulation link's local space.
        struct SetLocalCenterOfMass : Command
        {
            static constexpr auto command = Articulation::SetLinkLocalCenterOfMass;

            Physics::Vec3 value;
        };

        // Retrieves the center of mass position in this articulation link's local space.
        struct GetLocalCenterOfMass : Command
        {
            static constexpr auto command = Articulation::GetLinkLocalCenterOfMass;

            Physics::Vec3 value;
        };

        // Retrieves the 3x3 world-space inertia tensor matrix for this articulation link, stored as row-major.
        struct GetWorldInertiaTensorMatrix : Command
        {
            static constexpr auto command = Articulation::GetLinkWorldInertiaTensorMatrix;

            //stored as row major
            float value[9];
        };

        // Recomputes mass, center of mass, and inertia tensor for this articulation link from its attached shapes,
        // applying the provided overrides.
        struct RecomputeMassProperties : Command
        {
            static constexpr auto command = Articulation::RecomputeLinkMassProperties;

            Physics::MassPropertiesOverride overrides;
        };

        // Retrieves the total force accumulated on this articulation link during the current simulation step.
        struct GetAccumulatedForce : Command
        {
            static constexpr auto command = Articulation::GetLinkAccumulatedForce;

            float deltaTime;

            Physics::Vec3 value;
        };

        // Retrieves the total torque accumulated on this articulation link during the current simulation step.
        struct GetAccumulatedTorque : Command
        {
            static constexpr auto command = Articulation::GetLinkAccumulatedTorque;

            float deltaTime;

            Physics::Vec3 value;
        };

        // Computes the world-space velocity of a point specified in this articulation link's local space.
        struct GetLocalPointVelocity : Command
        {
            static constexpr auto command = Articulation::GetLinkLocalPointVelocity;

            Physics::Vec3 localPoint;

            Physics::Vec3 value;
        };

        // Computes the world-space velocity at a given world-space point on this articulation link.
        struct GetWorldPointVelocity : Command
        {
            static constexpr auto command = Articulation::GetLinkWorldPointVelocity;

            Physics::Vec3 worldPoint;

            Physics::Vec3 value;
        };

        // Finds the closest point on this articulation link's surface to a given world-space position.
        // Uses 'enginePose' as a reference frame. Returns the closest point and squared distance.
        struct GetClosestWorldPoint : Command
        {
            static constexpr auto command = Articulation::GetLinkClosestWorldPoint;

            Physics::Pose enginePose;
            Physics::Vec3 worldPosition;

            Physics::Vec3 outClosestWorldPosition;
            float outSqrDistance;
        };
    }

    // Commands for managing the articulation as a whole: creating/destroying links, querying the link hierarchy,
    // managing sleep state, solver iterations, and bulk operations on all joint positions/velocities/forces.
    namespace ArticulationData
    {
        // Creates a new link in the articulation as a child of 'parentLink' at the given pose.
        // Pass null for 'parentLink' to create the root link. Returns the new link handle via 'outNewLink'.
        struct CreateLink : Command
        {
            static constexpr auto command = Articulation::CreateLink;

            Physics::SDKObjectHandle parentLink;
            void* userData;
            Physics::Pose pose;

            Physics::SDKObjectHandle outNewLink;
        };

        // Destroys a link and removes it from the articulation hierarchy.
        struct DestroyLink : Command
        {
            static constexpr auto command = Articulation::DestroyLink;

            Physics::SDKObjectHandle value;
        };

        // Retrieves the total number of links in this articulation.
        struct GetLinkCount : Command
        {
            static constexpr auto command = Articulation::GetLinkCount;

            size_t value;
        };

        // Retrieves the engine-side user data pointers for all links in this articulation.
        // When 'sortedByIndex' is true, pointers are ordered by link index.
        // 'written' reports how many pointers were actually written.
        struct GetLinksUserData : Command
        {
            static constexpr auto command = Articulation::GetLinksUserData;

            bool sortedByIndex;

            void** buffer;
            size_t bufferSize;

            size_t written;
        };

        // Retrieves the SDK handle of the parent link for the given link.
        struct GetLinkParent : Command
        {
            static constexpr auto command = Articulation::GetLinkParent;

            Physics::SDKObjectHandle link;

            Physics::SDKObjectHandle value;
        };

        // Retrieves the engine-side user data pointer associated with the parent of the given link.
        struct GetLinkParentUserData : Command
        {
            static constexpr auto command = Articulation::GetLinkParentUserData;

            Physics::SDKObjectHandle link;

            void* value;
        };

        // Retrieves the number of child links for a given link in the articulation hierarchy.
        struct GetLinkChildCount : Command
        {
            static constexpr auto command = Articulation::GetLinkChildCount;

            Physics::SDKObjectHandle link;

            size_t value;
        };

        // Retrieves the engine-side user data pointers for all children of a given link.
        // 'written' reports how many pointers were actually written.
        struct GetLinkChildrenUserData : Command
        {
            static constexpr auto command = Articulation::GetLinkChildrenUserData;

            Physics::SDKObjectHandle link;

            void** buffer;
            size_t bufferSize;

            size_t written;
        };

        // Enables or disables articulation-level flags (e.g. DriveLimitsAsForces, ImmovableRoot, Simulated).
        struct SetFlags : Command
        {
            static constexpr auto command = Articulation::SetFlags;

            Physics::ArticulationFlags value;
            bool enabled;
        };

        // Retrieves the current articulation-level flags.
        struct GetFlags : Command
        {
            static constexpr auto command = Articulation::GetFlags;

            Physics::ArticulationFlags value;
        };

        // Retrieves the engine-side user data pointer associated with this articulation.
        struct GetUserData : Command
        {
            static constexpr auto command = Articulation::GetUserData;

            void* value;
        };

        // Queries whether this articulation is currently in a sleeping state.
        struct GetIsSleeping : Command
        {
            static constexpr auto command = Articulation::GetIsSleeping;

            bool value;
        };

        // Forces this articulation into the sleeping state.
        struct Sleep : Command
        {
            static constexpr auto command = Articulation::Sleep;
        };

        // Wakes this articulation from sleep so it resumes active simulation.
        struct WakeUp : Command
        {
            static constexpr auto command = Articulation::WakeUp;
        };

        // Sets the energy threshold below which this articulation is allowed to go to sleep.
        struct SetSleepThresholdOverride : Command
        {
            static constexpr auto command = Articulation::SetSleepThresholdOverride;

            float value;
        };

        // Retrieves the current sleep energy threshold for this articulation.
        struct GetSleepThresholdOverride : Command
        {
            static constexpr auto command = Articulation::GetSleepThresholdOverride;

            float value;
        };

        // Sets the number of position and velocity solver iterations for this articulation's constraints.
        struct SetSolverIterationsOverride : Command
        {
            static constexpr auto command = Articulation::SetSolverIterationsOverride;

            uint32_t positionIterations;
            uint32_t velocityIterations;
        };

        // Retrieves the number of position and velocity solver iterations for this articulation.
        struct GetSolverIterationsOverride : Command
        {
            static constexpr auto command = Articulation::GetSolverIterationsOverride;

            uint32_t positionIterations;
            uint32_t velocityIterations;
        };

        // Retrieves the dense Jacobian matrix for this articulation as a flat float buffer.
        // 'rowCount' and 'colCount' describe the matrix dimensions.
        struct GetDenseJacobianAsBuffer : Command
        {
            static constexpr auto command = Articulation::GetDenseJacobianAsFloatBuffer;

            uint32_t rowCount;
            uint32_t colCount;
            Physics::FloatBuffer value;
        };

        // Sets the generalized joint positions for all joints in the articulation as a flat float buffer.
        struct SetJointsPositions : Command
        {
            static constexpr auto command = Articulation::SetJointsPositions;

            Physics::FloatBuffer value;
        };

        // Retrieves the generalized joint positions for all joints in the articulation as a flat float buffer.
        struct GetJointsPositions : Command
        {
            static constexpr auto command = Articulation::GetJointsPositions;

            Physics::FloatBuffer value;
        };

        // Sets the generalized joint velocities for all joints in the articulation as a flat float buffer.
        struct SetJointsVelocities : Command
        {
            static constexpr auto command = Articulation::SetJointsVelocities;

            Physics::FloatBuffer value;
        };

        // Retrieves the generalized joint velocities for all joints in the articulation as a flat float buffer.
        struct GetJointsVelocities : Command
        {
            static constexpr auto command = Articulation::GetJointsVelocities;

            Physics::FloatBuffer value;
        };

        // Sets the generalized joint accelerations for all joints in the articulation as a flat float buffer.
        struct SetJointsAccelerations : Command
        {
            static constexpr auto command = Articulation::SetJointsAccelerations;

            Physics::FloatBuffer value;
        };

        // Retrieves the generalized joint accelerations for all joints in the articulation as a flat float buffer.
        struct GetJointsAccelerations : Command
        {
            static constexpr auto command = Articulation::GetJointsAccelerations;

            Physics::FloatBuffer value;
        };

        // Sets the generalized joint forces for all joints in the articulation as a flat float buffer.
        struct SetJointsForces : Command
        {
            static constexpr auto command = Articulation::SetJointsForces;

            Physics::FloatBuffer value;
        };

        // Retrieves the generalized joint forces for all joints in the articulation as a flat float buffer.
        struct GetJointsForces : Command
        {
            static constexpr auto command = Articulation::GetJointsForces;

            Physics::FloatBuffer value;
        };

        // Retrieves the joint drive forces for all joints in the articulation as a flat float buffer.
        struct GetJointsDriveForces : Command
        {
            static constexpr auto command = Articulation::GetJointsDriveForces;

            Physics::FloatBuffer value;
        };

        // Retrieves the gravity compensation forces for all joints in the articulation as a flat float buffer.
        struct GetJointsGravityForces : Command
        {
            static constexpr auto command = Articulation::GetJointsGravityForces;

            Physics::FloatBuffer value;
        };

        // Retrieves the Coriolis and centrifugal forces for all joints in the articulation as a flat float buffer.
        struct GetJointsCoriolisCentrifugalForces : Command
        {
            static constexpr auto command = Articulation::GetJointsCoriolisCentrifugalForces;

            Physics::FloatBuffer value;
        };

        // Retrieves the external forces acting on all joints in the articulation as a flat float buffer.
        // 'deltaTime' is used for force integration scaling.
        struct GetJointsExternalForces : Command
        {
            static constexpr auto command = Articulation::GetJointsExternalForces;

            Physics::FloatBuffer value;
            float deltaTime;
        };

        // Releases an SDK-owned float buffer previously returned by a bulk joint data query
        // (e.g. GetJointsPositions, GetDenseJacobianAsBuffer) to free backend memory.
        struct ReleaseDataBuffer : Command
        {
            static constexpr auto command = Articulation::ReleaseDataBuffer;

            Physics::FloatBuffer value;
        };
    }
}
