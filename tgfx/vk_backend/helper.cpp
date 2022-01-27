#include <iostream>
#include "tgfx_forwarddeclarations.h"
#include "tgfx_structs.h"
#include "helper.h"
#include <tgfx_core.h>
#include <tgfx_helper.h>
#include "core.h"
#include "extension.h"
#include "resource.h"
#include "predefinitions_vk.h"
#include "includes.h"


//Hardware Capability Helpers
static void  GetGPUInfo_General(gpu_tgfx_handle GPUHandle, const char** NAME, unsigned int* API_VERSION, unsigned int* DRIVER_VERSION, gpu_type_tgfx* GPUTYPE, const memory_description_tgfx** MemType_descs,
    unsigned int* MemType_descsCount, unsigned char* isGraphicsOperationsSupported, unsigned char* isComputeOperationsSupported, unsigned char* isTransferOperationsSupported) {
    gpu_public* VKGPU = (gpu_public*)GPUHandle;
    if (NAME) { *NAME = VKGPU->DEVICENAME(); }
    if (API_VERSION) { *API_VERSION = VKGPU->APIVERSION(); }
    if (DRIVER_VERSION) { *DRIVER_VERSION = VKGPU->DRIVERSION(); }
    if (GPUTYPE) { *GPUTYPE = VKGPU->DEVICETYPE(); }

    if (MemType_descs) { *MemType_descs = VKGPU->DESC().MEMTYPEs; *MemType_descsCount = VKGPU->DESC().MEMTYPEsCOUNT; }
    if (isGraphicsOperationsSupported) { *isGraphicsOperationsSupported = VKGPU->GRAPHICSSUPPORTED(); }
    if (isComputeOperationsSupported) { *isComputeOperationsSupported = VKGPU->COMPUTESUPPORTED(); }
    if (isTransferOperationsSupported) { *isTransferOperationsSupported = VKGPU->TRANSFERSUPPORTED(); }
}
static unsigned char  GetTextureTypeLimits(texture_dimensions_tgfx dims, texture_order_tgfx dataorder, texture_channels_tgfx channeltype,
    textureusageflag_tgfx_handle usageflag, gpu_tgfx_handle GPUHandle, unsigned int* MAXWIDTH, unsigned int* MAXHEIGHT, unsigned int* MAXDEPTH,
    unsigned int* MAXMIPLEVEL) {
    VkImageFormatProperties props;
    VkResult result = vkGetPhysicalDeviceImageFormatProperties(rendergpu->PHYSICALDEVICE(), Find_VkFormat_byTEXTURECHANNELs(channeltype),
        Find_VkImageType(dims), Find_VkTiling(dataorder), *(VkImageUsageFlags*)usageflag,
        0, &props);
    if (result != VK_SUCCESS) {
        printer(result_tgfx_FAIL, ("GFX->GetTextureTypeLimits() has failed with: " + std::to_string(result)).c_str());
        return false;
    }
    printer(result_tgfx_SUCCESS, "After vkGetPhysicalDeviceImageFormatProperties()!");

    if (MAXWIDTH) { *MAXWIDTH = props.maxExtent.width; }
    if(MAXHEIGHT){ *MAXHEIGHT = props.maxExtent.height; }
    if(MAXDEPTH){ *MAXDEPTH = props.maxExtent.depth; }
    if(MAXMIPLEVEL){ *MAXMIPLEVEL = props.maxMipLevels; }
    return true;
}
static textureusageflag_tgfx_handle CreateTextureUsageFlag(unsigned char isCopiableFrom, unsigned char isCopiableTo,
    unsigned char isRenderableTo, unsigned char isSampledReadOnly, unsigned char isRandomlyWrittenTo) {
    VkImageUsageFlags* UsageFlag = new VkImageUsageFlags;
    *UsageFlag = ((isCopiableFrom) ? VK_IMAGE_USAGE_TRANSFER_SRC_BIT : 0) |
        ((isCopiableTo) ? VK_IMAGE_USAGE_TRANSFER_DST_BIT : 0) |
        ((isRenderableTo) ? VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT : 0) |
        ((isSampledReadOnly) ? VK_IMAGE_USAGE_SAMPLED_BIT : 0) |
        ((isRandomlyWrittenTo) ? VK_IMAGE_USAGE_STORAGE_BIT : 0);
    return (textureusageflag_tgfx_handle)UsageFlag;
}
static void  GetSupportedAllocations_ofTexture(unsigned int GPUIndex, unsigned int* SupportedMemoryTypesBitset) {

}
//You can't create a memory type, only set allocation size of a given Memory Type (Memory Type is given by the GFX initialization process)
extern result_tgfx SetMemoryTypeInfo(unsigned int MemoryType_id, unsigned long long AllocationSize, extension_tgfx_listhandle Extensions);
static initializationsecondstageinfo_tgfx_handle Create_GFXInitializationSecondStageInfo(
    gpu_tgfx_handle RendererGPU,
    //You have to specify how much shader input categories you're gonna use max (material types that have general shader inputs + material instances that have per instance shader inputs)
    unsigned int MaterialCount,
    //You have to specify max sum of how much shader inputs you're gonna use for materials (General and Per Instance)
    unsigned int MaxSumMaterial_SampledTexture, unsigned int MaxSumMaterial_ImageTexture, unsigned int MaxSumMaterial_UniformBuffer, unsigned int MaxSumMaterial_StorageBuffer,
    //You have to specify how many global shader inputs you're gonna use (Don't forget to look at descriptor indexing extensions)
    unsigned int GlobalSampledTextureInputCount, unsigned int GlobalImageTextureInputCount, unsigned int GlobalUniformBufferInputCount, unsigned int GlobalInputStorageBufferInputCount,
    //See the documentation
    unsigned char isGlobalUniformBuffer_Index1, unsigned char isGlobalSampledTexture_Index1,
    unsigned char ShouldActive_dearIMGUI,
    extension_tgfx_listhandle EXTList) {
    gpu_public* VKGPU = (gpu_public*)RendererGPU;

    unsigned int MAXDESC_SAMPLEDTEXTURE = MaxSumMaterial_SampledTexture + GlobalSampledTextureInputCount;
    unsigned int MAXDESC_IMAGETEXTURE = MaxSumMaterial_ImageTexture + GlobalImageTextureInputCount;
    unsigned int MAXDESC_UNIFORMBUFFER = MaxSumMaterial_UniformBuffer + GlobalUniformBufferInputCount;
    unsigned int MAXDESC_STORAGEBUFFER = MaxSumMaterial_StorageBuffer + GlobalInputStorageBufferInputCount;
    if (MAXDESC_SAMPLEDTEXTURE > VKGPU->EXTMANAGER()->GETMAXDESC(desctype_vk::SAMPLER) || MAXDESC_IMAGETEXTURE > VKGPU->EXTMANAGER()->GETMAXDESC(desctype_vk::IMAGE) ||
        MAXDESC_UNIFORMBUFFER > VKGPU->EXTMANAGER()->GETMAXDESC(desctype_vk::UBUFFER) || MAXDESC_STORAGEBUFFER > VKGPU->EXTMANAGER()->GETMAXDESC(desctype_vk::SBUFFER)) {
        printer(result_tgfx_FAIL, "One of the shader input types exceeds the GPU's limits. Don't forget that Global + General + Per Instance shouldn't exceed GPU's related shader input type limits!");
        return nullptr;
    }

    InitializationSecondStageInfo* info = new InitializationSecondStageInfo;

    info->MaxSumMaterial_ImageTexture = MaxSumMaterial_ImageTexture;
    info->MaxSumMaterial_SampledTexture = MaxSumMaterial_SampledTexture;
    info->MaxSumMaterial_StorageBuffer = MaxSumMaterial_StorageBuffer;
    info->MaxSumMaterial_UniformBuffer = MaxSumMaterial_UniformBuffer;
    info->GlobalShaderInput_ImageTextureCount = GlobalImageTextureInputCount;
    info->GlobalShaderInput_SampledTextureCount = GlobalSampledTextureInputCount;
    info->GlobalShaderInput_StorageBufferCount = GlobalInputStorageBufferInputCount;
    info->GlobalShaderInput_UniformBufferCount = GlobalUniformBufferInputCount;
    info->MaxMaterialCount = MaterialCount;
    info->renderergpu = VKGPU;
    info->isSampledTexture_Index1 = isGlobalSampledTexture_Index1;
    info->isUniformBuffer_Index1 = isGlobalUniformBuffer_Index1;
    info->shouldActivate_DearIMGUI = ShouldActive_dearIMGUI;
    return (initializationsecondstageinfo_tgfx_handle)info;
}
static void  GetMonitor_Resolution_ColorBites_RefreshRate(monitor_tgfx_handle MonitorHandle, unsigned int* WIDTH, unsigned int* HEIGHT, unsigned int* ColorBites, unsigned int* RefreshRate) {

}
//Barrier Dependency Helpers

