#import <Foundation/Foundation.h>

typedef struct {
    double red;
    double green;
    double blue;
    double alpha;
} MTLClearColor;

MTLClearColor MTLClearColorMake(double red, double green, double blue, double alpha);
