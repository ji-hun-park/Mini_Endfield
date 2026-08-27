using UnityEditor;
using UnityEngine;

public class TestFormat
{
    [MenuItem("Tools/Test Format")]
    public static void Run()
    {
        Debug.Log("System format: " + SystemInfo.GetGraphicsFormat(UnityEngine.Experimental.Rendering.DefaultFormat.LDR));
        Debug.Log("HDR format: " + SystemInfo.GetGraphicsFormat(UnityEngine.Experimental.Rendering.DefaultFormat.HDR));
    }
}