static waitsignaldescription_tgfx_handle CreateWaitSignal_DrawIndirectConsume() {
    return nullptr;
}
static waitsignaldescription_tgfx_handle CreateWaitSignal_VertexInput(unsigned char IndexBuffer, unsigned char VertexAttrib) {
    return nullptr;
}
static waitsignaldescription_tgfx_handle CreateWaitSignal_VertexShader(unsigned char UniformRead, unsigned char StorageRead, unsigned char StorageWrite) {
    return nullptr;
}
static waitsignaldescription_tgfx_handle CreateWaitSignal_FragmentShader(unsigned char UniformRead, unsigned char StorageRead, unsigned char StorageWrite) {
    return nullptr;
}
static waitsignaldescription_tgfx_handle CreateWaitSignal_ComputeShader(unsigned char UniformRead, unsigned char StorageRead, unsigned char StorageWrite) {
    return nullptr;
}
static waitsignaldescription_tgfx_handle CreateWaitSignal_FragmentTests(unsigned char isEarly, unsigned char isRead, unsigned char isWrite) {
    return nullptr;
}
static waitsignaldescription_tgfx_handle CreateWaitSignal_Transfer(unsigned char UniformRead, unsigned char StorageRead) {
    return nullptr;
}

//RENDERNODE HELPERS


