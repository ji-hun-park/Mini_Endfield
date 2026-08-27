using UnityEditor;
using UnityEngine;
using UnityEngine.Rendering.Universal;
using System.Reflection;

public class CheckExecute
{
    [MenuItem("Tools/Check Execute")]
    public static void Run()
    {
        var type = typeof(ScriptableRenderPass);
        var methods = type.GetMethods(BindingFlags.Public | BindingFlags.Instance);
        foreach (var m in methods)
        {
            if (m.Name == "Execute")
            {
                Debug.Log(m.Name + "(" + m.GetParameters()[0].ParameterType + ", " + m.GetParameters()[1].ParameterType + ")");
            }
        }
    }
}
