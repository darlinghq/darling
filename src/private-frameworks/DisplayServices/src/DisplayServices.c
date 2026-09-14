#include <CoreFoundation/CoreFoundation.h>
#include <stdint.h>
#include <stdbool.h>

int DisplayServicesGetBrightness(uint32_t display, float *brightness) {
	if (brightness) *brightness = 1.0f;
	return 0;
}

int DisplayServicesSetBrightness(uint32_t display, float brightness) {
	return 0;
}

int DisplayServicesGetLinearBrightness(uint32_t display, float *brightness) {
	if (brightness) *brightness = 1.0f;
	return 0;
}

int DisplayServicesSetLinearBrightness(uint32_t display, float brightness) {
	return 0;
}

int DisplayServicesCanChangeBrightness(uint32_t display) {
	return 1;
}

int DisplayServicesBrightnessChanged(uint32_t display, float brightness) {
	return 0;
}

int DisplayServicesRegisterForAmbientLightEvents(void) {
	return 0;
}

int DisplayServicesUnregisterForAmbientLightEvents(void) {
	return 0;
}
