#import <Foundation/Foundation.h>

typedef struct {
    double red;
    double green;
    double blue;
    double alpha;
} MTLClearColor;

inline MTLClearColor MTLClearColorMake(double red, double green, double blue, double alpha) {
    MTLClearColor item = { red, green, blue, alpha, };
    return item;
}
