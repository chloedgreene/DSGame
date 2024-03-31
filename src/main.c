// SPDX-License-Identifier: CC0-1.0
//
// SPDX-FileContributor: Antonio Niño Díaz, 2023

#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <filesystem.h>
#include <maxmod9.h>
#include <nds.h>
#include <math.h>
#include <nds/arm9/trig_lut.h>

//Assets
#include "soundbank.h"
#include "blahaj.h"

//System Librarys
#include "chloe_filesystem.h"

#include <nds/arm9/video.h>
#include <nds/arm9/videoGL.h>
#include <nds/arm9/background.h>

////////////////
// DEBUG MODE //
////////////////
#define CHLOE_DEBUG_ASK_BEFORE_CHANGING
//Long story Short
//This make the Code for debug mode not even included in the game, so s p e e d


typedef struct {
    NE_Camera *Camera;
    NE_Model *Model;

    //Game State
    int frame_step;
} SceneData;

void Update3DScene(uint16_t keys,void *arg){

    SceneData *Scene = arg;
    Scene->frame_step++;
    float swim_state = sinLerp(Scene->frame_step * 300) / 150;
    float detail_state = sinLerp(Scene->frame_step * 450) >> 6;
    NE_ModelSetRot(Scene->Model,0, detail_state/18 ,(int)swim_state);

}

void Draw3DScene(void *arg)
{
    SceneData *Scene = arg;

    NE_ClearColorSet(RGB15(1,5,16),1,0);
    NE_CameraUse(Scene->Camera);
    NE_ModelDraw(Scene->Model);
}

void Init3DScene(void *arg){

    SceneData *Scene = arg;

    Scene->Model = NE_ModelCreate(NE_Static);
    Scene->Camera = NE_CameraCreate();
    Scene->frame_step = 0;
    NE_Material *Material = NE_MaterialCreate();
    NE_CameraSet(Scene->Camera,
                  -5, 0, 0,
                  0, 0, 0,
                  0, 1, 0);
    NE_ModelLoadStaticMeshFAT(Scene->Model, "blahaj_model.bin");
    NE_MaterialTexLoad(Material, NE_RGB5, 256, 256, NE_TEXGEN_TEXCOORD,blahajBitmap);
    NE_ModelSetMaterial(Scene->Model, Material);
    NE_ModelSetCoord(Scene->Model,0,0,0);
    NE_LightSet(0, NE_White, -0.5, -0.5, -0.5);

}

int main(int argc, char **argv)
{
    NE_Init3D();
    #ifdef CHLOE_DEBUG_ASK_BEFORE_CHANGING
        NE_TextureSystemReset(0, 0, NE_VRAM_AB); //Change VRAM bank so BANKC can be used for stuff
        consoleDemoInit();
    #endif
    WaitForNitro();

    irqEnable(IRQ_HBLANK);
    irqSet(IRQ_VBLANK, NE_VBLFunc);
    irqSet(IRQ_HBLANK, NE_HBLFunc);

    mmInitDefault("nitro:/soundbank.bin");
    mmLoad(MOD_ROLLINGDOWNTHESTREET);
    soundEnable();
    mmStart(MOD_ROLLINGDOWNTHESTREET, MM_PLAY_LOOP);


    SceneData Scene = { 0 };
    Init3DScene(&Scene);

    //printf("BETA | chloegreene.ca");

    #ifdef CHLOE_DEBUG_ASK_BEFORE_CHANGING
        int fpscount = 0;
        int oldsec = 0;
        int seconds = 0;
        int current_fps = 0;
    #endif
    while (1)
    {
        #ifdef CHLOE_DEBUG_ASK_BEFORE_CHANGING
        time_t unixTime = time(NULL);
        struct tm* timeStruct = gmtime((const time_t *)&unixTime);
        seconds = timeStruct->tm_sec;
        if (seconds != oldsec)
        {
            oldsec = seconds;
            current_fps = fpscount;
            fpscount = 0;
        }
        #endif
        NE_WaitForVBL(0);

        scanKeys();

        uint16_t keys_down = keysDown();


        Update3DScene(keys_down,&Scene);
        NE_ProcessArg(Draw3DScene, &Scene);


        #ifdef CHLOE_DEBUG_ASK_BEFORE_CHANGING
            printf("\x1B[2J\x1B[H");
            printf("==Debug Menu==\n");
            printf("Frame %d\n",Scene.frame_step);
            printf("FPS: %d\n",current_fps);
            printf("Frame Buffer Overflow %d",sizeof(int));
            fpscount++;
        #endif
    }

    soundDisable();

}