#include<string.h>
#import <OpenGLES/ES3/gl.h>
#import <OpenGLES/ES3/glext.h>
#include"assert.h"
#include"mem.h"
#include"png.h"
#include"gles_texture_png.h"

#define T gles_texture_png_t

struct T{
    int             width;
    int             height;
    unsigned int    tid;
    png_byte        colortype;    
    char            *filepath;
    unsigned char   *rawdata;
};

static unsigned char *_read_png_file(T tx);

T
gles_texture_png_new
(const char *filepath)
{
    T txt;
    int len;
    
    assert(filepath);
    len = strlen(filepath);
    assert(len>0);

    txt = ALLOC(sizeof(*txt));
    txt->rawdata = NULL;
    txt->filepath = ALLOC(sizeof(*(txt->filepath))*(len+1));

    strncpy(txt->filepath, filepath, len);
    txt->filepath[len] = 0;

    return txt;
}

void
gles_texture_png_free
(T *tx)
{
    assert(tx);
    assert(*tx);

    FREE((*tx)->filepath);
    FREE((*tx)->rawdata);

    if((*tx)->tid)
        glDeleteTextures(1, &((*tx)->tid));

    FREE(*tx);
}

void         
gles_texture_png_load
(T tx)
{
    assert(tx);
    tx->rawdata = _read_png_file(tx);
    assert(tx->rawdata);
    glGenTextures(1, &(tx->tid));
    glBindTexture(GL_TEXTURE_2D, tx->tid);
    if(PNG_COLOR_TYPE_GRAY == tx->colortype){
        glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, tx->width, tx->height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, tx->rawdata);
    }else if(PNG_COLOR_TYPE_GRAY_ALPHA == tx->colortype){
        glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, tx->width, tx->height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, tx->rawdata);
    }else if(PNG_COLOR_TYPE_RGB == tx->colortype){
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tx->width, tx->height, 0, GL_RGB, GL_UNSIGNED_BYTE, tx->rawdata);
    }else if(PNG_COLOR_TYPE_RGB_ALPHA == tx->colortype){
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tx->width, tx->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tx->rawdata);
    }else{
        /**
         * 因为已经在rawdada里解压成32位的位图了, 所以当颜色是不支持的调色板之类的时, 用RGBA是比较合适的选择, 显示效果也没问题
         */
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tx->width, tx->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tx->rawdata);
    }
    FREE(tx->rawdata);
}

unsigned int 
gles_texture_png_width
(T tx)
{
    assert(tx);
    return tx->width;
}


unsigned int 
gles_texture_png_height
(T tx)
{
    assert(tx);
    return tx->height;
}


unsigned int 
gles_texture_png_tid
(T tx)
{
    assert(tx);
    return tx->tid;
}


static
unsigned char *
_read_png_file(T tx)
{
    char header[8];
    png_structp pngsp;
    png_infop pngip;
    png_byte pngbd;
    png_bytep *bytep;
    unsigned char* retval;
    int x, y, i, n;
    int number_of_passes;
    unsigned int byte;

    FILE *fp = fopen(tx->filepath, "rb");
    assert(fp);
    fread(header, 1, 8, fp);

    assert(!png_sig_cmp((png_const_bytep) header, 0, 8));

    pngsp = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    assert(pngsp);

    pngip = png_create_info_struct(pngsp);
    assert(pngip);

    png_init_io(pngsp, fp);
    png_set_sig_bytes(pngsp, 8);

    png_read_info(pngsp, pngip);

    tx->width		    = png_get_image_width(pngsp, pngip);
    tx->height		    = png_get_image_height(pngsp, pngip);
    tx->colortype	    = png_get_color_type(pngsp, pngip);
    pngbd			    = png_get_bit_depth(pngsp, pngip);
    number_of_passes	= png_set_interlace_handling(pngsp);
    if(PNG_COLOR_TYPE_PALETTE == tx->colortype){
        png_set_expand( pngsp ); 
    }

    if(PNG_COLOR_TYPE_GRAY == tx->colortype && pngbd < 8){
        png_set_expand( pngsp ); 
    }
    
    if(png_get_valid( pngsp, pngip, PNG_INFO_tRNS)){
        png_set_expand( pngsp );
    }
    
    if(16 == pngbd){
        png_set_strip_16( pngsp );
    }
    
    if(PNG_COLOR_TYPE_GRAY == tx->colortype || PNG_COLOR_TYPE_GRAY_ALPHA == tx->colortype){
        png_set_gray_to_rgb( pngsp );
    }

    byte = 4;
    switch(tx->colortype){
        case PNG_COLOR_TYPE_GRAY:
            byte = 1;
            break;

        case PNG_COLOR_TYPE_GRAY_ALPHA:
            byte = 2;
            break;

        case PNG_COLOR_TYPE_RGB:
            byte = 3;
            break;

        case PNG_COLOR_TYPE_RGB_ALPHA:
            byte = 4;
            break;
    }

    png_read_update_info(pngsp, pngip);
    retval = (unsigned char *) ALLOC(tx->width * tx->height * 4);
    bytep = (png_bytep *) ALLOC(tx->height * sizeof(png_bytep));
    i = 0;
    while(i != tx->height){
        n = tx->height - (i + 1);
        bytep[n] = retval + (n * tx->width * byte);
        ++i;
    }

    png_read_image(pngsp, bytep);
    png_read_end(pngsp, NULL);
    png_destroy_read_struct(&pngsp, &pngip, NULL);

    fclose(fp);
    FREE(bytep);
    return retval;
}
