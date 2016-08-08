#ifndef GLES_VBFILE_INCLUDE
#define GLES_VBFILE_INCLUDE


/***
 * vbf文件头, 定的很简单
 * 开始阶段够了
 */
struct vb_header{
    
    int     width;
    int     height;
    int     azimuth;

    int     area_vcnt;
    int     line_vcnt;
    int     prim_rcnt;

    int     area_vbuf_size;
    int     area_cbuf_size;
    int     line_vbuf_size;
    int     line_cbuf_size;
    int     line_ibuf_size;

    int     area_vbuf_offset;
    int     area_cbuf_offset;
    int     line_vbuf_offset;
    int     line_cbuf_offset;
    int     line_ibuf_offset;

    char    reserved[448];
};
#endif /*GLES_VBFILE_INCLUDE*/
