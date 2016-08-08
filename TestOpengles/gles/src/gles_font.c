#include <math.h>
#include <float.h>
#include "ft2build.h"
#include "freetype/freetype.h"
#include "freetype/ftglyph.h"
#include "freetype/ftoutln.h"
#include "freetype/fttrigon.h"

#include "assert.h"
#include "mem.h"
#include "arena.h"
#include "sparsearray.h"

#include"gles_utils.h"
#include"gles_font.h"
#include"gles_matrix.h"

#ifdef ANDROID_IDRN
#include"error_functions.h"
#endif

#define T gles_font_t

#define A_POSITION              0
#define A_TEXCOORD              1

struct T{
    unsigned int                    pid;
    unsigned int                    tid;
    unsigned int                    upmatrix;
    unsigned int                    ummatrix;
    unsigned int                    utexture;
    unsigned int                    utextcolor;
    int                             ready;
    int                             resolution;
    int                             glyphmargin;
    float                           dimension;
    float                           squaresize;
    const GLfloat                  *pmat;
    FT_Face                         fontface;
    FT_Library                      libft;
    arena_t                         arena;
    sparsearray_t                   glyphs;
    char                            *filepath;
};

struct glyph{
    FT_Glyph_Metrics metric;
    
    float advance_horizontal;

    float advance_vertical;
    
    float texcoordx;
    float texcoordy;
    
    float atlasx;
    float atlasy;
};



static void _calc_resolution(T fft);
static void _read_font(T fft);
static int  _next_power_of_2(int number);
static void _generate_tex_from_glyph(FT_GlyphSlot glyph, GLubyte* texture, 
        int atlasX, int atlasY, int texSize, 
        int resolution, int marginSize, int drawBorder);
static void _set_pixel (GLubyte* texture, int offset, int size, int x, int y, GLubyte val);
static void _drawtext_pm(T fft, const GLfloat *pmat, const GLfloat *mmat, 
                    const char* str, int color, float x, float y);
static void _draw_glyph(T fft, const GLfloat *pmat, const GLfloat *mmat, struct glyph *gi, float x, float y);

static const char _fontvshader[] = 
"#version 300 es                                            \n"
"layout(location = 0) in vec4  aPosition;                   \n"
"layout(location = 1) in vec2  aTexCoord;                   \n"
"out vec2 vTexCoord;                                        \n"
"uniform mat4   uProjectionMat;                             \n"
"uniform mat4   uModelMat;                                  \n"
"void main( void ) {                                        \n"
"    vTexCoord = aTexCoord;                                 \n"
"    gl_Position = uProjectionMat*uModelMat*aPosition;      \n"
"}\n";
static const char _fontfshader[] = 
"#version 300 es                                            \n"
"precision mediump float;                                   \n"
"in vec2 vTexCoord;                                         \n"
"uniform sampler2D uTex;                                    \n"
"uniform vec4 uTextColor;                                   \n"
"layout(location = 0) out vec4 outColor;                    \n"
"void main() {                                              \n"
"   vec4 texcol = texture(uTex, vTexCoord);                 \n"
"   outColor    = vec4(vec3(uTextColor.rgb), texcol.a);      \n"
"}\n";

static GLfloat _i_mat[16];

T            
gles_font_new
(const char *filepath, int resolution)
{
    T fft;
    arena_t arena;
    int len;

    
    assert(filepath);
    arena       = arena_new();
    fft    = (T)ARENA_CALLOC(arena, 1, sizeof(*fft));
    len = strlen(filepath);
    assert(len>0);

    fft->arena = arena;
    fft->filepath = ARENA_ALLOC(arena, sizeof(*(fft->filepath))*(len+1));
    strncpy(fft->filepath, filepath, len);
    fft->filepath[len] = 0;

    fft->glyphs = sparsearray_new(512);
    fft->resolution = resolution;
    fft->ready = 0;

    gles_matrix_identity_4(_i_mat, 0);

    return fft;
}


void         
gles_font_free
(T *fft)
{
    assert(fft);
    assert(*fft);
    glDeleteProgram((*fft)->pid);
    /**
     * 按套路先释放FreeType的字型
     * 再释放FreeType的库实例
     */
    FT_Done_Face((*fft)->fontface);
    FT_Done_FreeType((*fft)->libft);

    /**
     * 释放材质对象
     */
    if(0 != (*fft)->tid) {
        glDeleteTextures(1, &((*fft)->tid));
    }

    /**
     * 释放符号表
     */
    sparsearray_free(&((*fft)->glyphs));
    arena_dispose(&((*fft)->arena));
}


