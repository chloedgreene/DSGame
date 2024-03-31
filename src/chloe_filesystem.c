#include "chloe_filesystem.h"
void WaitForNitro(){

    bool init_ok = nitroFSInit(NULL);
    if (!init_ok)
    {
        NE_TextureSystemReset(0, 0, NE_VRAM_AB); //Change VRAM bank so BANKC can be used for stuff
        consoleDemoInit();
        perror("nitroFSInit()");
        while (1)
        {
            swiWaitForVBlank();
            uint16_t keys = keysDown();
            if (keys & KEY_START)
                return 0;
        }
    }
    
}