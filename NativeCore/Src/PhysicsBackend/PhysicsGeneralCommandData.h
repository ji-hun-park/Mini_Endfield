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
#include "PhysicsCommonTypes.h"
#include "PhysicsVecTypes.h"

namespace PhysicsCommands
{
    namespace SdkData
    {
        // Initializes the physics SDK. Sets the engine object ID offset used for handle mapping
        // and reports whether the vehicle extension module is present. Returns true on success via 'result'.
        struct InitializeSdk : Command
        {
            static constexpr auto command = SDK::InitializeSdk;

            uint16_t engineObjectIdOffset = 0;

            bool vehicleExtensionModulePresent = false;

            bool result = false;
        };

        // Shuts down the physics SDK and releases all associated global resources.
        struct ShutdownSdk : Command
        {
            static constexpr auto command = SDK::ShutdownSdk;
        };

        // Retrieves integration metadata from the physics backend including name, version, supported features, and limits.
        struct GetSdkIntegrationInfo : Command
        {
            static constexpr auto command = SDK::GetIntegrationInfo;
            Physics::IntegrationInfo value;
        };

        // Sets the error verbosity level controlling which physics backend messages are reported to the console.
        struct SetErrorVerbosityLevel : Command
        {
            static constexpr auto command = SDK::SetErrorVerbosityLevel;
            Physics::ErrorVerbosityLevel value;
        };

        // Connects to a visual debugger (e.g. PhysX Visual Debugger) at the specified network address and port.
        // Returns true via 'result' if the connection was established within the timeout.
        struct ConnectVisualDebugger : Command
        {
            static constexpr auto command = SDK::ConnectVisualDebugger;

            const char* addr = NULL;
            int port = -1;
            int timeoutInMs = 15;

            bool result = false;
        };

        // Disconnects from the currently connected visual debugger session.
        struct DisconnectVisualDebugger : Command
        {
            static constexpr auto command = SDK::DisconnectVisualDebugger;
        };

        // Queries whether a visual debugger session is currently active. Returns result via 'value'.
        struct GetIsVisualDebuggerConnected : Command
        {
            static constexpr auto command = SDK::GetIsVisualDebuggerConnected;

            bool value;
        };

        // Updates the camera viewport in the visual debugger so the remote view tracks the editor/game camera.
        struct SetVisualDebuggerViewport : Command
        {
            static constexpr auto command = SDK::SetVisualDebuggerViewport;

            Physics::Vec3 position;
            Physics::Vec3 up;
            Physics::Vec3 target;
        };
    }
}
