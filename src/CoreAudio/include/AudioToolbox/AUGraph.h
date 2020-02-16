#ifndef AUGRAPH_H
#define AUGRAPH_H
#include <AudioUnit/AudioUnit.h>
#include <AudioUnit/AudioUnitProperties.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct AUGraphData* AUGraph;
typedef SInt32 AUNode;

struct AudioUnitNodeConnection
{
	AUNode sourceNode;
	UInt32 sourceOutputNumber;
	AUNode destNode;
	UInt32 destInputNumber;
};

typedef struct AudioUnitNodeConnection AudioUnitNodeConnection;
typedef struct AudioUnitNodeConnection AUNodeConnection;

struct AUNodeRenderCallback
{
	AUNode destNode;
	AudioUnitElement destInputNumber;
	AURenderCallbackStruct cback;
};
typedef struct AUNodeRenderCallback AUNodeRenderCallback;

struct AUNodeInteraction
{
	UInt32 nodeInteractionType;
	union
	{
		AUNodeConnection connection;
		AUNodeRenderCallback inputCallback;
	} nodeInteraction;
};
typedef struct AUNodeInteraction AUNodeInteraction;

OSStatus AUGraphAddNode(AUGraph inGraph, const AudioComponentDescription *inDescription, AUNode *outNode);
OSStatus AUGraphAddRenderNotify(AUGraph inGraph, AURenderCallback inCallback, void *inRefCon);
OSStatus AUGraphClearConnections(AUGraph inGraph);
OSStatus AUGraphClose(AUGraph inGraph);
OSStatus AUGraphConnectNodeInput(AUGraph inGraph, AUNode inSourceNode, UInt32 inSourceOutputNumber, AUNode inDestNode, UInt32 inDestInputNumber);
OSStatus AUGraphCountNodeInteractions(AUGraph inGraph, AUNode inNode, UInt32 *outNumInteractions);
OSStatus AUGraphDisconnectNodeInput(AUGraph inGraph, AUNode inDestNode, UInt32 inDestInputNumber);
OSStatus AUGraphGetCPULoad(AUGraph inGraph, Float32 *outAverageCPULoad);
OSStatus AUGraphGetIndNode(AUGraph inGraph, UInt32 inIndex, AUNode *outNode);
OSStatus AUGraphGetInteractionInfo(AUGraph inGraph, UInt32 inInteractionIndex, AUNodeInteraction *outInteraction);
OSStatus AUGraphGetMaxCPULoad(AUGraph inGraph, Float32 *outMaxLoad);
OSStatus AUGraphGetNodeCount(AUGraph inGraph, UInt32 *outNumberOfNodes);
OSStatus AUGraphGetNodeInteractions(AUGraph inGraph, AUNode inNode, UInt32 *ioNumInteractions, AUNodeInteraction *outInteractions);
OSStatus AUGraphGetNumberOfInteractions(AUGraph inGraph, UInt32 *outNumInteractions);
OSStatus AUGraphInitialize(AUGraph inGraph);
OSStatus AUGraphIsInitialized(AUGraph inGraph, Boolean *outIsInitialized);
OSStatus AUGraphIsOpen(AUGraph inGraph, Boolean *outIsOpen);
OSStatus AUGraphIsRunning(AUGraph inGraph, Boolean *outIsRunning);
OSStatus AUGraphNodeInfo(AUGraph inGraph, AUNode inNode, AudioComponentDescription *outDescription, AudioUnit *outAudioUnit);
OSStatus AUGraphOpen(AUGraph inGraph);
OSStatus AUGraphRemoveNode(AUGraph inGraph, AUNode inNode);
OSStatus AUGraphRemoveRenderNotify(AUGraph inGraph, AURenderCallback inCallback, void *inRefCon);
OSStatus AUGraphSetNodeInputCallback(AUGraph inGraph, AUNode inDestNode, UInt32 inDestInputNumber, const AURenderCallbackStruct *inInputCallback);
OSStatus AUGraphStart(AUGraph inGraph);
OSStatus AUGraphStop(AUGraph inGraph);
OSStatus AUGraphUninitialize(AUGraph inGraph);
OSStatus AUGraphUpdate(AUGraph inGraph, Boolean *outIsUpdated);
OSStatus DisposeAUGraph(AUGraph inGraph);
OSStatus NewAUGraph(AUGraph *outGraph);

#ifdef __cplusplus
}
#endif


#endif