void         
gles_font_pmat
(T fft, const GLfloat *pmat)
{
    assert(fft);
    fft->pmat = pmat;
	glUseProgram(fft->pid);
    glUniformMatrix4fv(fft->upmatrix, 1, GL_FALSE, pmat);
    glUniformMatrix4fv(fft->ummatrix, 1, GL_FALSE, _i_mat);
}

void         
gles_font_load
(T fft)
{

    int ret;
    assert(fft);
    /**
     * 这里有毒, 吓我一跳
     * 估计这两个接口是特殊宏, 外面套断言宏之后行为非常怪异
     * 具体就是不会报错但里面的实例没分配内存
     */
    //assert(0 == FT_Init_FreeType(&(fft->libft)));
    //assert(0 == FT_New_Face(fft->libft, fft->filepath, 0, &(fft->fontface)));
    ret = FT_Init_FreeType(&(fft->libft));
    assert(0 == ret);
    ret = FT_New_Face(fft->libft, fft->filepath, 0, &(fft->fontface));
    assert(0 == ret);

    //中文支持要花点时间, 直接这样设了不管用
    //FT_Select_Charmap(fft->fontface, FT_ENCODING_UNICODE);

    fft->ready = 1;
    _calc_resolution(fft);

    _read_font(fft);

    fft->pid = gles_create_program(_fontvshader, _fontfshader);
    if (!fft->pid) {
#ifdef ANDROID_IDRN
        log_e("Could not create font texture program.");
#endif
    }else{
        fft->upmatrix = glGetUniformLocation(fft->pid, "uProjectionMat");
        fft->ummatrix = glGetUniformLocation(fft->pid, "uModelMat");
        fft->utexture = glGetUniformLocation(fft->pid, "uTex");
        fft->utextcolor = glGetUniformLocation(fft->pid, "uTextColor");
    }
    gles_check_error("GraphicsInit font texture");
}

float        
gles_font_size
(T fft)
{
    assert(fft);
    return fft->squaresize;
}


unsigned int 
gles_font_tid
(T fft)
{
    assert(fft);
    return fft->tid;
}


int          
gles_font_text_bounds
(T fft, const char *str, struct bounds_t *b, float x, float y)
{
    assert(fft);
    assert(fft->ready);
    const char *c;
    struct glyph *gi;
    float w, h;
    float b_left, b_top, b_right, b_bottom;
    float r_left, r_top, r_right, r_bottom;
    float xmargin;
    float ymargin;
    b_left = FLT_MAX;
    b_top = FLT_MAX;
    b_right = FLT_MIN;
    b_bottom = FLT_MIN;
    for (c = str; *c != '\0'; c++) {
        gi = sparsearray_get(fft->glyphs, (long) *c);
        if(NULL != gi){
            x += gi->advance_horizontal / 2.0;
            xmargin = ((float)gi->metric.width)/((float)2*64);
            ymargin = ((float)gi->metric.horiBearingY)/((float)2*64);

            r_left  = -0.5f;
            r_top   = -0.5f;
            r_right =  0.5f;
            r_bottom=  0.5f;
            r_left  *= (float)(fft->squaresize)/2;
            r_left  += (x - xmargin);
            r_top   *= (float)(fft->squaresize)/2;
            r_top   += (y + ymargin);
            r_right *= (float)(fft->squaresize)/2;
            r_right += (x - xmargin);
            r_bottom *= (float)(fft->squaresize)/2;
            r_bottom += (y + ymargin);

            if(b_left > r_left)
                b_left = r_left;
            if(b_top > r_top)
                b_top = r_top;
            if(b_right < r_right)
                b_right = r_right;
            if(b_bottom < r_bottom)
                b_bottom = r_bottom;
        }
    }
    b->left = b_left;
    b->top = b_top;
    b->right = b_right;
    b->bottom = b_bottom;
    return 0;
}

inline void         
gles_font_drawtext(T fft,
                    const char* str, int color, float x, float y)
{
    _drawtext_pm(fft, NULL, NULL, str, color, x, y);
}

inline void         
gles_font_drawtext_p(T fft, const GLfloat *pmat,
                    const char* str, int color, float x, float y)
{
    _drawtext_pm(fft, pmat, _i_mat, str, color, x, y);
}