//WaitInfos is a pointer because function expects a list of waits (Color attachment output and also VertexShader-UniformReads etc)
static passwaitdescription_tgfx_handle CreatePassWait_DrawPass(drawpass_tgfx_handle* PassHandle, unsigned char SubpassIndex,
    waitsignaldescription_tgfx_handle* WaitInfos, unsigned char isLastFrame) {
    return nullptr;
}
//WaitInfo is single, because function expects only one wait and it should be created with CreateWaitSignal_ComputeShader()
static passwaitdescription_tgfx_handle CreatePassWait_ComputePass(computepass_tgfx_handle* PassHandle, unsigned char SubpassIndex,
    waitsignaldescription_tgfx_handle WaitInfo, unsigned char isLastFrame) {
    return nullptr;
}
//WaitInfo is single, because function expects only one wait and it should be created with CreateWaitSignal_Transfer()
static passwaitdescription_tgfx_handle CreatePassWait_TransferPass(transferpass_tgfx_handle* PassHandle,
    transferpasstype_tgfx Type, waitsignaldescription_tgfx_handle WaitInfo, unsigned char isLastFrame) {
    return nullptr;
}
//There is no option because you can only wait for a penultimate window pass
//I'd like to support last frame wait too but it confuses the users and it doesn't have much use
static passwaitdescription_tgfx_handle CreatePassWait_WindowPass(windowpass_tgfx_handle* PassHandle) {
    return nullptr;
}

