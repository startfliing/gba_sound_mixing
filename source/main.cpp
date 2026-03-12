#include "tonc.h"

#include "terminal.hpp"

#include "image.h"

int main(){
    //enable Border BG
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
    REG_DISPCNT = DCNT_BG0 | DCNT_BG1 | DCNT_MODE0;

    
    //Setup is done. Lets put it into action!
    Terminal::log("Hello World!");

    while(1){

        if(key_hit(KEY_START)){
            Terminal::reset();
        }

        //update random nunmber
        qran();

        //poll what keys are down
        key_poll();

        //helps with visual tearing
        vid_vsync();
    }
}