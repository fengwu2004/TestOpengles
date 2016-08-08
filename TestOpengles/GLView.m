//
//  GLView.m
//  TestOpengles
//
//  Created by ky on 8/8/16.
//  Copyright © 2016 yellfun. All rights reserved.
//

#import "GLView.h"

typedef struct {
    float Position[3];
    float Color[4];
} Vertex;

const Vertex Vertices[] = {
    {{1, -1, 0}, {1, 0, 0, 1}},
    {{1, 1, 0}, {0, 1, 0, 1}},
    {{-1, 1, 0}, {0, 1, 1, 1}},
    {{-1, -1, 0}, {0, 0, 0, 1}}
};

const GLubyte Indices[] = {
    0, 1, 2,
    2, 3, 0
};


@interface GLView()<GLKViewDelegate>

@property (nonatomic, assign) GLuint positionSlot;
@property (nonatomic, assign) GLuint colorSlot;
@property (nonatomic, assign) GLuint programHandle;
@property (nonatomic, assign) GLuint vertexBuffer;
@property (nonatomic, assign) GLuint indexBuffer;
@end

@implementation GLView

- (instancetype)initWithFrame:(CGRect)frame {
    
    self = [super initWithFrame:frame];
    
    if (self) {
        
        [self createEAGContext];
        
        [self configure];
        
        [self addVertex];
        
        [self compileShaders];
    }
    
    return self;
}

- (void)createEAGContext {
    
    EAGLRenderingAPI api = kEAGLRenderingAPIOpenGLES2;
    
    _context = [[EAGLContext alloc] initWithAPI:api];
    
    [EAGLContext setCurrentContext:_context];
}

-(void)configure {
    
    _glkView = [[GLKView alloc] initWithFrame:self.bounds context:_context];
    
    _glkView.delegate = self;
    
    [self addSubview:_glkView];
    
    _glkView.drawableDepthFormat = GLKViewDrawableDepthFormat24;
}

- (void)addVertex {
    
    glGenBuffers(1, &_vertexBuffer);
    
    glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);
    
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
    
    glGenBuffers(1, &_indexBuffer);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBuffer);
    
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW);
}

- (GLuint)compileShader:(NSString*)shaderName withType:(GLenum)shaderType {
    
    // 1
    NSString* shaderPath = [[NSBundle mainBundle] pathForResource:shaderName ofType:@"glsl"];
    
    NSError* error;
    
    NSString* shaderString = [NSString stringWithContentsOfFile:shaderPath encoding:NSUTF8StringEncoding error:&error];
    
    if (!shaderString) {
        
        NSLog(@"Error loading shader: %@", error.localizedDescription);
        
        exit(1);
    }
    
    // 2
    GLuint shaderHandle = glCreateShader(shaderType);
    
    // 3
    const char* shaderStringUTF8 = [shaderString UTF8String];
    
    int32_t shaderStringLength = (int32_t)[shaderString length];
    
    glShaderSource(shaderHandle, 1, &shaderStringUTF8, &shaderStringLength);
    
    // 4
    glCompileShader(shaderHandle);
    
    // 5
    GLint compileSuccess;
    
    glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &compileSuccess);
    
    if (compileSuccess == GL_FALSE) {
        
        GLchar messages[256];
        
        glGetShaderInfoLog(shaderHandle, sizeof(messages), 0, &messages[0]);
        
        NSString *messageString = [NSString stringWithUTF8String:messages];
        
        NSLog(@"%@", messageString);
        
        exit(1);
    }
    
    return shaderHandle;
}

- (void)compileShaders {
    
    // 1
    GLuint vertexShader = [self compileShader:@"SimpleVertex" withType:GL_VERTEX_SHADER];
    
    GLuint fragmentShader = [self compileShader:@"SimpleFragment" withType:GL_FRAGMENT_SHADER];
    
    // 2
    _programHandle = glCreateProgram();
    
    glAttachShader(_programHandle, vertexShader);
    
    glAttachShader(_programHandle, fragmentShader);
    
    glLinkProgram(_programHandle);
    
    // 3
    GLint linkSuccess;
    
    glGetProgramiv(_programHandle, GL_LINK_STATUS, &linkSuccess);
    
    if (linkSuccess == GL_FALSE) {
        
        GLchar messages[256];
        
        glGetProgramInfoLog(_programHandle, sizeof(messages), 0, &messages[0]);
        
        NSString *messageString = [NSString stringWithUTF8String:messages];
        
        NSLog(@"%@", messageString);
        
        exit(1);
    }
    
    _positionSlot = glGetAttribLocation(_programHandle, "Position");
    
    _colorSlot = glGetAttribLocation(_programHandle, "SourceColor");
    
    glEnableVertexAttribArray(_positionSlot);
    
    glEnableVertexAttribArray(_colorSlot);
}

- (void)glkView:(GLKView *)view drawInRect:(CGRect)rect {
    
//    glClearColor(1, 1, 1, 1.0);
    
    glClear(GL_COLOR_BUFFER_BIT);
    
    glUseProgram(_programHandle);
    
    glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);
    
    glVertexAttribPointer(_positionSlot, 3, GL_FLOAT, GL_FALSE,
                          sizeof(Vertex), 0);
    
    glVertexAttribPointer(_colorSlot, 4, GL_FLOAT, GL_FALSE,
                          sizeof(Vertex), (GLvoid*) (sizeof(float) *3));
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBuffer);
    
    glDrawElements(GL_TRIANGLES, sizeof(Indices)/sizeof(GLubyte), GL_UNSIGNED_BYTE, (void*)0);
}

@end
