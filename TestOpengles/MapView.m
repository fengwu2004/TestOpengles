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

#define IOS_MAX_TOUCHES_COUNT 2

#define TOUCH_ACTION_DOWN   0
#define TOUCH_ACTION_MOVE   1
#define TOUCH_ACTION_UP     2
#define TOUCH_MULTI_DOWN    3
#define TOUCH_MULTI_UP      4

@interface MapView()<GLKViewDelegate>

@property (nonatomic, retain) MapRender *mapRender;

@end

@implementation MapView

- (id)initWithFrame:(CGRect)frame {
    
    self = [super initWithFrame:frame];
    
    [self createEAGContext];
    
    [self configMap];
    
    [self setDisplayLink];
    
    self.multipleTouchEnabled = YES;
    
    _glkView.multipleTouchEnabled = YES;
    
    return self;
}

- (void)setDisplayLink {
    
    CADisplayLink* displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(render:)];
    
    [displayLink addToRunLoop:[NSRunLoop mainRunLoop] forMode:NSDefaultRunLoopMode];
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event {
    
    float xs[IOS_MAX_TOUCHES_COUNT] = {0.0f};
    
    float ys[IOS_MAX_TOUCHES_COUNT] = {0.0f};
    
    int i = 0;
    
    CGFloat contentScaleFactor = self.contentScaleFactor;
    
    NSSet *allTouches = event.allTouches;
    
    for (UITouch *touch in allTouches) {
        
        xs[i] = [touch locationInView:self].x / contentScaleFactor;
        
        ys[i] = [touch locationInView:self].y / contentScaleFactor;
        
        ++i;
        
        if (i >= 2) {
            
            break;
        }
    }
    
    if (i == 1) {
        
        [_mapRender handleTouch:TOUCH_ACTION_DOWN firstFinger:CGPointMake(xs[0], ys[0]) secondFinger:CGPointMake(xs[1], ys[1])];
    }
    
    if (i >= 2) {
        
        [_mapRender handleTouch:TOUCH_MULTI_DOWN firstFinger:CGPointMake(xs[0], ys[0]) secondFinger:CGPointMake(xs[1], ys[1])];
    }
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event {
    
    float xs[IOS_MAX_TOUCHES_COUNT] = {0.0f};
    
    float ys[IOS_MAX_TOUCHES_COUNT] = {0.0f};
    
    int i = 0;
    
    CGFloat contentScaleFactor = self.contentScaleFactor;
    
    NSSet *allTouches = event.allTouches;
    
    for (UITouch *touch in allTouches) {
        
        xs[i] = [touch locationInView:self].x / contentScaleFactor;
        
        ys[i] = [touch locationInView:self].y / contentScaleFactor;
        
        ++i;
        
        if (i >= 2) {
            
            break;
        }
    }
    
    [_mapRender handleTouch:TOUCH_ACTION_MOVE firstFinger:CGPointMake(xs[0], ys[0]) secondFinger:CGPointMake(xs[1], ys[1])];
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event {
    
    float xs[IOS_MAX_TOUCHES_COUNT] = {0.0f};
    
    float ys[IOS_MAX_TOUCHES_COUNT] = {0.0f};
    
    int i = 0;
    
    CGFloat contentScaleFactor = self.contentScaleFactor;
    
    NSSet *allTouches = event.allTouches;
    
    for (UITouch *touch in allTouches) {
        
        xs[i] = [touch locationInView:self].x / contentScaleFactor;
        
        ys[i] = [touch locationInView:self].y / contentScaleFactor;
        
        ++i;
        
        if (i >= 2) {
            
            break;
        }
    }
    
    if (i == 1) {
        
        [_mapRender handleTouch:TOUCH_MULTI_UP firstFinger:CGPointMake(xs[0], ys[0]) secondFinger:CGPointMake(xs[1], ys[1])];
    }
    
    if (i >= 2) {
        
        [_mapRender handleTouch:TOUCH_ACTION_UP firstFinger:CGPointMake(xs[0], ys[0]) secondFinger:CGPointMake(xs[1], ys[1])];
    }
}

- (void)render:(CADisplayLink*)displayLink {
    
    [_glkView display];
}

- (void)configMap {
    
    NSString *glesPath = [[NSBundle mainBundle] pathForResource:@"LE" ofType:@"vbf"];
    
    _mapRender = [[MapRender alloc] initRender:glesPath];
    
    [_mapRender setSize:_glkView.bounds.size.width height:_glkView.bounds.size.height];
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
