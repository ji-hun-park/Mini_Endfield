using UnityEngine;
using UnityEngine.Rendering;
using UnityEngine.Rendering.Universal;
using UnityEngine.Rendering.RenderGraphModule;
using System;

namespace Endfield.Rendering
{
    public class NativeRenderFeature : ScriptableRendererFeature
    {
        class NativeRenderPass : ScriptableRenderPass
        {
            class PassData
            {
                public TextureHandle colorTarget;
                public TextureHandle depthTarget;
            }

            public override void RecordRenderGraph(RenderGraph renderGraph, ContextContainer frameData) { 
                UniversalCameraData cameraData = frameData.Get<UniversalCameraData>();
                if (cameraData.cameraType != CameraType.Game && cameraData.cameraType != CameraType.SceneView)
                    return;

                UniversalResourceData resourceData = frameData.Get<UniversalResourceData>();

                using (var builder = renderGraph.AddUnsafePass<PassData>("Native Vulkan Pass", out var passData))
                {
                    passData.colorTarget = resourceData.activeColorTexture;
                    passData.depthTarget = resourceData.activeDepthTexture;

                    if (passData.colorTarget.IsValid())
                        builder.UseTexture(passData.colorTarget, AccessFlags.Write);
                    if (passData.depthTarget.IsValid())
                        builder.UseTexture(passData.depthTarget, AccessFlags.ReadWrite);

                    builder.AllowPassCulling(false); builder.AllowGlobalStateModification(true);
                    
                    builder.SetRenderFunc((PassData data, UnsafeGraphContext context) =>
                    {
                        var cmd = CommandBufferHelpers.GetNativeCommandBuffer(context.cmd);
                        
                        if (data.colorTarget.IsValid())
                        {
                            if (data.depthTarget.IsValid())
                                CoreUtils.SetRenderTarget(cmd, data.colorTarget, data.depthTarget);
                            else
                                CoreUtils.SetRenderTarget(cmd, data.colorTarget);
                        }

                        IntPtr renderEventFunc = VulkanPluginWrapper.GetRenderEventFunc();
                        if (renderEventFunc != IntPtr.Zero)
                        {
                            cmd.IssuePluginEvent(renderEventFunc, 1);
                        }
                    });
                }
            }

            #pragma warning disable CS0115
            public virtual void Execute(ScriptableRenderContext context, ref RenderingData renderingData)
            {
                if (renderingData.cameraData.cameraType != CameraType.Game && renderingData.cameraData.cameraType != CameraType.SceneView)
                    return;

                var cmd = CommandBufferPool.Get("Native Vulkan Pass");
                IntPtr renderEventFunc = VulkanPluginWrapper.GetRenderEventFunc();
                if (renderEventFunc != IntPtr.Zero)
                {
                    cmd.IssuePluginEvent(renderEventFunc, 1);
                }
                context.ExecuteCommandBuffer(cmd);
                CommandBufferPool.Release(cmd);
            }
            #pragma warning restore CS0115
        }

        NativeRenderPass m_ScriptablePass;

        public override void Create()
        {
            m_ScriptablePass = new NativeRenderPass();
            m_ScriptablePass.renderPassEvent = RenderPassEvent.AfterRenderingPostProcessing;
        }

        public override void AddRenderPasses(ScriptableRenderer renderer, ref RenderingData renderingData)
        {
            renderer.EnqueuePass(m_ScriptablePass); 
        }
    }
}












