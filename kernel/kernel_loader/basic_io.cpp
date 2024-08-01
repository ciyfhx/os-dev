#include "basic_io.hpp"

/**  
* fb_write_cell:
*  Writes a character with the given foreground and background to position i
*  in the framebuffer.
*
*  @param i  The location in the framebuffer
*  @param c  The character
*  @param fg The foreground color
*  @param bg The background color
*/
void fb_write_cell(unsigned int i, char c, unsigned char fg, unsigned char bg)
{
    char* fb = (char*) VIDEO_RAM;
    fb[i] = c;
    fb[i + 1] = ((fg & 0x0F) << 4) | (bg & 0x0F);
}

unsigned int writing_state = 0;       // character writing position
unsigned int cursor_state = 0;        // position of the cursor

int fb_write(char *buf, unsigned int len)
{
    for (unsigned int i=0; i<len;i++)
    {
        fb_write_cell((writing_state+(i*2)), *(buf+i), FB_WHITE, FB_BLACK);
    }
    writing_state+=len*2;
    cursor_state+=len;
    fb_move_cursor(cursor_state);
    return 0;
}


int fb_write(char *buf)
{
    int i = 0;
    while(*(buf+i) != '\0')
    {
        fb_write_cell((writing_state+(i*2)), *(buf+i), FB_WHITE, FB_BLACK);
        i++;
    }
    writing_state+=i*2;
    cursor_state+=i;
    fb_move_cursor(cursor_state);
    return 0;
}

void panic(char *buf)
{
    fb_write(buf);
    asm("hlt");
}