//
//  MapRender.h
//  TestOpengles
//
//  Created by ky on 8/8/16.
//  Copyright © 2016 yellfun. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

@interface MapRender : NSObject

- (id)initRender:(NSString*)mapFile;

- (void)setSize:(CGFloat)width height:(CGFloat)height;

- (void)zoom:(CGFloat)zoomValue;

- (void)rotate:(CGFloat)angle;

- (void)pan:(CGFloat)x y:(CGFloat)y;

- (void)draw;

@end
