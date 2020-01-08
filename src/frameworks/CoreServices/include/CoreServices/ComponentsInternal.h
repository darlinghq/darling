#ifndef COMPONENTSINTERNAL_H
#define COMPONENTSINTERNAL_H

class CarbonComponent
{
public:
	virtual ~CarbonComponent() {}
};

inline static UInt32 GetComponentType(Component comp)
{
	long l = long(comp);
	return l & 0xffff0000;
}

inline static UInt32 GetComponentIndex(Component comp)
{
	long l = long(comp);
	return l & 0xffff;
}

inline static Component CreateComponent(UInt32 type, UInt32 index)
{
	long l = type & 0xffff0000;
	l |= (index & 0xffff);
	return (void*) l;
}

enum
{
	kComponentTypeAudioUnit = 'au\0\0'
};

#endif

