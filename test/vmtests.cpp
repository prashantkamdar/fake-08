#include <string>
#include <vector>

#include "doctest.h"

#include "../source/vm.h"
#include "../source/hostVmShared.h"
#include "../source/host.h"
#include "stubhost.h"

#include "../source/fontdata.h"


TEST_CASE("Vm memory functions") {
    StubHost* stubHost = new StubHost();
    PicoRam* memory = new PicoRam();
    Graphics* graphics = new Graphics(get_font_data(), memory);
    Input* input = new Input(memory);
    Audio* audio = new Audio(memory);

    Vm* vm = new Vm(stubHost, memory, graphics, input, audio);

    SUBCASE("memory data stats with 0"){
        CHECK(memory->data[0] == 0);
    }
    SUBCASE("resetting memory zeroes out everything"){
        for(int i = 0; i < 0x8000; ++i) {
            memory->data[i] = i & 255;
        }

        memory->Reset();

        bool allZeroes = true;
        for(int i = 0; i < 0x8000; ++i) {
            allZeroes &= memory->data[i] == 0;
        }

        CHECK(allZeroes);
    }
    SUBCASE("size of PicoRam struct corrct"){
        CHECK(sizeof(PicoRam) == 0x8000);
    }
    SUBCASE("size of data correct"){
        CHECK(sizeof(memory->data) == 0x8000);
    }
    SUBCASE("simple peek and poke"){
        vm->vm_poke(2, 232);

        CHECK(vm->vm_peek(2) == 232);
    }
    SUBCASE("simple peek2 and poke2"){
        vm->vm_poke2(41, -1031);

        CHECK(vm->vm_peek2(41) == -1031);
    }
    SUBCASE("simple peek4 and poke4"){
        vm->vm_poke4(0x7123, 49249);

        CHECK(vm->vm_peek4(0x7123) == 49249);
    }
    SUBCASE("poking spritesheet"){
        //147: 1001 0011
        //left pixel: 0011 = 3
        //right pixel: 1001 = 9
        vm->vm_poke(0, 147);

        CHECK_EQ(graphics->sget(0, 0), 3);
        CHECK_EQ(graphics->sget(1, 0), 9);
    }
    SUBCASE("poking map"){
        vm->vm_poke(0x2000, 201);
        vm->vm_poke(0x2003, 11);

        CHECK_EQ(graphics->mget(0, 0), 201);
        CHECK_EQ(graphics->mget(3, 0), 11);
    }
    SUBCASE("poking sprite flags"){
        //39: 0010 0111
        vm->vm_poke(0x3000, 39);

        CHECK_EQ(graphics->fget(0), 39);
        CHECK_EQ(graphics->fget(0, 0), true);
        CHECK_EQ(graphics->fget(0, 3), false);
    }
    SUBCASE("poking music"){
        //78: 0100 1110
        vm->vm_poke(0x3100, 78);

        CHECK_EQ(memory->songs[0].sfx0, 78);
        CHECK_EQ(memory->songs[0].start, 0);
    }
    SUBCASE("poking sfx"){
        //97: 0110 0001
        //key is first 6 bits: 10 0001 : 33
        //waveform is next 3: (0) 01 : 1
        vm->vm_poke(0x3200, 97);

        CHECK_EQ(memory->sfx[0].notes[0].key, 33);
        CHECK_EQ(memory->sfx[0].notes[0].waveform, 1);
    }
    SUBCASE("poking general use ram"){
        vm->vm_poke(0x4300, 215);

        CHECK_EQ(memory->generalUseRam[0], 215);
    }
    SUBCASE("setting cart data"){
        vm->vm_cartdata("dummy");
        vm->vm_dset(13, 56);

        CHECK_EQ(vm->vm_dget(13), 56);
    }
    SUBCASE("poking cart data"){
        vm->vm_poke(0x5e00, 56);
        vm->vm_cartdata("dummy");

        CHECK_EQ(memory->cartData[0], 56);
        CHECK_EQ(vm->vm_dget(0), 56);
    }
    SUBCASE("poking draw palette data"){
        //21 : 0001 0101 (transparet: true) (color mapped 5)
        vm->vm_poke(0x5f02, 21);

        CHECK_EQ(memory->drawState.drawPaletteMap[2], 21);
        CHECK_EQ(graphics->getDrawPalMappedColor(2), 5);
        CHECK_EQ(graphics->isColorTransparent(2), true);
    }
    SUBCASE("poking clip data"){
        //7 : 0000 0111
        vm->vm_poke(0x5f21, 89);

        CHECK_EQ(memory->drawState.clip_yb, 89);
    }
    SUBCASE("poking unknown ram at 0x5f24"){
        //7 : 0000 0111
        vm->vm_poke(0x5f24, 254);

        CHECK_EQ(memory->drawState.unknown05f24, 254);
    }
    SUBCASE("poking color"){
        vm->vm_poke(0x5f25, 12);

        CHECK_EQ(memory->drawState.color, 12);
    }
    SUBCASE("poking text cursor"){
        vm->vm_poke(0x5f26, 74);

        CHECK_EQ(memory->drawState.text_x, 74);
    }
    SUBCASE("poking camera position"){
        vm->vm_poke2(0x5f2a, -72);

        CHECK_EQ(memory->drawState.camera_y, -72);
    }
    SUBCASE("poking draw mode"){
        vm->vm_poke(0x5f2c, 2);

        CHECK_EQ(memory->drawState.drawMode, 2);
    }
    SUBCASE("poking dev kit mode"){
        vm->vm_poke(0x5f2d, 2);

        CHECK_EQ(memory->drawState.devkitMode, 2);
    }
    SUBCASE("poking persist palette"){
        vm->vm_poke(0x5f2e, 1);

        CHECK_EQ(memory->drawState.persistPalette, 1);
    }
    SUBCASE("poking sound pause state"){
        vm->vm_poke(0x5f2f, 4);

        CHECK_EQ(memory->drawState.soundPauseState, 4);
    }
    SUBCASE("poking suppress pause"){
        vm->vm_poke(0x5f30, 1);

        CHECK_EQ(memory->drawState.suppressPause, 1);
    }
    SUBCASE("poking fill pattern"){
        vm->vm_poke(0x5f31, 24);

        CHECK_EQ(memory->drawState.fillPattern[0], 24);
    }
    SUBCASE("poking fill pattern transparency bit"){
        vm->vm_poke(0x5f33, 1);

        CHECK_EQ(memory->drawState.fillPatternTransparencyBit, 1);
    }
    SUBCASE("poking color setting flag"){
        /*
-- bit  0x1000.0000 means the non-colour bits should be observed
-- bit  0x0100.0000 transparency bit
-- bits 0x00FF.0000 are the usual colour bits
-- bits 0x0000.FFFF are interpreted as the fill pattern
        */
        vm->vm_poke(0x5f34, 36);

        CHECK_EQ(memory->drawState.colorSettingFlag, 36);
    }
    SUBCASE("poking line invalid"){
        vm->vm_poke(0x5f35, 1);

        CHECK_EQ(memory->drawState.lineInvalid, 1);
    }
    SUBCASE("poking tline width"){
        vm->vm_poke(0x5f38, 65);

        CHECK_EQ(memory->drawState.tlineMapWidth, 65);
    }
    SUBCASE("poking tline height"){
        vm->vm_poke(0x5f39, 66);

        CHECK_EQ(memory->drawState.tlineMapHeight, 66);
    }
    SUBCASE("poking tline x"){
        vm->vm_poke(0x5f3a, 67);

        CHECK_EQ(memory->drawState.tlineMapXOffset, 67);
    }
    SUBCASE("poking tline y"){
        vm->vm_poke(0x5f3b, 68);

        CHECK_EQ(memory->drawState.tlineMapYOffset, 68);
    }
    SUBCASE("poking line x"){
        vm->vm_poke2(0x5f3c, -3);

        CHECK_EQ(memory->drawState.line_x, -3);
    }
    SUBCASE("poking line y"){
        vm->vm_poke2(0x5f3e, 262);

        CHECK_EQ(memory->drawState.line_y, 262);
    }
    SUBCASE("poking audio hardware state") {
        vm->vm_poke(0x5f40, 3);

        CHECK_EQ(memory->hwState.audioHardwareState[0], 3);
    }
    SUBCASE("poking rng state") {
        //rng gets seeded, we need to zero it out to test this easier
        memory->hwState.rngState[0] = 0;
        vm->vm_poke(0x5f44, 20);

        CHECK_EQ(memory->hwState.rngState[0], 20);
    }
    SUBCASE("poking button state") {
        //0000 1010
        vm->vm_poke(0x5f4c, 10);

        CHECK_EQ(memory->hwState.buttonStates[0], 10);
        CHECK_EQ(input->btn(), 10);
        CHECK_EQ(input->btn(0), false);
        CHECK_EQ(input->btn(1), true);
        CHECK_EQ(input->btn(2), false);
        CHECK_EQ(input->btn(3), true);
    }
    SUBCASE("poking unknown input block") {
        vm->vm_poke(0x5f54, 53);

        CHECK_EQ(memory->hwState.unknownInputBlock[0], 53);
    }
    SUBCASE("poking btnp repeat delay") {
        vm->vm_poke(0x5f5c, 10);

        CHECK_EQ(memory->hwState.btnpRepeatDelay, 10);
    }
    SUBCASE("poking btnp repeat interval") {
        vm->vm_poke(0x5f5d, 25);

        CHECK_EQ(memory->hwState.btnpRepeatInterval, 25);
    }
    SUBCASE("poking color bitmask") {
        vm->vm_poke(0x5f5e, 42);

        CHECK_EQ(memory->hwState.colorBitmask, 42);
    }
    SUBCASE("poking alternate palette flag") {
        vm->vm_poke(0x5f5f, 10);

        CHECK_EQ(memory->hwState.alternatePaletteFlag, 10);
    }
    SUBCASE("poking alternate palette map") {
        vm->vm_poke(0x5f60, 11);

        CHECK_EQ(memory->hwState.alternatePaletteMap[0], 11);
    }
    SUBCASE("poking alternate palette map") {
        vm->vm_poke(0x5f70, 7);

        CHECK_EQ(memory->hwState.alternatePaletteScreenLineBitfield[0], 7);
    }
    SUBCASE("poking gpio pins") {
        vm->vm_poke(0x5f80, 192);

        CHECK_EQ(memory->hwState.gpioPins[0], 192);
    }
    SUBCASE("poking screen data (first two pixels") {
        //195: 1100 0011 (left pixel 3, right pixel 12)
        vm->vm_poke(0x6000, 195);

        CHECK_EQ(memory->screenBuffer[0], 195);
        CHECK_EQ(graphics->pget(0, 0), 3);
        CHECK_EQ(graphics->pget(1, 0), 12);
    }
    SUBCASE("poking screen data (last two pixels") {
        //210: 1101 0010
        vm->vm_poke(0x7fff, 210);

        CHECK_EQ(memory->screenBuffer[8191], 210);
        CHECK_EQ(graphics->pget(126, 127), 2);
        CHECK_EQ(graphics->pget(127, 127), 13);
    }
    SUBCASE("Loading cart copies cart rom to memory") {
        vm->LoadCart("carts/cartparsetest.p8.png");

        SUBCASE("Gfx data is populated") {
            CHECK(memory->spriteSheetData[0] == 255);
            CHECK(memory->spriteSheetData[1] == 1);
            CHECK(memory->spriteSheetData[2] == 2);
            CHECK(memory->spriteSheetData[3] == 3);
        }
        SUBCASE("Sprite flags is populated") {
            CHECK(memory->spriteFlags[0] == 0);
            CHECK(memory->spriteFlags[1] == 1);
        }
        SUBCASE("Map data is populated") {
            CHECK(memory->mapData[0] == 1);
            CHECK(memory->mapData[1] == 0);
        }
        SUBCASE("Sfx data is populated") {
            CHECK(memory->sfx[0].data[0] == 13);
            CHECK(memory->sfx[0].editorMode == 0);
            CHECK(memory->sfx[0].loopRangeEnd == 0);
            CHECK(memory->sfx[0].loopRangeEnd == 0);
            CHECK(memory->sfx[0].speed == 29);

            CHECK(memory->sfx[0].notes[0].waveform == 0);
            CHECK(memory->sfx[0].notes[0].effect == 0);
            CHECK(memory->sfx[0].notes[0].key == 13);
            CHECK(memory->sfx[0].notes[0].volume == 3);
            CHECK(memory->sfx[0].notes[0].data[0] == 13);
            CHECK(memory->sfx[0].notes[0].data[1] == 6);
        }
        SUBCASE("Music data is populated") {
            CHECK(memory->songs[0].sfx0 == 1);
            CHECK(memory->songs[0].start == 0);
            CHECK(memory->songs[0].sfx1 == 66);
            CHECK(memory->songs[0].loop == 0);
            CHECK(memory->songs[0].sfx2 == 67);
            CHECK(memory->songs[0].stop == 0);
            CHECK(memory->songs[0].sfx3 == 68);
            CHECK(memory->songs[0].mode == 0);
        }
    }
    SUBCASE("reload writes cart rom over changes") {
        vm->LoadCart("carts/cartparsetest.p8.png");

        graphics->sset(0, 0, 4);
        graphics->fset(0, 3);
        graphics->mset(0, 0, 10);

        vm->vm_reload(0, 0, 0x4300, "");

        CHECK_EQ(graphics->sget(0, 0), 15);
        CHECK_EQ(graphics->fget(0), 0);
        CHECK_EQ(graphics->mget(0, 0), 1);
    }
    SUBCASE("memset sets memory") {
        vm->vm_memset(0x3000, 13, 4);

        CHECK_EQ(graphics->fget(0), 13);
        CHECK_EQ(graphics->fget(3), 13);
    }
    SUBCASE("memcpy copies memory") {
        vm->LoadCart("carts/cartparsetest.p8.png");
        vm->vm_memcpy(0x3000, 0, 4);

        CHECK_EQ(memory->spriteSheetData[1], 1);
    }
    SUBCASE("dget before setting cart data key does nothing") {
        vm->vm_dset(34, 1923);

        CHECK_EQ(vm->vm_dget(34), 0);
    }
    SUBCASE("dget after setting cart data key stores value") {
        vm->vm_cartdata("uniquekey");
        vm->vm_dset(34, 1923);

        CHECK_EQ(vm->vm_dget(34), 1923);
    }
    SUBCASE("pico driller style input"){
        vm->LoadCart("carts/drillerinputtest.p8");

        SUBCASE("no buttons gives 0"){
            vm->UpdateAndDraw();
            bool btnbits = vm->ExecuteLua(
                "function btnbitstest0()\n"
                " return btnbits == 0\n"
                "end\n",
                "btnbitstest0");

            CHECK(btnbits);
        }
        SUBCASE("left pushed returns 1"){
            stubHost->stubInput(1, 1);
            vm->UpdateAndDraw();
            bool btnbits = vm->ExecuteLua(
                "function btnbitstest1()\n"
                " return btnbits == 1\n"
                "end\n",
                "btnbitstest1");

            CHECK(btnbits);
        }
        SUBCASE("right pushed returns 2"){
            stubHost->stubInput(2, 2);
            vm->UpdateAndDraw();
            bool btnbits = vm->ExecuteLua(
                "function btnbitstest2()\n"
                " return btnbits == 2\n"
                "end\n",
                "btnbitstest2");

            CHECK(btnbits);
        }
        SUBCASE("right pushed detected by band op"){
            stubHost->stubInput(2, 2);
            vm->UpdateAndDraw();
            bool btnbits = vm->ExecuteLua(
                "function rightbandtest()\n"
                " return rightband == 2\n"
                "end\n",
                "rightbandtest");

            CHECK(btnbits);
        }
        vm->CloseCart();
    }

    
    delete graphics;
    delete input;
    delete audio;

    delete vm;

    delete memory;
}