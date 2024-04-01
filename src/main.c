////////////////
// DEBUG MODE //
////////////////
#define CHLOE_DEBUG_ASK_BEFORE_CHANGING
//Long story Short
//This make the Code for debug mode not even included in the game, so s p e e d


#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <filesystem.h>
#include <maxmod9.h>
#include <nds.h>
#include <math.h>
#include <nds/arm9/trig_lut.h>

//Assets
#include "soundbank.h"
#include "blahaj.h"
#include "block_pallette.h"
#ifdef CHLOE_DEBUG_ASK_BEFORE_CHANGING
#include "debug.h"
#endif

//System Librarys
#include "chloe_filesystem.h"
#include "chloe_libmap_fix.h"
#include "chloe_render_atom.h"
#include "map_data.h"
#include "map_parser.h"
#include "geo_generator.h"
#include "surface_gatherer.h"

#include <nds/arm9/video.h>
#include <nds/arm9/videoGL.h>
#include <nds/arm9/background.h>


typedef struct {
    NE_Camera *Camera;
    NE_Model *Model;
    AtomModel *surfaces;  // Array of surfaces
    int surface_count;

    //Game State
    unsigned int frame_step;
} SceneData;

void Update3DScene(uint16_t keys,void *arg){

    SceneData *Scene = arg;
    Scene->frame_step++;
    float swim_state = sinLerp(Scene->frame_step * 300) >> 7;
    float detail_state = sinLerp(Scene->frame_step * 450) >> 6; // Bit shift for FAST Dividing because this is a old little system 
    NE_ModelSetRot(Scene->Model,0, detail_state/18 ,(int)swim_state);
    NE_ViewRotate(Scene->frame_step,0,0);
    float RotateX = sinLerp(Scene->frame_step * 300) >> 8;
    float RotateY = cosLerp(Scene->frame_step * 300) >> 8;

    NE_CameraSet(Scene->Camera,
                  RotateX,0,RotateY,
                  0, 0, 0,
                  0, 1, 0);

}

void Draw3DScene(void *arg)
{
    SceneData *Scene = arg;
    NE_ClearColorSet(RGB15(1,5,16),1,0);
    NE_CameraUse(Scene->Camera);
    NE_PolyFormat(31,1,NE_LIGHT_0,NE_CULL_BACK,0);
    NE_ModelDraw(Scene->Model);

    for (int s = 0; s < Scene->surface_count; ++s) {
        AtomModel *surf = &Scene->surfaces[s];
        NE_PolyBegin(GL_QUAD_STRIP); // Adjust this based on your requirement
        
            for (int v = 0; v < surf->vertex_count; v ++) {
            // Draw each vertex of the triangle
                NE_PolyVertex(surf->arr[v].x,   surf->arr[v].y,   surf->arr[v].z);
            }
        
        NE_PolyEnd();
    }

}

// Define a hash function for vertices
uint32_t hashVertex(const AtomVertex *v) {
    // Combine the coordinates to create a unique hash value
    uint32_t hash = 0;
    hash ^= v->x + 0x9e3779b9 + (hash << 6) + (hash >> 2);
    hash ^= v->y + 0x9e3779b9 + (hash << 6) + (hash >> 2);
    hash ^= v->z + 0x9e3779b9 + (hash << 6) + (hash >> 2);
    return hash;
}

// Define a function to compare vertices for equality
bool equalVertices(const AtomVertex *v1, const AtomVertex *v2) {
    return v1->x == v2->x && v1->y == v2->y && v1->z == v2->z;
}

// Structure to represent a hash set
typedef struct HashSet {
    AtomVertex *keys;
    bool *filled;
    int capacity;
} HashSet;

// Initialize a hash set
HashSet initHashSet(int capacity) {
    HashSet set;
    set.keys = malloc(capacity * sizeof(AtomVertex));
    set.filled = calloc(capacity, sizeof(bool));
    set.capacity = capacity;
    return set;
}

