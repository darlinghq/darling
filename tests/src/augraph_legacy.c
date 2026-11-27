/*
 * Copyright (C) 2026 Lubos Dolezel
 *
 * The legacy AUGraph calls that take a ComponentDescription or work on the graph's output unit:
 * AUGraphNewNode, AUGraphGetNodeInfo, AUGraphAddRenderNotify, AUGraphRemoveRenderNotify,
 * AUGraphGetCPULoad and AUGraphGetMaxCPULoad. The graph uses the generic output unit, which needs no audio device.
 */
#include <AudioToolbox/AUComponent.h>
#include <AudioToolbox/AUGraph.h>
#include <stdio.h>
#include <string.h>

/* the layout of a Carbon ComponentDescription, which AUGraph's legacy calls take */
struct ComponentDescription {
	OSType componentType;
	OSType componentSubType;
	OSType componentManufacturer;
	UInt32 componentFlags;
	UInt32 componentFlagsMask;
};

static int failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); failures++; } } while (0)

static OSStatus notify(void* refCon, AudioUnitRenderActionFlags* flags, const AudioTimeStamp* ts, UInt32 bus,
	UInt32 frames, AudioBufferList* data)
{
	return noErr;
}

int main(void)
{
	AUGraph graph = NULL;
	AUNode node = 0;
	struct ComponentDescription cd;
	struct ComponentDescription back;
	UInt32 classSize = 123;
	void* classData = (void*) 1;
	AudioUnit unit = NULL;
	Float32 load = -1, maxLoad = -1;

	memset(&cd, 0, sizeof(cd));
	cd.componentType = kAudioUnitType_Output;
	cd.componentSubType = kAudioUnitSubType_GenericOutput;
	cd.componentManufacturer = kAudioUnitManufacturer_Apple;

	CHECK(NewAUGraph(&graph) == noErr);
	CHECK(AUGraphNewNode(graph, (void*) &cd, 0, NULL, &node) == noErr);
	CHECK(AUGraphOpen(graph) == noErr);

	memset(&back, 0, sizeof(back));
	CHECK(AUGraphGetNodeInfo(graph, node, (void*) &back, &classSize, &classData, &unit) == noErr);
	CHECK(back.componentType == kAudioUnitType_Output);
	CHECK(back.componentSubType == kAudioUnitSubType_GenericOutput);
	CHECK(unit != NULL);
	CHECK(classSize == 0);

	CHECK(AUGraphAddRenderNotify(graph, notify, NULL) == noErr);
	CHECK(AUGraphRemoveRenderNotify(graph, notify, NULL) == noErr);

	CHECK(AUGraphGetCPULoad(graph, &load) == noErr);
	CHECK(load >= 0 && load <= 1);
	CHECK(AUGraphGetMaxCPULoad(graph, &maxLoad) == noErr);
	CHECK(maxLoad >= 0 && maxLoad <= 1);

	CHECK(AUGraphClose(graph) == noErr);
	CHECK(DisposeAUGraph(graph) == noErr);
	if (!failures)
		puts("PASS");
	return failures != 0;
}
