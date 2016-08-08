//
//  GLESUtils.h
//  TestOpengles
//
//  Created by ky on 16/8/5.
//  Copyright © 2016年 yellfun. All rights reserved.
//

#import <Foundation/Foundation.h>
#include <OpenGLES/ES2/gl.h>

@interface GLESUtils : NSObject

+ (GLuint)loadShader:(GLenum)type withFilepath:(NSString *)shaderFilepath;

+ (GLuint)loadShader:(GLenum)type withString:(NSString *)shaderString;

@end
