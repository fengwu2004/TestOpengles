#include<stdio.h>
#include<sys/times.h>
#include<time.h>
#include<stdlib.h>
#include<time.h>
#include<math.h>
#include"mem.h"
#include"assert.h"
#include"gles_vec_layer.h"
#include"gles_vbfile.h"

#ifdef ANDROID_IDRN
#include"error_functions.h"
#endif

#define ATTRIBUTE_POSITION  0
#define ATTRIBUTE_COLOR     1

#define T gles_vec_layer_t


struct T{
    gles_map_t  map;
    int         width;
    int         height;
    int         azimuth;
    int         area_ver_count;         //区域顶点数目
    int         line_ver_count;         //轮廓线顶点数目
    int         line_prim_reset_count;  //轮廓线图元重启计数
    GLuint      area_vbo_id;            //区域vboid
    GLuint      line_vbo_id;            //轮廓线vboid
    GLuint      line_ibo_id;            //轮廓线iboid
    GLuint      line_vao_id;            //轮廓线vaoid
};


static void _dump_vbfile(struct vb_header * vbh){

#ifdef ANDROID_IDRN
    log_i("-----dump vbfile map-----");
    log_i("\twidth:%d, height:%d, azimuth:%d\n", vbh->width, vbh->height, vbh->azimuth);

    log_i("-----dump vbfile size:%d-----", sizeof(*vbh));
    log_i("\tcount:\n\t\tarea_vcnt:%d\n\t\tline_vcnt:%d\n\t\tprim_rcnt:%d\n",
            vbh->area_vcnt, vbh->line_vcnt, vbh->prim_rcnt);

    log_i("\tsize:\n\t\tarea_vbuf_size:%d\n\t\tarea_cbuf_size:%d\n\t\tline_vbuf_size:%d\n\t\tline_cbuf_size:%d\n\t\tline_ibuf_size:%d\n",
            vbh->area_vbuf_size, vbh->area_cbuf_size, vbh->line_vbuf_size, vbh->line_cbuf_size, vbh->line_ibuf_size);

    log_i("\toffset:\n\t\tarea_vbuf_offset:%d\n\t\tarea_cbuf_offset:%d\n\t\tline_vbuf_offset:%d\n\t\tline_cbuf_offset:%d\n\t\tline_ibuf_offset:%d\n",
            vbh->area_vbuf_offset, vbh->area_cbuf_offset, vbh->line_vbuf_offset, vbh->line_cbuf_offset, vbh->line_ibuf_offset);
    log_i("*****dump vbfile*****");
#endif
}
T            
gles_vec_layer_new
(gles_map_t map, const char *vbfile)
{
    T gvl;
    float *area_vbuf;
    float *area_cbuf;
    float *line_vbuf;
    float *line_cbuf;
    GLushort *line_ibuf;

//    log_i("--------------[%s]", vbfile);

    gvl = ALLOC(sizeof(*gvl));
    gvl->map = map;

    struct vb_header vbh;
    FILE *file = fopen(vbfile, "r");
    fread(&vbh, sizeof(struct vb_header), 1, file);
    _dump_vbfile(&vbh);

    gvl->width = vbh.width;
    gvl->height= vbh.height;
    gvl->azimuth= vbh.azimuth;
    gvl->area_ver_count = vbh.area_vcnt;
    gvl->line_ver_count = vbh.line_vcnt;
    gvl->line_prim_reset_count = vbh.prim_rcnt;

    area_vbuf = ALLOC(vbh.area_vbuf_size);
    area_cbuf = ALLOC(vbh.area_cbuf_size);
    line_vbuf = ALLOC(vbh.line_vbuf_size);
    line_cbuf = ALLOC(vbh.line_cbuf_size);
    line_ibuf = ALLOC(vbh.line_ibuf_size);

    fread(area_vbuf, vbh.area_vbuf_size, 1, file);
    fread(area_cbuf, vbh.area_cbuf_size, 1, file);
    fread(line_vbuf, vbh.line_vbuf_size, 1, file);
    fread(line_cbuf, vbh.line_cbuf_size, 1, file);
    fread(line_ibuf, vbh.line_ibuf_size, 1, file);

    fclose(file);
    /**
     * 构造底图色块的vbo
     */
    glGenBuffers(1, &(gvl->area_vbo_id));
	glBindBuffer( GL_ARRAY_BUFFER, gvl->area_vbo_id );
	glBufferData( GL_ARRAY_BUFFER, vbh.area_vbuf_size + vbh.area_cbuf_size, 0, GL_STATIC_DRAW );
	glBufferSubData( GL_ARRAY_BUFFER, 0,                  vbh.area_vbuf_size, area_vbuf);
	glBufferSubData( GL_ARRAY_BUFFER, vbh.area_vbuf_size, vbh.area_cbuf_size, area_cbuf);
	glBindBuffer( GL_ARRAY_BUFFER, 0 );

    /**
     * 构造地图描边的包含顶点缓存和颜色缓存的vbo
     */
    glGenBuffers(1, &(gvl->line_vbo_id));
	glBindBuffer( GL_ARRAY_BUFFER, gvl->line_vbo_id );
	glBufferData( GL_ARRAY_BUFFER, vbh.line_vbuf_size + vbh.line_cbuf_size, 0, GL_STATIC_DRAW);
	glBufferSubData( GL_ARRAY_BUFFER, 0,                  vbh.line_vbuf_size, line_vbuf);
	glBufferSubData( GL_ARRAY_BUFFER, vbh.line_vbuf_size, vbh.line_cbuf_size, line_cbuf);

    /**
     * 构造地图描边的包含索引缓存的vbo
     */
    glGenBuffers(1, &(gvl->line_ibo_id));
	glBindBuffer( GL_ARRAY_BUFFER, gvl->line_ibo_id );
	glBufferData( GL_ARRAY_BUFFER, vbh.line_ibuf_size, 0, GL_STATIC_DRAW);
	glBufferSubData( GL_ARRAY_BUFFER, 0, vbh.line_ibuf_size, line_ibuf);
    
    /**
     * 构造地图描边的vao并将上面两个vbo绑定到vao
     */
    glGenVertexArrays(1, &(gvl->line_vao_id));
    glBindVertexArray(gvl->line_vao_id);

    glBindBuffer(GL_ARRAY_BUFFER, gvl->line_vbo_id);
    glEnableVertexAttribArray(ATTRIBUTE_POSITION);
    glEnableVertexAttribArray(ATTRIBUTE_COLOR);
    glVertexAttribPointer(ATTRIBUTE_POSITION, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glVertexAttribPointer(ATTRIBUTE_COLOR,    4, GL_FLOAT, GL_FALSE, 0, (void*)(vbh.line_vbuf_size));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gvl->line_ibo_id);

    /**
     * 一定要解绑
     */
    glBindVertexArray(0);
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

    FREE(area_vbuf);
    FREE(area_cbuf);
    FREE(line_vbuf);
    FREE(line_cbuf);
    FREE(line_ibuf);

    return gvl;
}

