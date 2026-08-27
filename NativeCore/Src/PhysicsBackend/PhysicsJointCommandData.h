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
#include "PhysicsJointTypes.h"
#include "PhysicsActorDescriptor.h"
#include "PhysicsPose.h"
#include "PhysicsVecTypes.h"

namespace PhysicsCommands
{
    namespace JointData
    {
        // Enables or disables specific joint flags (e.g. preprocessing, projection, collision reporting).
        struct SetFlags : Command
        {
            static constexpr auto command = Joint::SetFlags;

            Physics::JointFlags value;
            bool enabled;
        };

        // Retrieves the current flags set on this joint.
        struct GetFlags : Command
        {
            static constexpr auto command = Joint::GetFlags;

            Physics::JointFlags value;
        };

        // Retrieves the engine-side user data pointer associated with this joint.
        struct GetUserData : Command
        {
            static constexpr auto command = Joint::GetUserData;

            void* value;
        };

        // Sets the local-space anchor pose for the parent or child actor of this joint.
        // 'target' selects which actor (Parent or Child) the pose applies to.
        struct SetActorLocalPose : Command
        {
            static constexpr auto command = Joint::SetActorLocalPose;

            Physics::PoseTarget target;
            Physics::Pose value;
        };

        // Retrieves the local-space anchor pose for the parent or child actor of this joint.
        struct GetActorLocalPose : Command
        {
            static constexpr auto command = Joint::GetActorLocalPose;

            Physics::PoseTarget target;
            Physics::Pose value;
        };

        // Sets the maximum force and torque thresholds at which this joint will break.
        struct SetBreakForce : Command
        {
            static constexpr auto command = Joint::SetBreakForce;

            float force;
            float torque;
        };

        // Queries whether the underlying constraint is still valid (e.g. not broken and actors still exist).
        struct GetIsConstraintValid : Command
        {
            static constexpr auto command = Joint::GetIsConstraintValid;

            bool value;
        };

        // Retrieves the linear and angular forces currently applied by this joint's constraint solver.
        struct GetForces : Command
        {
            static constexpr auto command = Joint::GetForces;

            Physics::Vec3 linear;
            Physics::Vec3 angular;
        };

        // Sets the inverse mass and inertia scale for one side of the joint, allowing asymmetric constraint response.
        // 'target' selects which actor (Parent or Child) the scale applies to.
        struct SetInvMassAndInertiaScale : Command
        {
            static constexpr auto command = Joint::SetInvMassAndInertiaScale;

            Physics::PoseTarget target;
            float value;
        };

        // Reassigns the two actors connected by this joint. Each entry includes the actor handle, user data, and type.
        struct SetActors : Command
        {
            static constexpr auto command = Joint::SetActors;

            Physics::ActorDescriptor actors[2];
        };

        // Retrieves the actor descriptors for both actors connected by this joint.
        struct GetActors : Command
        {
            static constexpr auto command = Joint::GetActors;

            Physics::ActorDescriptor actors[2];
        };

        // Wakes up both actors connected by this joint so that the constraint is evaluated in the next simulation step.
        struct WakeUpActors : Command
        {
            static constexpr auto command = Joint::WakeUpActors;
        };

        // Sets the spring and damping parameters for a 6-DOF joint axis, enabling soft limits.
        struct Set6DofSpringLimit : Command
        {
            static constexpr auto command = Joint::Set6DofSpringLimit;

            Physics::ConstraintAxis axis;
            float spring;
            float damping;
        };

        // Sets the angular or linear limit range for a 6-DOF joint axis, including bounciness and contact distance.
        struct Set6DofLimit : Command
        {
            static constexpr auto command = Joint::Set6DofLimit;

            Physics::ConstraintAxis axis;
            float min;
            float max;
            float bounciness;
            float bounceMinVelocity;
            float contactDistance;
        };

        // Retrieves the angular or linear limit range for a 6-DOF joint axis, including bounciness and contact distance.
        struct Get6DofLimit : Command
        {
            static constexpr auto command = Joint::Get6DofLimit;

