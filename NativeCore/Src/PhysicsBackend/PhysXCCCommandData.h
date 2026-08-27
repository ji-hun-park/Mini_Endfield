// Unity Physics Backend System Native Plugin API Data Protocol copyright © 2025 Unity Technologies ApS
//
// Licensed under the Unity Companion License for Unity - dependent projects--see[Unity Companion License](http://www.unity3d.com/legal/licenses/Unity_Companion_License).
//
// Unless expressly provided otherwise, the Software under this license is made available strictly on an “AS IS” BASIS WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED.Please review the license for details on these and other terms and conditions.
//
// Please note that the following data protocol types represent the data used internally by Unity's Physics Module in order to communicate with the current SDK Integration. As such these types are subject to rapid change between different Unity versions.

#pragma once
#include "PhysXPhysicsExtensionCommands.h"
#include "PhysicsVecTypes.h"

namespace PhysicsCommands
{
    namespace PhysXExt
    {
        namespace CharacterControllerData
        {
            // Enables or disables overlap recovery on the character controller manager.
            // When enabled, the controller attempts to resolve penetrations with scene geometry.
            struct SetCharacterControllerMgrOverlapRecovery : Command
            {
                static constexpr auto command = PhysXExtension::SetCharacterControllerMgrOverlapRecovery;
                bool value;
            };

            // Creates a new character controller with the given capsule dimensions, physics material,
            // and movement parameters. Returns handles to the controller, its backing kinematic body, and backing shape.
            struct CreateCharacterController : Command
            {
                static constexpr auto command = PhysXExtension::CreateCharacterController;

                Physics::SDKObjectHandle material;
                void* userData;

                Physics::Vec3 initialPosition;
                float slopeLimit;
                float contactOffset;
                float stepOffset;
                float height;
                float radius;

                Physics::SDKObjectHandle outController;
                Physics::SDKObjectHandle outBackingBody;
                Physics::SDKObjectHandle outBackingShape;
            };

            // Destroys a character controller and releases its associated backing body and shape.
            struct DestroyCharacterController : Command
            {
                static constexpr auto command = PhysXExtension::DestroyCharacterController;

                Physics::SDKObjectHandle value;
            };

            // Updates the capsule dimensions (height and radius) of an existing character controller.
            struct SetCharacterControllerExtents : Command
            {
                static constexpr auto command = PhysXExtension::SetCharacterControllerExtents;

                float height;
                float radius;
            };

            // Sets the maximum obstacle height the character controller can automatically step over.
            struct SetCharacterControllerStepOffset : Command
            {
                static constexpr auto command = PhysXExtension::SetCharacterControllerStepOffset;

                float value;
            };

            // Sets the maximum slope angle as cosine threshold [0,1] the character controller can walk up.
            struct SetCharacterControllerSlopeLimit : Command
            {
                static constexpr auto command = PhysXExtension::SetCharacterControllerSlopeLimit;

                float value;
            };

            // Sets the contact offset (skin width) for the character controller.
            // This creates a small buffer around the capsule to improve collision detection.
            struct SetCharacterControllerContactOffset : Command
            {
                static constexpr auto command = PhysXExtension::SetCharacterControllerContactOffset;

                float value;
            };

            // Moves the character controller along 'moveDir' over 'deltaTime', performing collision detection
            // and sliding along surfaces. Movements smaller than 'minMoveDistance' are ignored.
            // Returns a bitmask of touched surfaces via 'outTouching' (sides, up, down).
            struct MoveCharacterController : Command
            {
                static constexpr auto command = PhysXExtension::MoveCharacterController;

                Physics::Vec3 moveDir;
                float minMoveDistance;
                float deltaTime;

                uint32_t outTouching;
            };

            // Sets the kinematic target position for the character controller's backing rigid body.
            struct SetCharacterControllerKinematicTarget : Command
            {
                static constexpr auto command = PhysXExtension::SetCharacterControllerKinematicTarget;

                Physics::Vec3 position;
            };

            // Retrieves the current kinematic target position of the character controller's backing rigid body.
            struct GetCharacterControllerKinematicTarget : Command
            {
                static constexpr auto command = PhysXExtension::GetCharacterControllerKinematicTarget;

                Physics::Vec3 outPosition;
            };

            using OnFetchCharacterControllerCollisions = void(*)(void* ctx, void* hitsVector);

            // Fetches all collision hits accumulated during the last character controller move and clears the internal buffer.
            // Results are delivered via the 'callback' with opaque hit data that the engine unpacks.
            struct GetAndClearCharacterControllerCollisions : Command
            {
                static constexpr auto command = PhysXExtension::GetAndClearCharacterControllerCollisions;

                OnFetchCharacterControllerCollisions callback;
                void* ctx;
            };
        }
    }
}
