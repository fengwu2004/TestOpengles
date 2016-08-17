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
    
    [self configMap];
    
    [self setDisplayLink];
    
    return self;
}

- (void)setDisplayLink {
    
    CADisplayLink* displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(render:)];
    
    [displayLink addToRunLoop:[NSRunLoop mainRunLoop] forMode:NSDefaultRunLoopMode];
}

- (void)render:(CADisplayLink*)displayLink {
    
    [_glkView display];
}

- (void)configMap {
    
    NSString *glesPath = [[NSBundle mainBundle] pathForResource:@"LE" ofType:@"vbf"];
    
    _mapRender = [[MapRender alloc] initRender:glesPath];
    
    [_mapRender setSize:300 height:300];
}

- (void)createEAGContext {
    
    EAGLRenderingAPI api = kEAGLRenderingAPIOpenGLES3;
    
    _context = [[EAGLContext alloc] initWithAPI:api];
    
    [EAGLContext setCurrentContext:_context];
    
    _glkView = [[GLKView alloc] initWithFrame:self.bounds context:_context];
    
    _glkView.delegate = self;
    
    [self addSubview:_glkView];
    
    _glkView.drawableDepthFormat = GLKViewDrawableDepthFormat24;
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
