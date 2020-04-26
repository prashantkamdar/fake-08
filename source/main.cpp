

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <3ds.h>

#include <string>

#include "graphics.h"
#include "console.h"
#include "logger.h"

#include "input.h"

//this has the macro _TEST defined in it
#include "tests/test_base.h"


//3ds specific helper function
uint8_t ConvertInputToP8(u32 input){
	uint8_t result = 0;
	if (input & KEY_LEFT){
		result |= P8_KEY_LEFT;
	}

	if (input & KEY_RIGHT){
		result |= P8_KEY_RIGHT;
	}

	if (input & KEY_UP){
		result |= P8_KEY_UP;
	}

	if (input & KEY_DOWN){
		result |= P8_KEY_DOWN;
	}

	if (input & KEY_B){
		result |= P8_KEY_O;
	}

	if (input & KEY_A){
		result |= P8_KEY_X;
	}

	if (input & KEY_START){
		result |= P8_KEY_PAUSE;
	}

	if (input & KEY_SELECT){
		result |= P8_KEY_7;
	}

	return result;
}

void clear3dsFrameBuffer() {
	#if _TEST
	int bgcolor = 255;
	#else
	int bgcolor = 0;
	#endif
	uint8_t* fb = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);
	//clear whole top framebuffer
	memset(fb, bgcolor, 240*400*3);

}

u64 _prof_last_time = 0, _prof_now_time = 0, _prof_frame_time = 0;
double _prof_frameTimeMs;

void recordCodeTimeBlock(const char* name){
	//uncomment to write to console and log. Very slow on device (io bound?)
	/*
	_prof_now_time = svcGetSystemTick();
    _prof_frame_time = _prof_now_time - _prof_last_time;
	_prof_last_time = _prof_now_time;

	_prof_frameTimeMs = _prof_frame_time / CPU_TICKS_PER_MSEC;

	printf("block name: %s time in ms: %f\n", name, _prof_frameTimeMs);

	Logger::Write("block name: ", name, _prof_frameTimeMs);
	Logger::Write(name);
	Logger::Write(" ms: ");
	Logger::Write(std::to_string(_prof_frameTimeMs).c_str());
	Logger::Write("\n");
	*/
}

//3ds specific helper function
void postFlip3dsFunction() {
	gfxFlushBuffers();
	recordCodeTimeBlock("gfxFlushBuffers");
	gfxSwapBuffers();
	recordCodeTimeBlock("gfxSwapBuffers");
	gspWaitForVBlank();
	recordCodeTimeBlock("gspWaitForVBlank");
}



uint64_t getSystemTick(){
	return svcGetSystemTick();
}



double getTicksPerMs(){
	return CPU_TICKS_PER_MSEC;
}

int main(int argc, char* argv[])
{
	

	Logger::Initialize();
	Logger::Write("created Logger\n");
	gfxInitDefault();

	Logger::Write("gfxInitDefault()\n");

	Logger::Write("initializing Console\n");
	Console *console = new Console();
	Logger::Write("initialized console\n");

	//test or not both hardcoded to loading test cart as of now
	#if _TEST
	consoleInit(GFX_BOTTOM, NULL);

	Logger::Write("Loading cart\n");
	console->LoadCart("lilking.p8");
	Logger::Write("Cart Loaded\n");

	#else
	console.LoadCart("testcart.p8");
	#endif
	
	// Main loop
	Logger::Write("Starting main loop\n");

	uint8_t targetFps = console->GetTargetFps();
	double targetFrametimeMs = 1000.0 / (double)targetFps;

	std::function<void()> clearFb = clear3dsFrameBuffer;
	std::function<void()> postFlip = postFlip3dsFunction;

	u64 last_time = 0, now_time = 0, frame_time = 0;

	recordCodeTimeBlock("startup");

	while (aptMainLoop())
	{
		//Scan all the inputs. This should be done once for each frame
		hidScanInput();

		now_time = svcGetSystemTick();
     	frame_time = now_time - last_time;
		last_time = now_time;

		double frameTimeMs = frame_time / CPU_TICKS_PER_MSEC;

		#if _TEST
		consoleClear();
		printf("svcGetSystemTick(): %lld \n", now_time);
		printf("frame time (ticks): %lld \n", frame_time);
		printf("frame time (ms): %f \n", frameTimeMs);
		printf("target fps: %d \n", targetFps);
		printf("target frame time (ms): %f \n", targetFrametimeMs);
		#endif

		//sleep for remainder of time
		if (frameTimeMs < targetFrametimeMs) {
			double msToSleep = targetFrametimeMs - frameTimeMs;
			
			#if _TEST
			printf("sleeping for : %f ms\n", msToSleep);
			#endif

			svcSleepThread(msToSleep * 1000 * 1000);

			last_time += CPU_TICKS_PER_MSEC * msToSleep;
		}
		
		
		//hidKeysDown returns information about which buttons have been just pressed (and they weren't in the previous frame)
		u32 kDown = hidKeysDown();
		u32 kHeld = hidKeysHeld();

		bool lpressed = kHeld & KEY_L;
		bool rpressed = kHeld & KEY_R;

		if (lpressed && rpressed) break; // break in order to return to hbmenu

		uint8_t p8kDown = ConvertInputToP8(kDown);
		uint8_t p8kHeld = ConvertInputToP8(kHeld);

		//_update();
		

		//uint8_t* fb_b = gfxGetFramebuffer(GFX_BOTTOM, GFX_LEFT, NULL, NULL);
		//clear whole framebuffer
		//memset(fb_b, bgcolor, 240*320*3);

		//cart draw
		//_draw();

		recordCodeTimeBlock("PreUpdate");

		console->UpdateAndDraw(frame_time, clear3dsFrameBuffer, p8kDown, p8kHeld, recordCodeTimeBlock);

		//send pico 8 screen to framebuffer, then call the function to flush and swap buffers, and wait for vblank
		
		uint8_t* fb = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);

		recordCodeTimeBlock("GetFb");

		console->FlipBuffer(fb, postFlip, recordCodeTimeBlock);
	}


	Logger::Write("Turning off console and exiting logger\n");
	console->TurnOff();
	delete console;
	Logger::Exit();
	gfxExit();
	return 0;
}