void         
gles_font_set_mmat(T fft, const GLfloat *mmat)
{
	glUseProgram(fft->pid);
    glUniformMatrix4fv(fft->ummatrix, 1, GL_FALSE, mmat);
}

void         
gles_font_reset_mat(T fft)
{
	glUseProgram(fft->pid);
    glUniformMatrix4fv(fft->ummatrix, 1, GL_FALSE, _i_mat);
}

static void         
_drawtext_pm(T fft, const GLfloat *pmat, const GLfloat *mmat, 
                    const char* str, int color, float x, float y)
{
    assert(fft);
    assert(fft->ready);

    const char *c;
    struct glyph *gi;
    float offsetx;
    GLfloat cu[4] = {gles_color_red(color), 
                        gles_color_green(color), 
                        gles_color_blue(color), 
                        gles_color_alpha(color)};

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glUseProgram(fft->pid);
	glActiveTexture (GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, fft->tid);

    glUniform1i(fft->utexture, 0);
    glUniform4fv(fft->utextcolor, 1, cu);
    if(NULL != pmat)
        glUniformMatrix4fv(fft->upmatrix, 1, GL_FALSE, pmat);
    if(NULL != mmat)
        glUniformMatrix4fv(fft->ummatrix, 1, GL_FALSE, mmat);

    // Send the vertex and texture coordinates to the shaders

    offsetx = x;
    for (c = str; *c != '\0'; c++) {
        gi = sparsearray_get(fft->glyphs, (long) *c);
        if(NULL != gi){
            offsetx += gi->advance_horizontal / 2.0;
        }else{
            continue;
        }
        _draw_glyph(fft, pmat, mmat, gi, offsetx, y);
    }
}

static 
void 
_draw_glyph(T fft, const GLfloat *pmat, const GLfloat *mmat, struct glyph *gi, float x, float y)
{
    int i;
	float xmargin = ((float)gi->metric.width)/((float)2*64);
	float ymargin = ((float)gi->metric.horiBearingY)/((float)2*64);
    
    float texCoords[8] = {
        gi->atlasx, gi->atlasy,
        gi->atlasx + fft->dimension, gi->atlasy,
        gi->atlasx, gi->atlasy + fft->dimension,
        gi->atlasx + fft->dimension, gi->atlasy + fft->dimension
    };

    // Calculate the dimension of the glyph Quad.
    float quad[8]	= {
        -0.5f, 0.5f, 
         0.5f, 0.5f, 
        -0.5f, -0.5f,
         0.5f, -0.5f};

	for (i = 0; i<8;){
		quad[i]   *= (float)(fft->squaresize)/2;
        quad[i]   += x;
        quad[i]   -= xmargin;
        quad[i+1] *= (float)(fft->squaresize)/2;
        quad[i+1] += y;
        quad[i+1] += ymargin;
		i		  += 2;
	}


    // Initialize the texture with texture unit 0
    glEnableVertexAttribArray(A_POSITION);
    glEnableVertexAttribArray(A_TEXCOORD);
    glVertexAttribPointer(A_POSITION, 2, GL_FLOAT, GL_FALSE, 0, quad);
    glVertexAttribPointer(A_TEXCOORD, 2, GL_FLOAT, GL_FALSE, 0, texCoords);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

static 
void 
_calc_resolution(T fft)
{
    long numglyphs;
    int squaresize;
    int numglyphs_p_row;
    int texsize;
    int realtexsize;
    int glyphmargin;
    GLint maxtexturesize;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxtexturesize);
    while(fft->resolution > 0){
        glyphmargin                 = (int)ceil(fft->resolution*0.1f);
        numglyphs                   = fft->fontface->num_glyphs;
        squaresize                  = fft->resolution + glyphmargin;
        numglyphs_p_row             = (int)ceil(sqrt((double)numglyphs));
        texsize                     = numglyphs_p_row*squaresize;
        realtexsize                 = _next_power_of_2(texsize);
        if(realtexsize <= maxtexturesize){
            break;
        }
        fft->resolution -= 5;
    }
    
    assert(fft->resolution > 0);
    fft->glyphmargin        = glyphmargin;
}


