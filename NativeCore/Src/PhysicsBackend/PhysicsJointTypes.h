// Unity Physics Backend System Native Plugin API Data Protocol copyright © 2025 Unity Technologies ApS
//
// Licensed under the Unity Companion License for Unity - dependent projects--see[Unity Companion License](http://www.unity3d.com/legal/licenses/Unity_Companion_License).
//
// Unless expressly provided otherwise, the Software under this license is made available strictly on an “AS IS” BASIS WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED.Please review the license for details on these and other terms and conditions.
//
// Please note that the following data protocol types represent the data used internally by Unity's Physics Module in order to communicate with the current SDK Integration. As such these types are subject to rapid change between different Unity versions.

#pragma once

#include <limits>

namespace Physics
{
    // Bitmask flags controlling joint behavior and solver options.
    enum class JointFlags
    {
        None,
        Preprocessing = 1 << 0,        // Enable joint preprocessing for improved stability
        Projection = 1 << 1,           // Enable error projection to correct constraint drift
        CollisionReporting = 1 << 2,   // Enable collision between the two connected bodies
        DriveLimitsAreForces = 1 << 3, // Interpret drive limits as forces rather than impulses
        LegacyLimitRanges = 1 << 4     // Use legacy limit range conventions
    };

    inline constexpr JointFlags operator | (JointFlags lhs, JointFlags rhs)
    {
        using type_t = std::underlying_type_t<JointFlags>;
        return static_cast<JointFlags>(static_cast<type_t>(lhs) | static_cast<type_t>(rhs));
    }

    inline constexpr JointFlags& operator |= (JointFlags& lhs, JointFlags rhs)
    {
        lhs = lhs | rhs;
        return lhs;
    }

    inline constexpr JointFlags operator &(JointFlags lhs, JointFlags rhs)
    {
        using type_t = std::underlying_type_t<JointFlags>;
        return static_cast<JointFlags>(static_cast<type_t>(lhs) & static_cast<type_t>(rhs));
    }

    // Identifies a specific constraint axis for joint limits, locks, and motor drives.
    enum class ConstraintAxis
    {
        AngularX,   // Rotation around the X axis (twist)
        AngularY,   // Rotation around the Y axis (swing 1)
        AngularZ,   // Rotation around the Z axis (swing 2)
        AngularYZ,  // Combined Y+Z swing cone
        LinearX,    // Translation along the X axis
        LinearY,    // Translation along the Y axis
        LinearZ,    // Translation along the Z axis
        Distance,   // Distance constraint (used by distance joints)
        Slerp       // Spherical linear interpolation constraint
    };

    // Enumerates the supported joint types. Values are ordered to match the C# bindings for ArticulationBody.
    //invalid is in the specific value here as we currently bind this enum to C# for articulations
    //and the order there was done as such
    enum class JointType
    {
        Invalid = 4,
        Fixed = 0,         // Rigidly locks two bodies together (no relative motion)
        Prismatic = 1,     // Allows linear sliding along one axis
        Hinge = 2,         // Allows rotation around one axis (revolute joint)
        BallAndSocket = 3, // Allows rotation around all axes but no translation (spherical joint)
        Distance = 5,      // Maintains a distance range between two anchor points
        Dof6 = 6,          // Configurable 6-degree-of-freedom joint with per-axis limits and drives
        Count = 7
    };

    // Controls the degree-of-freedom state for a single joint axis.
    enum class JointDofLock
    {
        Locked,  // Axis is fully constrained (no motion)
        Limited, // Axis is constrained within a limit range
        Free     // Axis is unconstrained
    };

    // Selects which side of a joint (parent or child body) a pose or parameter applies to.
    enum class PoseTarget
    {
        Parent = 0,
        Child = 1
    };

    // Defines the minimum and maximum limit values for a joint axis.
    struct JointLimitRange
    {
        // Unit depends on joint type : radian for hinge and ball & socket, distance unit for the other
        float min = -std::numeric_limits<float>::max();
        float max = std::numeric_limits<float>::max();
    };
}
