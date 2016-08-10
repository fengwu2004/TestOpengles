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

- (void)draw;

@end