static 
void 
_read_font(T fft)
{
    int numGlyphs;
    int numGlyphsPerRow;
    int texSize;
    int realTexSize;
    int texAtlasX;
    int texAtlasY;
    GLubyte* textureData;
    FT_UInt gindex;
    FT_ULong charcode;


    assert(fft);
    FT_Set_Pixel_Sizes(fft->fontface, fft->resolution, fft->resolution);
    numGlyphs         = fft->fontface->num_glyphs;
    fft->squaresize   = fft->resolution + fft->glyphmargin;
    numGlyphsPerRow   = (int)ceil(sqrt((double)numGlyphs)); //=numRows (texture is a square)
    texSize           = numGlyphsPerRow*fft->squaresize;
    realTexSize       = _next_power_of_2(texSize);
    
    textureData        = CALLOC(realTexSize*realTexSize*2, sizeof(GLubyte));
    if (fft->tid){
        glDeleteTextures(1, &(fft->tid));
        fft->tid = 0;
    }
    
    glGenTextures(1, &(fft->tid));
    glBindTexture(GL_TEXTURE_2D, fft->tid);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    gles_check_error("font atlas texture");
    
    texAtlasX       = 0;
    texAtlasY       = 0;
    gindex          = 0;
    charcode    = 0;
    for (charcode=FT_Get_First_Char(fft->fontface, &gindex); gindex != 0;
             charcode=FT_Get_Next_Char(fft->fontface, charcode, &gindex)) {
        
        if (FT_Load_Glyph(fft->fontface, gindex, FT_LOAD_DEFAULT)) {
#ifdef ANDROID_IDRN
            log_e("Error loading glyph with index %i and charcode %i. Skipping.", gindex, (int)charcode);
#endif
            continue;
        }
        
        FT_GlyphSlot glyph = fft->fontface->glyph;
        FT_Render_Glyph(glyph, FT_RENDER_MODE_NORMAL);
        struct glyph *glyphInfo = ARENA_ALLOC(fft->arena, sizeof(*glyphInfo));

        glyphInfo->metric		= glyph->metrics;
        glyphInfo->atlasx		= (texAtlasX*fft->squaresize)/(float)realTexSize;
        glyphInfo->atlasy		= (texAtlasY*fft->squaresize)/(float)realTexSize;
        
        glyphInfo->advance_horizontal   = (glyph->advance.x)/(float)(64.0f);
        glyphInfo->advance_vertical	    = (glyph->advance.y)/(float)(64.0f);

        sparsearray_put         (fft->glyphs, charcode, glyphInfo);
        
        _generate_tex_from_glyph(glyph, textureData, texAtlasX, texAtlasY, realTexSize, fft->resolution, fft->glyphmargin, 0);
        

        texAtlasX++;
        if (texAtlasX >= numGlyphsPerRow) {
            texAtlasX=0;
            texAtlasY++;
        }

    }
    
    glTexImage2D (GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, realTexSize, realTexSize, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, textureData);
    
    FREE(textureData);
    gles_check_error("read fon end");

    fft->dimension = (fft->squaresize)/(float)realTexSize;
}

static 
void 
_generate_tex_from_glyph(FT_GlyphSlot glyph, GLubyte* texture, 
        int atlasX, int atlasY, int texSize, 
        int resolution, int marginSize, int drawBorder)
{
    int squareSize, baseOffset, gr, gw, w, h;

    squareSize = resolution + marginSize;
    baseOffset = atlasX*squareSize + atlasY*squareSize*texSize;
    
    if (drawBorder) {
        for (w=0; w<squareSize; w++){ 
            _set_pixel(texture, baseOffset, texSize, w, 0, 255);
        }
        
        for (h=1; h<squareSize; h++)
            for (w=0; w<squareSize; w++)
                _set_pixel(texture,baseOffset,texSize,w,h, (w==0||w==squareSize-1)?255:(h==squareSize-1)?255:0);
    }
    
    gr = glyph->bitmap.rows;
    gw = glyph->bitmap.width;
    for (h=0; h<gr; h++)
        for (w=0; w<gw; w++)
            _set_pixel(texture, baseOffset+marginSize, texSize, w, marginSize+h, glyph->bitmap.buffer[w+h*gw]);
}

static 
void 
_set_pixel (GLubyte* texture, int offset, int size, int x, int y, GLubyte val)
{
    texture[2*(offset+x+y*size)] = texture[2*(offset+x+y*size)+1] = val;
}

static 
int  
_next_power_of_2(int number)
{
    int rval = 1;
    while (rval<number) rval*=2;
    return rval;
}
