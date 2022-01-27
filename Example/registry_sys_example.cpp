#include <iostream>
#include <string>
#include <windows.h>

#include "registrysys_tapi.h"
#include "virtualmemorysys_tapi.h"
#include "unittestsys_tapi.h"
#include "array_of_strings_tapi.h"
#include "threadingsys_tapi.h"
#include "filesys_tapi.h"
#include "logger_tapi.h"
#include "bitset_tapi.h"
#include "profiler_tapi.h"
#include "allocator_tapi.h"


#include "tgfx_forwarddeclarations.h"
#include "tgfx_helper.h"
#include "tgfx_core.h"
#include "tgfx_renderer.h"
#include "tgfx_gpucontentmanager.h"


#include <stdint.h>

THREADINGSYS_TAPI_PLUGIN_LOAD_TYPE threadingsys = NULL;
char stop_char;

unsigned char unittest_func(const char** output_string, void* data){
	*output_string = "Unit test in registry_sys_example has run!";
	return 0;
}

void thread_print(){
	while(stop_char != '1'){
		printf("This thread index: %u\n", threadingsys->funcs->this_thread_index());
	}
	Sleep(2000);
	printf("Finished thread index: %u\n", threadingsys->funcs->this_thread_index());
}
void printf_log_tgfx(result_tgfx result, const char* text) {
	printf("TGFX Result %u: %s\n", (unsigned int)result, text);
}