// Insert a vertex into the hash set
void insertIntoHashSet(HashSet *set, const AtomVertex *v) {
    uint32_t hash = hashVertex(v) % set->capacity;
    while (set->filled[hash]) {
        hash = (hash + 1) % set->capacity;
    }
    set->keys[hash] = *v;
    set->filled[hash] = true;
}

// Check if a vertex exists in the hash set
bool existsInHashSet(const HashSet *set, const AtomVertex *v) {
    uint32_t hash = hashVertex(v) % set->capacity;
    while (set->filled[hash]) {
        if (equalVertices(v, &set->keys[hash])) {
            return true;
        }
        hash = (hash + 1) % set->capacity;
    }
    return false;
}

// Free memory allocated for the hash set
void freeHashSet(HashSet *set) {
    free(set->keys);
    free(set->filled);
}

void Init3DScene(void *arg){

    SceneData *Scene = arg;

    Scene->Model = NE_ModelCreate(NE_Static);
    Scene->Camera = NE_CameraCreate();
    Scene->frame_step = 0;
    // Scene->arr = (AtomModel *)malloc(256 * sizeof(AtomModel)); // Alloc Vertices for map
    // assert(Scene->arr != NULL);
    NE_Material *Blahaj_Material = NE_MaterialCreate();
    NE_CameraSet(Scene->Camera,
                  16,16,16,
                  0, 0, 0,
                  0, 1, 0);

    printf("Loading Map...\n");
    map_parser_load("nitro:/maps/debug.map");
    printf("Generating Mesh...\n");
    geo_generator_run();
    printf("Surface Gather...\n");
    surface_gatherer_reset_params();
    surface_gatherer_set_split_type(SST_BRUSH);
    surface_gatherer_run();
    
    const surfaces *surfs = surface_gatherer_fetch();
    Scene->surface_count = surfs->surface_count;
    Scene->surfaces = malloc(Scene->surface_count * sizeof(AtomModel));
    assert(Scene->surfaces != NULL);
    printf("We have %d Surfaces\n",surfs->surface_count);
    for (int s = 0; s < surfs->surface_count; ++s) // Loop the Surfaces
	{
        surface *surf = &surfs->surfaces[s];// Get the Surface
        AtomModel *surfd = &Scene->surfaces[s];
        surfd->arr = malloc(surf->vertex_count * sizeof(AtomVertex));
        surfd->vertex_count = surf->vertex_count;
        HashSet vertexSet = initHashSet(surf->vertex_count);
        printf("\n\n");
        for (int v = 0; v < surf->vertex_count; ++v)
		{
            if (!existsInHashSet(&vertexSet, &surf->vertices[v].vertex)) {
                surfd->arr[v].x = surf->vertices[v].vertex.x;
                surfd->arr[v].y = surf->vertices[v].vertex.y;
                surfd->arr[v].z = surf->vertices[v].vertex.z;
                insertIntoHashSet(&vertexSet, &surf->vertices[v].vertex);
            }
            printf("%f,%f,%f\n",surf->vertices[v].vertex.x,surf->vertices[v].vertex.y,surf->vertices[v].vertex.z);
            
        }
        freeHashSet(&vertexSet);
    }


    NE_ModelLoadStaticMeshFAT(Scene->Model, "blahaj_model.bin");
    NE_MaterialTexLoad(Blahaj_Material, NE_RGB5, 256, 256, NE_TEXGEN_TEXCOORD,blahajBitmap);
    
    NE_ModelSetMaterial(Scene->Model, Blahaj_Material);
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
    mmLoad(MOD_SPACE_DEBRIS);
    soundEnable();
    mmStart(MOD_SPACE_DEBRIS, MM_PLAY_LOOP);


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
            //printf("\x1B[2J\x1B[H");
            //printf("==Debug Menu %s==\n",VERSION);
            //printf("Frame %d/%u\n",Scene.frame_step,~0);
            //printf("FPS: %d\n",current_fps);
            //printf("Surface Count:%d\n",Scene.surface_count);
            fpscount++;
        #endif
    }

    soundDisable();

}
