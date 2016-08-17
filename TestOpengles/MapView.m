//
//  GLView.m
//  TestOpengles
//
//  Created by ky on 8/8/16.
//  Copyright © 2016 yellfun. All rights reserved.
//

#import "MapView.h"
#import "MapRender.h"

#define kScreenWidth  [UIScreen mainScreen].bounds.size.width
#define kScreenHeight [UIScreen mainScreen].bounds.size.height

@interface MapView()<GLKViewDelegate>

@property (nonatomic, assign) GLuint positionSlot;
@property (nonatomic, assign) GLuint colorSlot;
@property (nonatomic, assign) GLuint programHandle;
@property (nonatomic, assign) GLuint vertexBuffer;
@property (nonatomic, assign) GLuint indexBuffer;
@property (nonatomic, retain) MapRender *mapRender;

@end

@implementation MapView

- (id)initWithFrame:(CGRect)frame {
    
    self = [super initWithFrame:frame];
    
    [self createEAGContext];
    
    return self;
}

- (void)configMap {
    
    NSString *glesPath = [[NSBundle mainBundle] pathForResource:@"LE" ofType:@"vbf"];
    
    _mapRender = [[MapRender alloc] initRender:glesPath];
    
    [_mapRender setSize:300 height:300];
}

- (void)createEAGContext {
    
    EAGLRenderingAPI api = kEAGLRenderingAPIOpenGLES3;
    
    _context = [[EAGLContext alloc] initWithAPI:api];
    
    _glkView = [[GLKView alloc] initWithFrame:self.bounds context:_context];
    
    _glkView.drawableDepthFormat = GLKViewDrawableDepthFormat24;
    
    [_glkView setDelegate:self];
    
    [self setupGL];
}

- (void)setupGL
{
    [EAGLContext setCurrentContext:self.context];
    
//    [_glkView bindDrawable];
    
    //Optional code to demonstrate how can you bind frame buffer and render buffer.
//    GLint defaultFBO;
//    
//    GLint defaultRBO;
//    
//    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &defaultFBO);
//    
//    glGetIntegerv(GL_RENDERBUFFER_BINDING, &defaultRBO);
//    
//    glBindFramebuffer( GL_FRAMEBUFFER, defaultFBO );
//    
//    glBindRenderbuffer( GL_RENDERBUFFER, defaultRBO );
    
    [self configMap];
}

- (void)glkView:(GLKView *)view drawInRect:(CGRect)rect {
    
    [_mapRender draw];
}

- (void)zoom {
    
    [_mapRender zoom:1.5];
}

- (void)rotate {
    
    
}

@end
