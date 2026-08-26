using UnityEditor;
using UnityEditor.Build;

[InitializeOnLoad]
public class EnableUnsafeCode
{
    static EnableUnsafeCode()
    {
        var buildTargetGroup = EditorUserBuildSettings.selectedBuildTargetGroup;
        if (buildTargetGroup == BuildTargetGroup.Unknown)
            return;

        // Ensure "Allow 'unsafe' Code" is checked in Player Settings
        if (!PlayerSettings.allowUnsafeCode)
        {
            PlayerSettings.allowUnsafeCode = true;
            UnityEngine.Debug.Log("Automatically enabled 'Allow unsafe code' in Player Settings.");
        }
    }
}

