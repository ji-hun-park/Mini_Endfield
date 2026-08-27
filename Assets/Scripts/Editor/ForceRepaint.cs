using UnityEditor;
using UnityEngine;

public class ForceRepaint
{
    [MenuItem("Tools/Force Repaint")]
    public static void Run()
    {
        SceneView.RepaintAll();
        Debug.Log("Forced Repaint!");
    }
}