void         
gles_vec_layer_free
(T *gvl)
{
    assert(gvl);
    assert(*gvl);

	glDeleteBuffers(1, &((*gvl)->area_vbo_id));
	glDeleteBuffers(1, &((*gvl)->line_vbo_id));
	glDeleteBuffers(1, &((*gvl)->line_ibo_id));
	glDeleteVertexArrays(1, &((*gvl)->line_vao_id));
    FREE(*gvl);
}


void         
gles_vec_layer_render
(T gvl, gles_2d_c_program_t gcp)
{

    gles_2d_c_program_vbo_draw_triangle(gcp, gvl->area_vbo_id, gvl->area_ver_count);
    
    glEnable(GL_PRIMITIVE_RESTART_FIXED_INDEX);
    gles_2d_c_program_draw_elements_vao(gcp, GL_LINE_LOOP, gvl->line_vao_id, gvl->line_ver_count + gvl->line_prim_reset_count);
    glDisable(GL_PRIMITIVE_RESTART_FIXED_INDEX);
}

int          
gles_vec_layer_width
(T gvl)
{
    assert(gvl);
    return gvl->width;
}

int          
gles_vec_layer_height
(T gvl)
{
    assert(gvl);
    return gvl->height;
}

int          
gles_vec_layer_azimuth
(T gvl)
{
    assert(gvl);
    return gvl->azimuth;
}

#undef ATTRIBUTE_POSITION
#undef ATTRIBUTE_COLOR 