int main(){
	auto registrydll = DLIB_LOAD_TAPI("tapi_registrysys.dll");
	if(registrydll == NULL){
		printf("Registry System DLL load failed!");
	}
	load_plugin_func loader = (load_plugin_func)DLIB_FUNC_LOAD_TAPI(registrydll, "load_plugin");
	registrysys_tapi* sys = ((registrysys_tapi_type*)loader(NULL, 0))->funcs;
	const char* first_pluginname = sys->list(NULL)[0];
	virtualmemorysys_tapi* virmemsys = ((VIRTUALMEMORY_TAPI_PLUGIN_TYPE)sys->get("tapi_virtualmemsys", MAKE_PLUGIN_VERSION_TAPI(0,0,0), 0))->funcs;

	//Generally, I recommend you to load unit test system after registry system
	//Because other systems may implement unit tests but if unittestsys isn't loaded, they can't implement them.
	auto unittestdll = DLIB_LOAD_TAPI("tapi_unittest.dll");
	load_plugin_func unittestloader = (load_plugin_func)DLIB_FUNC_LOAD_TAPI(unittestdll, "load_plugin");
	UNITTEST_TAPI_LOAD_TYPE unittest = (UNITTEST_TAPI_LOAD_TYPE)unittestloader(sys, 0);
	unittest_interface_tapi x;
	x.test = &unittest_func;
	unittest->add_unittest("naber", 0, x);

	//Create an array of strings in a 1GB of reserved virtual address space
	void* virmem_loc = virmemsys->virtual_reserve(1 << 30);
	auto aos_sys = DLIB_LOAD_TAPI("tapi_array_of_strings_sys.dll");
	load_plugin_func aosloader = (load_plugin_func)DLIB_FUNC_LOAD_TAPI(aos_sys, "load_plugin");
	ARRAY_OF_STRINGS_TAPI_LOAD_TYPE aos = (ARRAY_OF_STRINGS_TAPI_LOAD_TYPE)aosloader(sys, 0);
	array_of_strings_tapi* first_array = aos->virtual_allocator->create_array_of_string(virmem_loc, 1 << 30, 0);
	aos->virtual_allocator->change_string(first_array, 0, "Naber?");
	printf("%s", aos->virtual_allocator->read_string(first_array, 0));
	aos->virtual_allocator->change_string(first_array, 0, "FUCK");
	printf("%s", aos->virtual_allocator->read_string(first_array, 0));
	unittest->run_tests(0xFFFFFFFF, NULL, NULL);

	auto threadingsysdll = DLIB_LOAD_TAPI("tapi_threadedjobsys.dll");
	load_plugin_func threadingloader = (load_plugin_func)DLIB_FUNC_LOAD_TAPI(threadingsysdll, "load_plugin");
	threadingsys = (THREADINGSYS_TAPI_PLUGIN_LOAD_TYPE)threadingloader(sys, 0);
	
	auto filesysdll = DLIB_LOAD_TAPI("tapi_filesys.dll");
	load_plugin_func filesysloader = (load_plugin_func)DLIB_FUNC_LOAD_TAPI(filesysdll, "load_plugin");
	FILESYS_TAPI_PLUGIN_LOAD_TYPE filesys = (FILESYS_TAPI_PLUGIN_LOAD_TYPE)filesysloader(sys, 0);
	filesys->funcs->write_textfile("naber?", "first.txt", false);
	
	auto loggerdll = DLIB_LOAD_TAPI("tapi_logger.dll");
	load_plugin_func loggerloader = (load_plugin_func)DLIB_FUNC_LOAD_TAPI(loggerdll, "load_plugin");
	LOGGER_TAPI_PLUGIN_LOAD_TYPE loggersys = (LOGGER_TAPI_PLUGIN_LOAD_TYPE)loggerloader(sys, 0);
	loggersys->funcs->log_status("First log!");

	auto bitsetsysdll = DLIB_LOAD_TAPI("tapi_bitset.dll");
	load_plugin_func bitsetloader = (load_plugin_func)DLIB_FUNC_LOAD_TAPI(bitsetsysdll, "load_plugin");
	BITSET_TAPI_PLUGIN_LOAD_TYPE bitsetsys = (BITSET_TAPI_PLUGIN_LOAD_TYPE)bitsetloader(sys, 0);
	bitset_tapi* firstbitset = bitsetsys->funcs->create_bitset(100);
	bitsetsys->funcs->setbit_true(firstbitset, 5);
	printf("First true bit: %u and bitset size: %u\n", bitsetsys->funcs->getindex_firsttrue(firstbitset), bitsetsys->funcs->getbyte_length(firstbitset));

	auto profilersysdll = DLIB_LOAD_TAPI("tapi_profiler.dll");
	load_plugin_func profilersysloader = (load_plugin_func)DLIB_FUNC_LOAD_TAPI(profilersysdll, "load_plugin");
	PROFILER_TAPI_PLUGIN_LOAD_TYPE profilersys = (PROFILER_TAPI_PLUGIN_LOAD_TYPE)profilersysloader(sys, 0);


	auto tgfxdll = DLIB_LOAD_TAPI("tgfx_core.dll");
	load_plugin_func tgfxloader = (load_plugin_func)DLIB_FUNC_LOAD_TAPI(tgfxdll, "load_plugin");
	TGFX_PLUGIN_LOAD_TYPE tgfxsys = (TGFX_PLUGIN_LOAD_TYPE)tgfxloader(sys, 0);
	result_tgfx result = tgfxsys->api->load_backend(backends_tgfx_VULKAN, nullptr);
	printf("%c", result);

	gpu_tgfx_listhandle gpulist;
	tgfxsys->api->getGPUlist(&gpulist);
	TGFXLISTCOUNT(tgfxsys->api, gpulist, gpulist_count);
	printf("GPU COUNT: %u\n", gpulist_count);
	const char* gpuname;
	const memory_description_tgfx* memtypelist; unsigned int memtypelistcount;
	tgfxsys->api->helpers->GetGPUInfo_General(gpulist[0], &gpuname, nullptr, nullptr, nullptr, &memtypelist, &memtypelistcount, nullptr, nullptr, nullptr);
	std::cout << "Memory Type Count: " << memtypelistcount << " and GPU Name: " << gpuname << "\n";
	unsigned int devicelocalmemtype_id = UINT32_MAX, fastvisiblememtype_id = UINT32_MAX;
	for (unsigned int i = 0; i < memtypelistcount; i++) {
		tgfxsys->api->helpers->SetMemoryTypeInfo(memtypelist[i].memorytype_id, 10 * 1024 * 1024, nullptr);
		if (memtypelist[i].allocationtype == memoryallocationtype_DEVICELOCAL) { devicelocalmemtype_id = memtypelist[i].memorytype_id; }
		if (memtypelist[i].allocationtype == memoryallocationtype_FASTHOSTVISIBLE) { fastvisiblememtype_id = memtypelist[i].memorytype_id; }
	}
	initializationsecondstageinfo_tgfx_handle secondinfo = tgfxsys->api->helpers->Create_GFXInitializationSecondStageInfo(gpulist[0], 10, 100, 100, 100, 100, 100, 100, 100, 100, true, true, true, (extension_tgfx_listhandle)tgfxsys->api->INVALIDHANDLE);
	tgfxsys->api->initialize_secondstage(secondinfo);
	monitor_tgfx_listhandle monitorlist;
	tgfxsys->api->getmonitorlist(&monitorlist);
	TGFXLISTCOUNT(tgfxsys->api, monitorlist, monitorcount);
	printf("Monitor Count: %u", monitorcount);
	textureusageflag_tgfx_handle usageflag = tgfxsys->api->helpers->CreateTextureUsageFlag(true, true, true, true, true);
	texture_tgfx_handle swapchain_textures[2];
	window_tgfx_handle firstwindow;
	tgfxsys->api->create_window(1280, 720, monitorlist[0], windowmode_tgfx_WINDOWED, "RegistrySys Example", usageflag, nullptr, nullptr, swapchain_textures, &firstwindow);
	tgfxsys->api->renderer->Start_RenderGraphConstruction();
	transferpass_tgfx_handle firstbarriertp;
	tgfxsys->api->renderer->Create_TransferPass((passwaitdescription_tgfx_listhandle)&tgfxsys->api->INVALIDHANDLE, transferpasstype_tgfx_BARRIER, "FirstBarrierTP", &firstbarriertp);

	texture_tgfx_handle first_rt, second_rt;
	textureusageflag_tgfx_handle usageflag_tgfx = tgfxsys->api->helpers->CreateTextureUsageFlag(false, true, false, true, false);
	unsigned int WIDTH, HEIGHT, MIP = 0;
	tgfxsys->api->helpers->GetTextureTypeLimits(texture_dimensions_tgfx_2D, texture_order_tgfx_SWIZZLE, texture_channels_tgfx_RGBA8UB, usageflag_tgfx, gpulist[0], &WIDTH, &HEIGHT, nullptr, &MIP);
	tgfxsys->api->contentmanager->Create_Texture(texture_dimensions_tgfx_2D, 1280, 720, texture_channels_tgfx_RGBA8UB, 1, usageflag_tgfx, texture_order_tgfx_SWIZZLE, devicelocalmemtype_id, &first_rt);
	tgfxsys->api->contentmanager->Create_Texture(texture_dimensions_tgfx_2D, 1280, 720, texture_channels_tgfx_RGBA8UB, 1, usageflag_tgfx, texture_order_tgfx_SWIZZLE, devicelocalmemtype_id, &second_rt);



	vec4_tgfx white; white.x = 255; white.y = 255; white.z = 255; white.w = 0;
	rtslotusage_tgfx_handle rtslotusages[2] = { tgfxsys->api->helpers->CreateRTSlotUsage_Color(0, operationtype_tgfx_READ_AND_WRITE, drawpassload_tgfx_CLEAR), (rtslotusage_tgfx_handle)tgfxsys->api->INVALIDHANDLE };
	rtslotdescription_tgfx_handle rtslot_descs[2] = { 
		tgfxsys->api->helpers->CreateRTSlotDescription_Color(first_rt, second_rt, operationtype_tgfx_READ_AND_WRITE, drawpassload_tgfx_CLEAR, true, 0, white), (rtslotdescription_tgfx_handle)tgfxsys->api->INVALIDHANDLE};
	rtslotset_tgfx_handle rtslotset_handle;
	tgfxsys->api->contentmanager->Create_RTSlotset(rtslot_descs, &rtslotset_handle);
	inheritedrtslotset_tgfx_handle irtslotset; 
	tgfxsys->api->contentmanager->Inherite_RTSlotSet(rtslotusages, rtslotset_handle, &irtslotset);
	subdrawpassdescription_tgfx_handle subdp_descs[2] = {	tgfxsys->api->helpers->CreateSubDrawPassDescription(irtslotset, subdrawpassaccess_tgfx_ALLCOMMANDS, subdrawpassaccess_tgfx_ALLCOMMANDS), (subdrawpassdescription_tgfx_handle)tgfxsys->api->INVALIDHANDLE};
	passwaitdescription_tgfx_handle dp_waits[2] = {			tgfxsys->api->helpers->CreatePassWait_TransferPass(&firstbarriertp, transferpasstype_tgfx_BARRIER, 
		tgfxsys->api->helpers->CreateWaitSignal_Transfer(true, true), false), (passwaitdescription_tgfx_handle)tgfxsys->api->INVALIDHANDLE};
	subdrawpass_tgfx_handle sub_dp;
	drawpass_tgfx_handle dp;
	tgfxsys->api->renderer->Create_DrawPass(subdp_descs, rtslotset_handle, dp_waits, "First DP", &sub_dp, &dp);
	tgfxsys->api->renderer->Finish_RenderGraphConstruction(nullptr);

	while (true) {
		profiledscope_handle_tapi scope;	unsigned long long duration;
		profilersys->funcs->start_profiling(&scope, "Run Loop", &duration, 1);
		tgfxsys->api->renderer->Run();
		profilersys->funcs->finish_profiling(&scope, 1);
	}
	printf("Application is finished!");

	return 1;
}