static subdrawpassdescription_tgfx_handle CreateSubDrawPassDescription(inheritedrtslotset_tgfx_handle irtslotset, subdrawpassaccess_tgfx WaitOP, subdrawpassaccess_tgfx ContinueOP) {
    subdrawpassdesc_vk* desc = new subdrawpassdesc_vk;
    desc->INHERITEDSLOTSET = (irtslotset_vk*)irtslotset;
    desc->WaitOp = WaitOP;
    desc->ContinueOp = ContinueOP;
    return (subdrawpassdescription_tgfx_handle)desc;
}


static shaderinputdescription_tgfx_handle CreateShaderInputDescription(unsigned char isGeneral, shaderinputtype_tgfx Type, unsigned int BINDINDEX,
    unsigned int ELEMENTCOUNT, shaderstageflag_tgfx_handle Stages) {
    return nullptr;
}
static rtslotdescription_tgfx_handle CreateRTSlotDescription_Color(texture_tgfx_handle Texture0, texture_tgfx_handle Texture1,
    operationtype_tgfx OPTYPE, drawpassload_tgfx LOADTYPE, unsigned char isUsedLater, unsigned char SLOTINDEX, vec4_tgfx clear_value) {
    rtslotdesc_vk* desc = new rtslotdesc_vk;
    desc->clear_value = clear_value;
    desc->isUsedLater = isUsedLater;
    desc->loadtype = LOADTYPE;
    desc->optype = OPTYPE;
    desc->textures[0] = (texture_vk*)Texture0;
    desc->textures[1] = (texture_vk*)Texture1;
    return (rtslotdescription_tgfx_handle)desc;
}
static rtslotdescription_tgfx_handle CreateRTSlotDescription_DepthStencil(texture_tgfx_handle Texture0, texture_tgfx_handle Texture1,
    operationtype_tgfx DEPTHOP, drawpassload_tgfx DEPTHLOAD, operationtype_tgfx STENCILOP, drawpassload_tgfx STENCILLOAD,
    float DEPTHCLEARVALUE, unsigned char STENCILCLEARVALUE) {
    return nullptr;
}
static rtslotusage_tgfx_handle CreateRTSlotUsage_Color(unsigned char SLOTINDEX, operationtype_tgfx OPTYPE, drawpassload_tgfx LOADTYPE){
    return nullptr;
}
static rtslotusage_tgfx_handle CreateRTSlotUsage_Depth(operationtype_tgfx DEPTHOP, drawpassload_tgfx DEPTHLOAD,
    operationtype_tgfx STENCILOP, drawpassload_tgfx STENCILLOAD) {
    return nullptr;
}
static depthsettings_tgfx_handle CreateDepthConfiguration(unsigned char ShouldWrite, depthtest_tgfx COMPAREOP) {
    return nullptr;
}
static stencilsettings_tgfx_handle CreateStencilConfiguration(unsigned char Reference, unsigned char WriteMask, unsigned char CompareMask,
    stencilcompare_tgfx CompareOP, stencilop_tgfx DepthFailOP, stencilop_tgfx StencilFailOP, stencilop_tgfx AllSuccessOP) {
    return nullptr;
}
static blendinginfo_tgfx_handle CreateBlendingConfiguration(unsigned char ColorSlotIndex, vec4_tgfx Constant, blendfactor_tgfx SRCFCTR_CLR,
    blendfactor_tgfx SRCFCTR_ALPHA, blendfactor_tgfx DSTFCTR_CLR, blendfactor_tgfx DSTFCTR_ALPHA, blendmode_tgfx BLENDOP_CLR,
    blendmode_tgfx BLENDOP_ALPHA, colorcomponents_tgfx WRITECHANNELs) {
    return nullptr;
}

