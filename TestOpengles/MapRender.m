//
//  MapRender.m
//  TestOpengles
//
//  Created by ky on 8/8/16.
//  Copyright © 2016 yellfun. All rights reserved.
//

#import "MapRender.h"
#include "gles_map_render.h"

@interface MapRender() {
    
    gles_map_render_t gmr;
}

@property (nonatomic, copy) NSString *mapFile;

@end

@implementation MapRender

- (id)initRender:(NSString*)mapFile {
    
    self = [super init];
    
    if (self) {
        
        self.mapFile = mapFile;
        
        NSString *glesPath = [[NSBundle mainBundle] pathForResource:@"font" ofType:@"ttf" inDirectory:@"glres"];
        
        NSString *folder = [glesPath stringByDeletingLastPathComponent];
        
        const char* str = [folder UTF8String];
        
        gmr = gles_map_render_new(str);
        
        gles_map_render_init(gmr);
    }
    
    return self;
}

- (void)setSize:(CGFloat)width height:(CGFloat)height {
    
    gles_map_render_resize(gmr, width, height);
    
    const char *szMapFile = [_mapFile UTF8String];
    
    gles_map_render_setmap(gmr, szMapFile);
}

- (void)draw {
    
    gles_map_render_render(gmr, 0);
}

@end
