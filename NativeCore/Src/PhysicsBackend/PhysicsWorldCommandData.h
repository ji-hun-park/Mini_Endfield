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
#include <limits>

#include "PhysicsCommands.h"
#include "PhysicsBodyTypes.h"
#include "PhysicsJointTypes.h"
#include "PhysicsWorldTypes.h"
#include "PhysicsEventTypes.h"
#include "PhysicsPose.h"

namespace Physics
{
    using ColliderBufferCallback = void(*)(void** buffer, size_t size);
}

namespace PhysicsCommands
{
    namespace WorldData
    {
        // Creates a new physics world (scene) with the specified solver, broadphase, friction, and simulation parameters.
        // Returns the world handle via 'outWorld'. The world is not populated until bodies are added.
        struct CreateWorld : Command
        {
            static constexpr auto command = World::CreateWorld;

            Physics::SDKObjectHandle outWorld = nullptr;
            Physics::ContactModificationEventCallback onModifyContactsCallback;
            void* userData = nullptr;

            Physics::SolverType solverType = Physics::SolverType::ProjectedGaussSeidelSolver;
            Physics::BroadphaseType broadphaseType = Physics::BroadphaseType::AutomaticBoxPruning;
            Physics::FrictionType frictionType = Physics::FrictionType::PatchFrictionType;
            Physics::ContactPairsMode contactPairsMode = Physics::ContactPairsMode::DefaultContactPairs;

            Physics::Vec3 gravity = { 0.0f, -9.81f, 0.0f };
            float bounceThreshold = 1.0f;
            float fastMotionThreshold = std::numeric_limits<float>::max();
            uint32_t jobWorkerCount = 0;

            Physics::Vec3 worldMin = { -256.0f, -256.0f, -256.0f };
            Physics::Vec3 worldMax = { 256.0f, 256.0f, 256.0f };
            uint32_t worldSubdivisions = 8;

            bool enableIncrementalStaticBroadphase = false;
            bool enableAdaptiveForce = false;
            bool enableEnhancedDeterminism = false;
        };

        // Destroys a physics world and releases all associated resources including bodies, shapes, and joints.
        struct DestroyWorld : Command
        {
            static constexpr auto command = World::DestroyWorld;
        };

        // Sets the gravity vector for this physics world.
        struct SetGravity : Command
        {
            static constexpr auto command = World::SetGravity;

            Physics::Vec3 value;

        };

        // Retrieves the current gravity vector for this physics world.
        struct GetGravity : Command
        {
            static constexpr auto command = World::GetGravity;

            Physics::Vec3 value;
        };

        // Sets the minimum relative velocity required for a bouncing contact to be generated.
        // Contacts with relative velocity below this threshold will not bounce.
        struct SetBounceThreshold : Command
        {
            static constexpr auto command = World::SetBounceThreshold;

            float value;
        };

        // Retrieves the current bounce velocity threshold for this world.
        struct GetBounceThreshold : Command
        {
            static constexpr auto command = World::GetBounceThreshold;

            float value;
        };

        // Retrieves basic statistics about the world (e.g. active body counts).
        struct GetBasicStats : Command
        {
            static constexpr auto command = World::GetBasicStats;

            Physics::WorldStats value;
        };

        // Sets the number of scratch buffer chunks available for simulation.
        // Scratch buffers reduce dynamic allocations during the simulation step.
        struct SetScratchBufferChunkCount : Command
        {
            static constexpr auto command = World::SetScratchBufferChunkCount;

            uint32_t value = 0;
        };

        // Advances the physics simulation by 'deltaTime' seconds.
        // 'jobGroupdId' identifies the job batch for threaded simulation.
        // 'ignoreEmptyScene' skips simulation if no actors are present.
        // 'ranSimulation' is set to false if the step was skipped.
        struct Simulate : Command
        {
            static constexpr auto command = World::Simulate;

            uint64_t jobGroupdId = 0;
            float deltaTime = 0.0f;
            float expectedMaxDeltaTime = std::numeric_limits<float>::max();
            bool ignoreEmptyScene = false;
            bool ranSimulation = true;
        };

        // Retrieves the contact pairs, trigger events, and joint break events generated by the last simulation step.
        // The caller must release these results via ReleaseLastSimulationStepData when done.
        struct GetLastSimulationStepResults : Command
        {
            static constexpr auto command = World::GetLastSimulationStepResults;