//EXTENSION HELPERS

static void Destroy_ExtensionData(extension_tgfx_handle ExtensionToDestroy) {

}
static unsigned char DoesGPUsupportsVKDESCINDEXING(gpu_tgfx_handle GPU) { gpu_public* VKGPU = (gpu_public*)GPU; return VKGPU->EXTMANAGER()->ISSUPPORTED_DESCINDEXING(); }


extern void set_helper_functions() {
    core_tgfx_main->helpers->CreateBlendingConfiguration = &CreateBlendingConfiguration;
    core_tgfx_main->helpers->CreateDepthConfiguration = &CreateDepthConfiguration;
    core_tgfx_main->helpers->CreatePassWait_ComputePass = &CreatePassWait_ComputePass;
    core_tgfx_main->helpers->CreatePassWait_DrawPass = &CreatePassWait_DrawPass;
    core_tgfx_main->helpers->CreatePassWait_TransferPass = &CreatePassWait_TransferPass;
    core_tgfx_main->helpers->CreatePassWait_WindowPass = &CreatePassWait_WindowPass;
    core_tgfx_main->helpers->CreateRTSlotDescription_Color = &CreateRTSlotDescription_Color;
    core_tgfx_main->helpers->CreateRTSlotDescription_DepthStencil = &CreateRTSlotDescription_DepthStencil;
    core_tgfx_main->helpers->CreateRTSlotUsage_Color = &CreateRTSlotUsage_Color;
    core_tgfx_main->helpers->CreateRTSlotUsage_Depth = &CreateRTSlotUsage_Depth;
    core_tgfx_main->helpers->CreateShaderInputDescription = &CreateShaderInputDescription;
    core_tgfx_main->helpers->CreateStencilConfiguration = &CreateStencilConfiguration;
    core_tgfx_main->helpers->CreateSubDrawPassDescription = &CreateSubDrawPassDescription;
    core_tgfx_main->helpers->CreateTextureUsageFlag = &CreateTextureUsageFlag;
    core_tgfx_main->helpers->CreateWaitSignal_ComputeShader = &CreateWaitSignal_ComputeShader;
    core_tgfx_main->helpers->CreateWaitSignal_DrawIndirectConsume = &CreateWaitSignal_DrawIndirectConsume;
    core_tgfx_main->helpers->CreateWaitSignal_FragmentShader = &CreateWaitSignal_FragmentShader;
    core_tgfx_main->helpers->CreateWaitSignal_FragmentTests = &CreateWaitSignal_FragmentTests;
    core_tgfx_main->helpers->CreateWaitSignal_Transfer = &CreateWaitSignal_Transfer;
    core_tgfx_main->helpers->CreateWaitSignal_VertexInput = &CreateWaitSignal_VertexInput;
    core_tgfx_main->helpers->CreateWaitSignal_VertexShader = &CreateWaitSignal_VertexShader;
    core_tgfx_main->helpers->Create_GFXInitializationSecondStageInfo = &Create_GFXInitializationSecondStageInfo;
    core_tgfx_main->helpers->Destroy_ExtensionData = &Destroy_ExtensionData;
    core_tgfx_main->helpers->DoesGPUsupportsVKDESCINDEXING = &DoesGPUsupportsVKDESCINDEXING;
    core_tgfx_main->helpers->GetGPUInfo_General = &GetGPUInfo_General;
    core_tgfx_main->helpers->GetMonitor_Resolution_ColorBites_RefreshRate = &GetMonitor_Resolution_ColorBites_RefreshRate;
    core_tgfx_main->helpers->GetSupportedAllocations_ofTexture = &GetSupportedAllocations_ofTexture;
    core_tgfx_main->helpers->GetTextureTypeLimits = &GetTextureTypeLimits;
    core_tgfx_main->helpers->SetMemoryTypeInfo = &SetMemoryTypeInfo;
}