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
#include "PhysicsVecTypes.h"

namespace Physics
{
    // Bitmask flags controlling rigid body behavior in the simulation.
    enum class BodyFlags
    {
        None = 0,
        Gravity = 1 << 0,     // Body is affected by gravity
        Kinematic = 1 << 1,   // Body is kinematic (moved via SetKinematicTarget, not by forces)
        Simulated = 1 << 2    // Body participates in the physics simulation
    };

    inline constexpr BodyFlags operator | (BodyFlags lhs, BodyFlags rhs)
    {
        using type_t = std::underlying_type_t<BodyFlags>;
        return static_cast<BodyFlags>(static_cast<type_t>(lhs) | static_cast<type_t>(rhs));
    }

    inline constexpr BodyFlags& operator |= (BodyFlags& lhs, BodyFlags rhs)
    {
        lhs = lhs | rhs;
        return lhs;
    }

    inline constexpr BodyFlags operator ~(BodyFlags v)
    {
        using type_t = std::underlying_type_t<BodyFlags>;

        return static_cast<BodyFlags>(~static_cast<type_t>(v));
    }

    inline constexpr BodyFlags operator &(BodyFlags lhs, BodyFlags rhs)
    {
        using type_t = std::underlying_type_t<BodyFlags>;
        return static_cast<BodyFlags>(static_cast<type_t>(lhs) & static_cast<type_t>(rhs));
    }

    inline constexpr BodyFlags operator &=(BodyFlags& lhs, BodyFlags rhs)
    {
        lhs = lhs & rhs;
        return lhs;
    }

    // Bitmask flags controlling articulation-level behavior.
    enum class ArticulationFlags
    {
        None = 0,
        DriveLimitsAsForces = 1 << 0,  // Interpret joint drive limits as force limits rather than impulse limits
        ImmovableRoot = 1 << 1,        // Root link is fixed in world space (cannot be moved by forces)
        Simulated = 1 << 2             // Articulation participates in the physics simulation
    };

    inline constexpr ArticulationFlags operator | (ArticulationFlags lhs, ArticulationFlags rhs)
    {
        using type_t = std::underlying_type_t<ArticulationFlags>;
        return static_cast<ArticulationFlags>(static_cast<type_t>(lhs) | static_cast<type_t>(rhs));
    }

    inline constexpr ArticulationFlags& operator |= (ArticulationFlags& lhs, ArticulationFlags rhs)
    {
        lhs = lhs | rhs;
        return lhs;
    }

    inline constexpr ArticulationFlags operator &(ArticulationFlags lhs, ArticulationFlags rhs)
    {
        using type_t = std::underlying_type_t<ArticulationFlags>;
        return static_cast<ArticulationFlags>(static_cast<type_t>(lhs) & static_cast<type_t>(rhs));
    }

    // Selects the collision detection algorithm used for a rigid body.
    enum class CollisionDetectionMode
    {
        Discrete = 0,             // Standard per-step discrete collision detection
        Continuous,               // Continuous detection against static geometry only
        ContinuousDynamic,        // Continuous detection against both static and dynamic geometry
        ContinuousSpeculative     // Speculative continuous detection using inflated bounds
    };

    // Determines how a force or torque value is applied to a rigid body.
    enum class ForceMode
    {
        Force = 0,       // Continuous force (mass-dependent, integrated over deltaTime)
        Impulse,         // Instantaneous impulse (mass-dependent, applied in one step)
        VelocityChange,  // Instantaneous velocity change (mass-independent, applied in one step)
        Acceleration = 5 // Continuous acceleration (mass-independent, integrated over deltaTime)
    };

    // Specifies which mass properties (center of mass, inertia tensor) should be overridden
    // when recomputing mass properties from attached shapes.
    struct MassPropertiesOverride
    {
        MassPropertiesOverride()
            : inertiaTensorRotation{0,0,0,1}
            , inertiaTensor{1,1,1}
            , centerOfMass{0,0,0}
            , overrideCenterOfMass(false)
            , overrideInertiaTensor(false)
        {}
        Vec4 inertiaTensorRotation;
        Vec3 inertiaTensor;
        Vec3 centerOfMass;
        bool overrideCenterOfMass;
        bool overrideInertiaTensor;
    };
}