            Physics::BodyPair* bodyContactPairs;
            size_t bodyContactPairsCount;

            Physics::TriggerEvent* triggerEvents;
            size_t triggerEventsCount;

            Physics::JointBreakEvent* jointBreakEvents;
            size_t jointBreakEventsCount;
        };

        // Releases the simulation results data retrieved by GetLastSimulationStepResults,
        // freeing any backend-allocated memory.
        struct ReleaseLastSimulationStepData : Command
        {
            static constexpr auto command = World::ReleaseLastSimulationStepData;
        };

        // Retrieves detailed performance and diagnostic statistics from the last simulation step.
        struct GetLastSimulationStepStats : Command
        {
            static constexpr auto command = World::GetLastSimulationStats;

            Physics::SimulationStepStats value;
        };

        // Retrieves the engine-side user data pointer associated with this world.
        struct GetUserData : Command
        {
            static constexpr auto command = World::GetUserData;

            void* userData;
        };

        // Creates a new rigid body in the world with the given pose, mass, damping, and flags.
        // 'isStatic' determines whether the body is static (immovable) or dynamic.
        // Returns the body handle via 'outBody'. The body is not added to the scene until AddActorsToScene is called.
        struct CreateBody : Command
        {
            static constexpr auto command = World::CreateBody;

            Physics::Pose pose;
            void* userData;
            float mass;
            float linearDamping;
            float angularDamping;
            Physics::BodyFlags flags;
            bool isStatic;
            uint8_t padding[7];

            Physics::SDKObjectHandle outBody;
        };

        // Adds previously created actor handles to the physics scene for simulation.
        // 'actors' points to an array of handles and 'bufferSize' specifies the count.
        struct AddActorsToScene : Command
        {
            static constexpr auto command = World::AddActorsToScene;
            Physics::SDKObjectHandle actors;
            int bufferSize;
        };

        // Destroys a rigid body and removes it from the physics scene.
        struct DestroyBody : Command
        {
            static constexpr auto command = World::DestroyBody;

            Physics::SDKObjectHandle value;
        };

        // Creates a new articulation (a tree of connected rigid bodies with reduced-coordinate joints).
        // Returns the articulation handle via 'outArticulation'.
        struct CreateArticulation : Command
        {
            static constexpr auto command = World::CreateArticulation;

            Physics::ArticulationFlags flags;
            void* userData;

            Physics::SDKObjectHandle outArticulation;
        };

        // Destroys an articulation and all of its links.
        struct DestoryArticulation : Command
        {
            static constexpr auto command = World::DestroyArticulation;

            Physics::SDKObjectHandle value;
        };

        // Creates a joint connecting two bodies with the specified type (Fixed, Hinge, 6DOF, etc.) and flags.
        // Returns the joint handle via 'outJoint'.
        struct CreateJoint : Command
        {
            static constexpr auto command = World::CreateJoint;

            Physics::SDKObjectHandle body0;
            Physics::SDKObjectHandle body1;
            void* userData;
            Physics::JointType type;
            Physics::JointFlags flags;

            Physics::SDKObjectHandle outJoint;
        };

        // Destroys a joint and removes its constraint from the simulation.
        struct DestroyJoint : Command
        {
            static constexpr auto command = World::DestroyJoint;

            Physics::SDKObjectHandle value;
        };

        // Retrieves the number of articulations currently present in this world.
        struct GetArticulationCount : Command
        {
            static constexpr auto command = World::GetArticulationCount;

            uint32_t value;
        };

        // Retrieves the SDK handles of all articulations in this world into the provided buffer.
        // 'written' reports how many handles were actually written.
        struct GetArticulations : Command
        {
            static constexpr auto command = World::GetArticulations;

            Physics::SDKObjectHandle* buffer;
            uint32_t bufferSize;
            uint32_t written;
        };

        // Retrieves all shapes in the world that belong to layers matching the given layer mask.
        // Results are delivered via the 'callback' which receives a buffer of shape user data pointers.
        struct GetAllShapesByLayerMask : Command
        {
            static constexpr auto command = World::GetAllShapesByLayerMask;

            uint32_t layerMask;
            Physics::ColliderBufferCallback callback;
        };
    }
}