            Physics::ConstraintAxis axis;
            float min;
            float max;
            float bounciness;
            float bounceMinVelocity;
            float contactDistance;
        };

        // Locks, limits, or frees a specific axis on a 6-DOF joint.
        struct Set6DofAxisLock : Command
        {
            static constexpr auto command = Joint::Set6DofAxisLock;

            Physics::ConstraintAxis axis;
            Physics::JointDofLock value;
        };

        // Configures the motor drive parameters (spring, damper, max force) for a specific 6-DOF joint axis.
        struct Set6DofAxisMotorConfiguration : Command
        {
            static constexpr auto command = Joint::Set6DofAxisMotorConfiguration;

            Physics::ConstraintAxis axis;
            float positionSpring;
            float positionDamper;
            float maximumForce;
            int usesAcceleration;
        };

        // Sets the target position for the 6-DOF joint's linear motor drives.
        struct Set6DofMotorTargetPosition : Command
        {
            static constexpr auto command = Joint::Set6DofMotorTargetPosition;

            Physics::Vec3 value;
        };

        // Sets the target rotation (as a quaternion) for the 6-DOF joint's angular motor drives.
        struct Set6DofMotorTargetRotation : Command
        {
            static constexpr auto command = Joint::Set6DofMotorTargetRotation;

            Physics::Vec4 value;
        };

        // Sets the target linear velocity for the 6-DOF joint's linear motor drives.
        struct Set6DofMotorTargetLinearVelocity : Command
        {
            static constexpr auto command = Joint::Set6DofMotorTargetLinearVelocity;

            Physics::Vec3 value;
        };

        // Sets the target angular velocity for the 6-DOF joint's angular motor drives.
        struct Set6DofMotorTargetAngularVelocity : Command
        {
            static constexpr auto command = Joint::Set6DofMotorTargetAngularVelocity;

            Physics::Vec3 value;
        };

        // Sets the linear and angular tolerance for the 6-DOF joint's projection (error correction).
        struct Set6DofProjectionTolerance : Command
        {
            static constexpr auto command = Joint::Set6DofProjectionTolerance;

            float linear;
            float angular;
        };

        // Sets the minimum and maximum distance limits for a distance joint.
        struct SetDistanceLimit : Command
        {
            static constexpr auto command = Joint::SetDistanceLimit;

            float min;
            float max;
        };

        // Sets the spring and damping parameters for a distance joint's limit, enabling a soft distance constraint.
        struct SetDistanceSpringLimit : Command
        {
            static constexpr auto command = Joint::SetDistanceSpringLimit;

            float spring;
            float damping;
        };

        // Sets the allowable error tolerance for a distance joint before the constraint is enforced.
        struct SetDistanceErrorTolerance : Command
        {
            static constexpr auto command = Joint::SetDistanceErrorTolerance;

            float value;
        };

        // Enables or disables the angular limit on a hinge (revolute) joint.
        struct SetHingeLimitEnabled : Command
        {
            static constexpr auto command = Joint::SetHingeLimitEnabled;
            bool value;
        };

        // Configures the motor for a hinge joint, setting max force and free-spin behavior.
        // When 'freeSpin' is true, the motor does not apply force if current velocity exceeds the target.
        struct SetHingeMotorConfiguration : Command
        {
            static constexpr auto command = Joint::SetHingeMotorConfiguration;

            float maxForce;
            // Instructs the motor to not add any force if the current velocity is greater than the target velocity
            bool freeSpin;
        };

        // Sets the target angular velocity for the hinge joint motor.
        struct SetHingeMotorTargetVelocity : Command
        {
            static constexpr auto command = Joint::SetHingeMotorTargetVelocity;

            float value;
        };

        // Sets the angular limit range for a hinge joint in radians, including bounciness and contact distance parameters.
        struct SetHingeLimit : Command
        {
            static constexpr auto command = Joint::SetHingeLimit;

            float min;
            float max;
            float bounciness;
            float bounceMinVelocity;
            float contactDistance;
        };
    }
}
