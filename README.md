# GFXVulkanBackend
This project is a Vulkan backend for Turan Engine's (Non-DLL, Non-STL version) GFX API. Turan Engine's created based on OpenGL but this type of usage limits most of the modern APIs, so I have to re-design all of the Turan Engine's GFX API according to this project. There are some new features too, such as GPU Memory Management and a better RenderGraph.

# How it works
1) Because this project is gonna be integrated in Turan Engine's Non-DLL Non-STL version, code architecture is similar. Source/Main.cpp is the entry point and each DLL is in folders called by their names (GFX, Vulkan, TuranAPI etc.). TuranAPI includes Multi-Threading, Logging, Profiling, I/O and Bitset (A nice vector<bool> implementation) libraries; GFX includes general interface for Vulkan backend calls; Vulkan is the place where Vulkan API functions are called; Editor is where user codes an executable.
2) Asset (Mesh, Shader, Texture etc) importing is handled in Editor/FileSystem. But Vulkan's depth buffer and 3D rendering isn't fully tested yet, so just skip that part.
3) In Turan Engine, application developer is responsible for defining a RenderGraph to render anything (Except IMGUI for now, but I'm working on it). A RenderGraph is a collection of RenderNodes that is able to run a specific GPU job (Transfer, Rasterize, Compute, Swapchain Display) and has dependencies on each other. That means, you have to create RenderNode(s) to define a RenderGraph. Creating RenderNodes is a little bit complicated, so you should check GFX/Renderer/README.md .
4) If you want to upload anything to the GPU, you should use GFXContentManager to do so. It is a -as the name suggests- GPU's content manager. It is responsible for copying and deleting resources on GPU. For detailed description, check GFX/Renderer/GPUContentManager.h and GFX/Renderer/Resources_README.md
5) Turan Engine has a Material system that consists of Material Type and lots of Material Instances of them. So there is nothing like "Bind Shader, Bind Uniform etc.". Material Type is a collection of programmable rasterization pipeline datas such as blending type, depth test type, shaders. Material Instance is a table that stores values of a Material Type's uniforms (Similar to Unreal's Material Instance but there is no optimization for static uniforms etc). You should use GFXRENDERER for your rendering operations, so a draw call is called like "GFXRENDERER->Render_DrawCall()". Draw Calling is highly related to the RenderGraph part, so read GFX/Renderer/README.md
6) dear IMGUI's integrated in GFX API's itself but dear IMGUI's graphics API related files are integrated in the related API's IMGUI folder and the needed linkage is made at initialization proccess. But dear IMGUI's Vulkan backend doesn't match with RenderGraph algorithm and window creation process of the GFX API which I couldn't find how to fix. I think I'm gonna change dear IMGUI's Vulkan backend.
7) You should call GFX->Create_Window() to create a window. Because it creates Swapchain Textures, you should handle textures' layout transitions etc.


# Plans:
1) Finish all of the non-coded parts
2) Support reconstruction of RenderGraph and GFX API as fast as possible (Custom memory allocation is needed)

# Work In Progress:
1) Multi-threading library is coded but application doesn't benefit from it
2) Vulkan initialization process is getting cleaner