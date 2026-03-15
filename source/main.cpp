#include "tonc.h"
#include "maxmod.h"

#include "terminal.hpp"
#include "soundbank.h"
#include "soundbank_bin.h"

#include "image.h"

#include "blt.h"
#include "cw.h"
#include "dftr.h"
#include "ditw.h"
#include "iwdws.h"
#include "kohd.h"
#include "mtw.h"
#include "sth.h"
#include "tu.h"

struct song{
    const void* albumTiles;
    const void* albumPal;
    char* songName;
};

song songs[9] = {
    {
        bltTiles,
        bltPal,
        "Bizarre Love Triangle"
    },
    {
        cwTiles,
        cwPal,
        "Careless Whisper"
    },
    {
        kohdTiles,
        kohdPal,
        "Knockin' on Heaven's Door"
    },
    {
        ditwTiles,
        ditwPal,
        "Dust in the Wind"
    },
    {
        mtwTiles,
        mtwPal,
        "More Than Words"
    },
    {
        sthTiles,
        sthPal,
        "Stairway To Heaven"
    },
    {
        tuTiles,
        tuPal,
        "The Unforgiven"
    },
    {
        dftrTiles,
        dftrPal,
        "Don't Fear the Reaper"
    },
    {
        iwdwsTiles,
        iwdwsPal,
        "I Wanna Dance With Somebody"
    }
};

void updateAlbum(int currSong, int nextSong){
    memcpy16(&tile_mem[4], songs[currSong].albumTiles, 1040);
    memcpy16(pal_obj_mem, songs[currSong].albumPal, 16);

    memcpy16(&tile_mem[4][128], songs[nextSong].albumTiles, 1040);
    memcpy16(&pal_obj_mem[16], songs[nextSong].albumPal, 16);
}

void songUpdate(int currSong, int nextSong){
    Terminal::reset();
    Terminal::log("Now Playing : %%", songs[currSong].songName);
    Terminal::log("Next Song : %%", songs[nextSong].songName);
    Terminal::log("Playing");
    mmStart( currSong, MM_PLAY_ONCE );
    updateAlbum(currSong, nextSong);
}

int main() {

    u8 cbb = 0;
    u8 sbb = 16;
    REG_BG0CNT = BG_BUILD(cbb, sbb, 0, 0, 1, 0, 0);

    //load palette
    memcpy16(pal_bg_mem, imagePal, imagePalLen/2);

    //load tiles
    LZ77UnCompVram(imageTiles, tile_mem[cbb]);
    
    //load image
    memcpy16(&se_mem[sbb], imageMap, imageMapLen/2);

    //enable Text BG
    REG_BG1CNT = Terminal::setCNT(1, cbb+1, sbb+1);
    REG_DISPCNT = DCNT_BG0 | DCNT_BG1 | DCNT_MODE0 | DCNT_OBJ | DCNT_OBJ_1D;


	irq_init(nullptr);

	// Maxmod requires the vblank interrupt to reset sound DMA.
	// Link the VBlank interrupt to mmVBlank, and enable it. 
	irq_set( II_VBLANK, mmVBlank, 0);
	irq_enable(II_VBLANK);

	// initialise maxmod with soundbank and 8 channels
    mmInitDefault( (mm_addr)soundbank_bin, 16 );

	// Start playing module

    int currSong = 0;
    int nextSong = 0;

    bool songOver = false;
    bool songPaused = false;

    songUpdate(currSong, nextSong);

    oam_init(oam_mem, 128);

    //set objects once, then use pal and tile updates to change cover
    obj_set_attr(&oam_mem[0], ATTR0_SQUARE | ATTR0_BLEND | ATTR0_Y(40),
		ATTR1_SIZE_64 | ATTR1_X(32), 1);
    obj_set_attr(&oam_mem[1], ATTR0_SQUARE | ATTR0_BLEND | ATTR0_Y(40),
		ATTR1_SIZE_64 | ATTR1_X(144), 129 | ATTR2_PALBANK(1));


	while( 1 ){

		VBlankIntrWait();
        key_poll();
		mmFrame();


		if ( key_hit(KEY_A) ) {
			if(mmActive()){
                mmStop();
            }
            currSong = nextSong;
            songUpdate(currSong, nextSong);
		}

        if(key_hit(KEY_SHOULDER)){
            nextSong = wrap(nextSong + key_tri_shoulder(), 0, 9);
            updateAlbum(currSong, nextSong);
            Terminal::eraseLine();
            Terminal::eraseLine();
            Terminal::log("Next Song : %%", songs[nextSong].songName);
            songPaused ? Terminal::log("Paused") : Terminal::log("Playing");
        }

        if(key_hit(KEY_START)){
            songPaused = !songPaused;
            Terminal::eraseLine();
            if(songPaused){
                Terminal::log("Paused");
                mmPause();
            }else{
                Terminal::log("Playing");
                mmResume();
            }
        }

        if(!mmActive() && !songPaused){
            currSong = nextSong;
            nextSong++;
            songUpdate(currSong, nextSong);
        }

	}
}