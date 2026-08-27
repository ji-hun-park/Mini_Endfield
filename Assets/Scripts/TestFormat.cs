using UnityEngine;

public class TestFormat : MonoBehaviour
{
    void Start()
    {
        Debug.Log("System format: " + SystemInfo.GetGraphicsFormat(UnityEngine.Experimental.Rendering.DefaultFormat.LDR));
        Debug.Log("HDR format: " + SystemInfo.GetGraphicsFormat(UnityEngine.Experimental.Rendering.DefaultFormat.HDR));
    }
}
