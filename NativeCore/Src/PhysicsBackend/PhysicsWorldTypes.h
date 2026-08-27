// Unity Physics Backend System Native Plugin API Data Protocol copyright © 2025 Unity Technologies ApS
//
// Licensed under the Unity Companion License for Unity - dependent projects--see[Unity Companion License](http://www.unity3d.com/legal/licenses/Unity_Companion_License).
//
// Unless expressly provided otherwise, the Software under this license is made available strictly on an “AS IS” BASIS WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED.Please review the license for details on these and other terms and conditions.
//
// Please note that the following data protocol types represent the data used internally by Unity's Physics Module in order to communicate with the current SDK Integration. As such these types are subject to rapid change between different Unity versions.

#pragma  once
#include <cstdint>
#include <type_traits>
#include "PhysicsCommonTypes.h"

namespace Physics
{
    // Selects the constraint solver algorithm used for the physics world.
    enum class SolverType
    {
        ProjectedGaussSeidelSolver = 0, // Standard iterative solver (PGS), good general-purpose performance
        TemporalGaussSeidelSolver = 1   // Temporal solver (TGS), improved joint stability at higher iteration cost
    };

    // Selects the broadphase collision detection algorithm used for the physics world.
    enum class BroadphaseType
    {
        SweepAndPruneBroadphase = 0, // Incremental sweep-and-prune; good for scenes with moderate object counts
        MultiBoxPruning = 1,         // Manual multi-box pruning (MBP) with user-defined regions
        AutomaticBoxPruning = 2      // Automatic box pruning; recommended default for most scenes
    };

    // Selects the friction computation model used by the solver.
    enum class FrictionType
    {
        PatchFrictionType = 0,          // Patch-based friction (default, best stability)
        OneDirectionalFrictionType = 1, // One-directional analytical friction
        TwoDirectionalFrictionType = 2  // Two-directional analytical friction (most accurate, highest cost)
    };

    // Bitmask controlling which additional contact pair types are generated during simulation.
    // By default only dynamic-static and dynamic-dynamic pairs are produced.
    enum class ContactPairsMode
    {
        DefaultContactPairs = 0,
        EnableKinematicKinematicPairs = 1 << 0, // Generate contact pairs between kinematic bodies
        EnableKinematicStaticPairs = 1 << 1,    // Generate contact pairs between kinematic and static bodies
        EnableAllContactPairs = EnableKinematicKinematicPairs | EnableKinematicStaticPairs
    };

    inline constexpr ContactPairsMode operator | (ContactPairsMode lhs, ContactPairsMode rhs)
    {
        using type_t = std::underlying_type_t<ContactPairsMode>;
        return static_cast<ContactPairsMode>(static_cast<type_t>(lhs) | static_cast<type_t>(rhs));
    }

    inline constexpr ContactPairsMode& operator |= (ContactPairsMode& lhs, ContactPairsMode rhs)
    {
        lhs = lhs | rhs;
        return lhs;
    }

    inline constexpr ContactPairsMode operator ~(ContactPairsMode v)
    {
        using type_t = std::underlying_type_t<ContactPairsMode>;

        return static_cast<ContactPairsMode>(~static_cast<type_t>(v));
    }

    inline constexpr ContactPairsMode operator &(ContactPairsMode lhs, ContactPairsMode rhs)
    {
        using type_t = std::underlying_type_t<ContactPairsMode>;
        return static_cast<ContactPairsMode>(static_cast<type_t>(lhs) & static_cast<type_t>(rhs));
    }

    inline constexpr ContactPairsMode operator &=(ContactPairsMode& lhs, ContactPairsMode rhs)
    {
        lhs = lhs & rhs;
        return lhs;
    }

    // Callback invoked during simulation when contact modification is enabled.
    // Receives the raw SDK-specific contact buffer and count, with 'isCCDStream' indicating
    // whether the contacts come from the CCD (continuous collision detection) pass.
    using ContactModificationEventCallback = void(*)(void* context, void* sdkSpecificContactBuffer, const size_t contactCount, bool isCCDStream);

    // Basic world statistics: counts of bodies, articulations, and constraints currently in the scene.
    struct WorldStats
    {
        WorldStats() = default;

        WorldStats(size_t articulations_, size_t bodies_, size_t constraints_)
            : articulations(articulations_)
            , bodies(bodies_)
            , constraints(constraints_)
        {
        }

        uint32_t bodies = 0;
        uint32_t articulations = 0;
        uint32_t constraints = 0;
    };

    // Detailed per-step simulation statistics including body counts by type, broadphase activity,
    // contact pair counts, and CCD/modifier/trigger pair tallies.
    struct SimulationStepStats
    {
        SimulationStepStats() = default;

        SimulationStepStats(
            uint32_t dynamicBodies_,
            uint32_t activeDynamicBodies_,
            uint32_t kinematicBodies_,
            uint32_t activeKinematicBodies_,
            uint32_t staticBodies_,
            uint32_t articulations_,
            uint32_t constraints_,
            uint32_t broadPhaseAdds_,
            uint32_t broadPhaseRemoves_,
            uint32_t newTouches_,
            uint32_t lostTouches_,
            uint32_t contactPairs_,
            uint32_t CCDContactPairs_,
            uint32_t modifiableContactPairs_,
            uint32_t triggerPairs_)
            : dynamicBodies(dynamicBodies_)
            , activeDynamicBodies(activeDynamicBodies_)
            , kinematicBodies(kinematicBodies_)
            , activeKinematicBodies(activeKinematicBodies_)
            , staticBodies(staticBodies_)
            , articulations(articulations_)
            , constraints(constraints_)
            , broadPhaseAdds(broadPhaseAdds_)
            , broadPhaseRemoves(broadPhaseRemoves_)
            , newTouches(newTouches_)
            , lostTouches(lostTouches_)
            , contactPairs(contactPairs_)
            , CCDContactPairs(CCDContactPairs_)
            , modifiableContactPairs(modifiableContactPairs_)
            , triggerPairs(triggerPairs_)
        {
        }

        uint32_t dynamicBodies = 0;
        uint32_t activeDynamicBodies = 0;
        uint32_t kinematicBodies = 0;
        uint32_t activeKinematicBodies = 0;
        uint32_t staticBodies = 0;
        uint32_t articulations = 0;
        uint32_t constraints = 0;
        uint32_t broadPhaseAdds = 0;
        uint32_t broadPhaseRemoves = 0;
        uint32_t newTouches = 0;
        uint32_t lostTouches = 0;
        uint32_t contactPairs = 0;
        uint32_t CCDContactPairs = 0;
        uint32_t modifiableContactPairs = 0;
        uint32_t triggerPairs = 0;
    };
}
