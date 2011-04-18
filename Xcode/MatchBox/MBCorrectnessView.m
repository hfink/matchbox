//Copyright (c) 2011 Sebastian Böhm sebastian@sometimesfood.org
//                   Heinrich Fink hf@hfink.eu
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in
//all copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//THE SOFTWARE.

#import "MBCorrectnessView.h"


@implementation MBCorrectnessView

@synthesize correctnessLevel;

const UInt32 topbottom_inset = 60;
const UInt32 circle_diameter = 30;

- (id)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
        [self initialize];
    }
    return self;
}

- (id)initWithCoder:(NSCoder *)decoder
{
    self = [super initWithCoder:decoder];
    if (self) {
        [self initialize];
    }
    return self;    
}

- (void)initialize
{
    [self setCorrectnessLevel:0.5];
    
    CGRect rect = [self bounds];
    rect = CGRectInset(rect, 25, topbottom_inset);    
    
    roundedRect = [[UIBezierPath bezierPathWithRoundedRect:rect 
                                              cornerRadius:6.0f] retain];
    
    
    [roundedRect setLineWidth:3];
}

- (void)drawRect:(CGRect)rect
{
    
    //draw one line from top to bottom with a margin of 10px
    CGContextRef context = UIGraphicsGetCurrentContext();
    CGContextSetLineWidth(context, 1);
    
    [[UIColor darkGrayColor] setStroke];

    //TODO: fill with gradient
    
    [roundedRect stroke];
    
}


- (void)dealloc
{
    [roundedRect release];
    [super dealloc];
}

//- (void)setCorrectnessLevel:(CGFloat) level 
//              withAnimation:(BOOL) animate
//{
//    [self setCorrectnessLevel:level];
//    [self setNeedsDisplay];
//    //TODO: add animation support
//}

@end
