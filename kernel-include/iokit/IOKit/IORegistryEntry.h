/*
 * Copyright (c) 1998-2019 Apple Inc. All rights reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 *
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */
/*
 * Copyright (c) 1998 Apple Computer, Inc.  All rights reserved.
 *
 * HISTORY
 *
 */


#ifndef _IOKIT_IOREGISTRYENTRY_H
#define _IOKIT_IOREGISTRYENTRY_H

#include <IOKit/IOTypes.h>
#include <libkern/c++/OSContainers.h>


extern const OSSymbol * gIONameKey;
extern const OSSymbol * gIOLocationKey;
extern const OSSymbol * gIORegistryEntryIDKey;
extern const OSSymbol * gIORegistryEntryPropertyKeysKey;

class IORegistryEntry;
class IORegistryPlane;
class IORegistryIterator;

typedef void (*IORegistryEntryApplierFunction)(IORegistryEntry * entry,
    void * context);

enum {
	kIORegistryIterateRecursively   = 0x00000001,
	kIORegistryIterateParents       = 0x00000002,
};

#ifdef KERNEL_PRIVATE
enum{
	kIORegistryEntryIndexedPropertyCLPC = 0,
	kIORegistryEntryIndexedPropertyCount,
};
#endif /* KERNEL_PRIVATE */

/*! @class IORegistryEntry : public OSObject
 *   @abstract The base class for all objects in the registry.
 *   @discussion The IORegistryEntry base class provides functions for describing graphs of connected registry entries, each with a dictionary-based property table. Entries may be connected in different planes, with differing topologies. Access to the registry is protected against multiple threads. Inside the kernel planes are specified with plane objects and are published by the creator - IOService exports the gIOServicePlane plane object for example. Non kernel clients specify planes by their name.
 */

class IORegistryEntry : public OSObject
{
	friend class IORegistryIterator;

	OSDeclareDefaultStructors(IORegistryEntry);

protected:
/*! @struct ExpansionData
 *   @discussion This structure will be used to expand the capablilties of this class in the future.
 */
	struct ExpansionData;

/*! @var reserved
 *   Reserved for future use.  (Internal use only)  */
	ExpansionData * reserved;

private:

	OSDictionary *      fRegistryTable;
	OSDictionary *      fPropertyTable;

public:
/* methods available in Mac OS X 10.1 or later */

/*! @function copyProperty
 *   @abstract Synchronized method to obtain a property from a registry entry or one of its parents (or children) in the hierarchy. Available in Mac OS X 10.1 or later.
 *   @discussion This method will search for a property, starting first with this registry entry's property table, then iterating recusively through either the parent registry entries or the child registry entries of this entry. Once the first occurrence is found, it will lookup and return the value of the property, using the OSDictionary::getObject semantics. The iteration keeps track of entries that have been recursed into previously to avoid loops. This method is synchronized with other IORegistryEntry accesses to the property table(s).
 *   @param aKey The property's name as a C-string.
 *   @param plane The plane to iterate over, eg. gIOServicePlane.
 *   @param options kIORegistryIterateRecursively may be set to recurse automatically into the registry hierarchy. Without this option, this method degenerates into the standard getProperty() call. kIORegistryIterateParents may be set to iterate the parents of the entry, in place of the children.
 *   @result The property value found, or zero. A reference on any found property is returned to caller, which should be released. */

	virtual OSObject * copyProperty( const char *           aKey,
	    const IORegistryPlane * plane,
	    IOOptionBits            options =
	    kIORegistryIterateRecursively |
	    kIORegistryIterateParents) const;

/*! @function copyProperty
 *   @abstract Synchronized method to obtain a property from a registry entry or one of its parents (or children) in the hierarchy. Available in Mac OS X 10.1 or later.
 *   @discussion This method will search for a property, starting first with this registry entry's property table, then iterating recusively through either the parent registry entries or the child registry entries of this entry. Once the first occurrence is found, it will lookup and return the value of the property, using the OSDictionary::getObject semantics. The iteration keeps track of entries that have been recursed into previously to avoid loops. This method is synchronized with other IORegistryEntry accesses to the property table(s).
 *   @param aKey The property's name as an OSString.
 *   @param plane The plane to iterate over, eg. gIOServicePlane.
 *   @param options kIORegistryIterateRecursively may be set to recurse automatically into the registry hierarchy. Without this option, this method degenerates into the standard getProperty() call. kIORegistryIterateParents may be set to iterate the parents of the entry, in place of the children.
 *   @result The property value found, or zero. A reference on any found property is returned to caller, which should be released. */

	virtual OSObject * copyProperty( const OSString *        aKey,
	    const IORegistryPlane * plane,
	    IOOptionBits            options =
	    kIORegistryIterateRecursively |
	    kIORegistryIterateParents) const;

/*! @function copyProperty
 *   @abstract Synchronized method to obtain a property from a registry entry or one of its parents (or children) in the hierarchy. Available in Mac OS X 10.1 or later.
 *   @discussion This method will search for a property, starting first with this registry entry's property table, then iterating recusively through either the parent registry entries or the child registry entries of this entry. Once the first occurrence is found, it will lookup and return the value of the property, using the OSDictionary::getObject semantics. The iteration keeps track of entries that have been recursed into previously to avoid loops. This method is synchronized with other IORegistryEntry accesses to the property table(s).
 *   @param aKey The property's name as an OSSymbol.
 *   @param plane The plane to iterate over, eg. gIOServicePlane.
 *   @param options kIORegistryIterateRecursively may be set to recurse automatically into the registry hierarchy. Without this option, this method degenerates into the standard getProperty() call. kIORegistryIterateParents may be set to iterate the parents of the entry, in place of the children.
 *   @result The property value found, or zero. A reference on any found property is returned to caller, which should be released. */

	virtual OSObject * copyProperty( const OSSymbol *        aKey,
	    const IORegistryPlane * plane,
	    IOOptionBits            options =
	    kIORegistryIterateRecursively |
	    kIORegistryIterateParents) const;

/*! @function copyParentEntry
 *   @abstract Returns an registry entry's first parent entry in a plane. Available in Mac OS X 10.1 or later.
 *   @discussion This function will return the parent to which a registry entry was first attached. Since the majority of registry entrys have only one provider, this is a useful simplification.
 *   @param plane The plane object.
 *   @result Returns the first parent of the registry entry, or zero if the entry is not attached into the registry in that plane. A reference on the entry is returned to caller, which should be released. */

	virtual IORegistryEntry * copyParentEntry( const IORegistryPlane * plane ) const;

/*! @function copyChildEntry
 *   @abstract Returns an registry entry's first child entry in a plane. Available in Mac OS X 10.1 or later.
 *   @discussion This function will return the child which first attached to a registry entry.
 *   @param plane The plane object.
 *   @result Returns the first child of the registry entry, or zero if the entry is not attached into the registry in that plane. A reference on the entry is returned to caller, which should be released. */

	virtual IORegistryEntry * copyChildEntry( const IORegistryPlane * plane ) const;

/* method available in Mac OS X 10.4 or later */
/*!
 *   @typedef Action
 *   @discussion Type and arguments of callout C function that is used when
 *  a runCommand is executed by a client.  Cast to this type when you want a C++
 *  member function to be used.  Note the arg1 - arg3 parameters are passed straight pass through to the action callout.
 *   @param target
 *       Target of the function, can be used as a refcon.  Note if a C++ function
 *  was specified, this parameter is implicitly the first parameter in the target
 *  member function's parameter list.
 *   @param arg0 Argument to action from run operation.
 *   @param arg1 Argument to action from run operation.
 *   @param arg2 Argument to action from run operation.
 *   @param arg3 Argument to action from run operation.
 */
	typedef IOReturn (*Action)(OSObject *target,
	    void *arg0, void *arg1,
	    void *arg2, void *arg3);

/*! @function runPropertyAction
 *   @abstract Single thread a call to an action w.r.t. the property lock
 *   @discussion Client function that causes the given action to be called in a manner that syncrhonises with the registry iterators and serialisers.  This functin can be used to synchronously manipulate the property table of this nub
 *   @param action Pointer to function to be executed in work-loop context.
 *   @param arg0 Parameter for action parameter, defaults to 0.
 *   @param arg1 Parameter for action parameter, defaults to 0.
 *   @param arg2 Parameter for action parameter, defaults to 0.
 *   @param arg3 Parameter for action parameter, defaults to 0.
 *   @result Returns the value of the Action callout.
 */
	virtual IOReturn runPropertyAction(Action action, OSObject *target,
	    void *arg0 = NULL, void *arg1 = NULL,
	    void *arg2 = NULL, void *arg3 = NULL);

private:
#if __LP64__
	OSMetaClassDeclareReservedUnused(IORegistryEntry, 0);
	OSMetaClassDeclareReservedUnused(IORegistryEntry, 1);
	OSMetaClassDeclareReservedUnused(IORegistryEntry, 2);
	OSMetaClassDeclareReservedUnused(IORegistryEntry, 3);
	OSMetaClassDeclareReservedUnused(IORegistryEntry, 4);
	OSMetaClassDeclareReservedUnused(IORegistryEntry, 5);
#else
	OSMetaClassDeclareReservedUsed(IORegistryEntry, 0);
	OSMetaClassDeclareReservedUsed(IORegistryEntry, 1);
	OSMetaClassDeclareReservedUsed(IORegistryEntry, 2);
	OSMetaClassDeclareReservedUsed(IORegistryEntry, 3);
	OSMetaClassDeclareReservedUsed(IORegistryEntry, 4);
	OSMetaClassDeclareReservedUsed(IORegistryEntry, 5);
#endif
	OSMetaClassDeclareReservedUnused(IORegistryEntry, 6);
	OSMetaClassDeclareReservedUnused(IORegistryEntry, 7);
	OSMetaClassDeclareReservedUnused(IORegistryEntry, 8);
	OSMetaClassDeclareReservedUnused(IORegistryEntry, 9);
	OSMetaClassDeclareReservedUnused(IORegistryEntry, 10);
	OSMetaClassDeclareReservedUnused(IORegistryEntry, 11);
	OSMetaClassDeclareReservedUnused(IORegistryEntry, 12);
	OSMetaClassDeclareReservedUnused(IORegistryEntry, 13);
	OSMetaClassDeclareReservedUnused(IORegistryEntry, 14);
	OSMetaClassDeclareReservedUnused(IORegistryEntry, 15);
	OSMetaClassDeclareReservedUnused(IORegistryEntry, 16);
	OSMetaClassDeclareReservedUnused(IORegistryEntry, 17);
	OSMetaClassDeclareReservedUnused(IORegistryEntry, 18);
	OSMetaClassDeclareReservedUnused(IORegistryEntry, 19);
	OSMetaClassDeclareReservedUnused(IORegistryEntry, 20);
	OSMetaClassDeclareReservedUnused(IORegistryEntry, 21);
	OSMetaClassDeclareReservedUnused(IORegistryEntry, 22);
	OSMetaClassDeclareReservedUnused(IORegistryEntry, 23);
	OSMetaClassDeclareReservedUnused(IORegistryEntry, 24);
	OSMetaClassDeclareReservedUnused(IORegistryEntry, 25);
	OSMetaClassDeclareReservedUnused(IORegistryEntry, 26);
	OSMetaClassDeclareReservedUnused(IORegistryEntry, 27);
	OSMetaClassDeclareReservedUnused(IORegistryEntry, 28);
	OSMetaClassDeclareReservedUnused(IORegistryEntry, 29);
	OSMetaClassDeclareReservedUnused(IORegistryEntry, 30);
	OSMetaClassDeclareReservedUnused(IORegistryEntry, 31);

public:

/* Registry accessors */

/*! @function getRegistryRoot
 *   @abstract Returns a pointer to the root instance of the registry.
 *   @discussion This method provides an accessor to the root of the registry for the machine. The root may be passed to a registry iterator when iterating a plane, and contains properties that describe the available planes, and diagnostic information for IOKit. Keys for these properties are in IOKitKeys.h.
 *   @result A pointer to the IORegistryEntry root instance. It should not be released by the caller. */

	static IORegistryEntry * getRegistryRoot( void );

/*! @function getGenerationCount
 *   @abstract Returns an generation count for all registry changing operations.
 *   @discussion This method provides an accessor to the current generation count (or seed) of the registry which changes when any topology change occurs in the registry - this does not include property table changes. It may be used to invalidate any caching of the results from IORegistryEntry methods.
 *   @result An integer generation count. */

	static SInt32            getGenerationCount( void );

/*! @function getPlane
 *   @abstract Looks up the plane object by a C-string name.
 *   @discussion Planes are usually provided as globals by the creator, eg. gIOServicePlane, gIODeviceTreePlane, or gIOAudioPlane, however they may also be looked up by name with this method.
 *   @result A pointer to the plane object, or zero if no such plane exists. The returned plane should not be released. */

	static const IORegistryPlane * getPlane( const char * name );

/* Registry Entry allocation & init */

/*! @function init
 *   @abstract Standard init method for all IORegistryEntry subclasses.
 *   @discussion A registry entry must be initialized with this method before it can be used. A property dictionary may passed and will be retained by this method for use as the registry entry's property table, or an empty one will be created.
 *   @param dictionary A dictionary that will become the registry entry's property table (retaining it), or zero which will cause an empty property table to be created.
 *   @result true on success, or false on a resource failure. */

	virtual bool init( OSDictionary * dictionary = NULL );

/*! @function free
 *   @abstract Standard free method for all IORegistryEntry subclasses.
 *   @discussion This method will release any resources of the entry, in particular its property table. Note that the registry entry must always be detached from the registry before free may be called, and subclasses (namely IOService) will have additional protocols for removing registry entries. free should never need be called directly. */

	virtual void free( void ) APPLE_KEXT_OVERRIDE;

/*! @function setPropertyTable
 *   @abstract Replace a registry entry's property table.
 *   @discussion This method will release the current property table of a the entry and replace it with another, retaining the new property table.
 *   @param dict The new dictionary to be used as the entry's property table. */

	virtual void setPropertyTable( OSDictionary * dict );

/* Synchronized property accessors; wrappers to OSDictionary
 * plus property creation helpers */

/*! @function setProperty
 *   @abstract Synchronized method to add a property to a registry entry's property table.
 *   @discussion This method will add or replace a property in a registry entry's property table, using the OSDictionary::setObject semantics. This method is synchronized with other IORegistryEntry accesses to the property table.
 *   @param aKey The properties name as an OSSymbol.
 *   @param anObject The property value.
 *   @result true on success or false on a resource failure. */

	virtual bool setProperty(const OSSymbol * aKey,
	    OSObject * anObject);

#ifdef KERNEL_PRIVATE
	OSObject * setIndexedProperty(uint32_t index, OSObject * anObject);
	OSObject * getIndexedProperty(uint32_t index) const;
#endif /* KERNEL_PRIVATE */

/*! @function setProperty
 *   @abstract Synchronized method to add a property to a registry entry's property table.
 *   @discussion This method will add or replace a property in a registry entry's property table, using the OSDictionary::setObject semantics. This method is synchronized with other IORegistryEntry accesses to the property table.
 *   @param aKey The property's name as an OSString.
 *   @param anObject The property value.
 *   @result true on success or false on a resource failure. */

	virtual bool setProperty(const OSString * aKey, OSObject * anObject);

/*! @function setProperty
 *   @abstract Synchronized method to add a property to a registry entry's property table.
 *   @discussion This method will add or replace a property in a registry entry's property table, using the OSDictionary::setObject semantics. This method is synchronized with other IORegistryEntry accesses to the property table.
 *   @param aKey The property's name as a C-string.
 *   @param anObject The property value.
 *   @result true on success or false on a resource failure. */

	virtual bool setProperty(const char * aKey, OSObject * anObject);

/*! @function setProperty
 *   @abstract Synchronized method to construct and add a OSString property to a registry entry's property table.
 *   @discussion This method will add or replace a property in a registry entry's property table, using the OSDictionary::setObject semantics. This method is synchronized with other IORegistryEntry accesses to the property table. The property is created as an OSString from the supplied C-string, set in the property table with the given name, and released.
 *   @param aKey The property's name as a C-string.
 *   @param aString The property value as a C-string.
 *   @result true on success or false on a resource failure. */

	virtual bool setProperty(const char * aKey, const char * aString);

/*! @function setProperty
 *   @abstract Synchronized method to construct and add an OSBoolean property to a registry entry's property table.
 *   @discussion This method will add or replace a property in a registry entry's property table, using the OSDictionary::setObject semantics. This method is synchronized with other IORegistryEntry accesses to the property table. The property is created as an OSBoolean from the supplied value, set in the property table with the given name, and released.
 *   @param aKey The property's name as a C-string.
 *   @param aBoolean The property's boolean value.
 *   @result true on success or false on a resource failure. */

	virtual bool setProperty(const char * aKey, bool aBoolean);

/*! @function setProperty
 *   @abstract Synchronized method to construct and add an OSNumber property to a registry entry's property table.
 *   @discussion This method will add or replace a property in a registry entry's property table, using the OSDictionary::setObject semantics. This method is synchronized with other IORegistryEntry accesses to the property table. The property is created as an OSNumber from the supplied value and size, set in the property table with the given name, and released.
 *   @param aKey The property's name as a C-string.
 *   @param aValue The property's numeric value.
 *   @param aNumberOfBits The property's size in bits, for OSNumber.
 *   @result true on success or false on a resource failure. */

	virtual bool setProperty( const char *       aKey,
	    unsigned long long aValue,
	    unsigned int       aNumberOfBits);

/*! @function setProperty
 *   @abstract Synchronized method to construct and add an OSData property to a registry entry's property table.
 *   @discussion This method will add or replace a property in a registry entry's property table, using the OSDictionary::setObject semantics. This method is synchronized with other IORegistryEntry accesses to the property table. The property is created as an OSData copied from the supplied data and length, set in the property table with the given name, and released.
 *   @param aKey The property's name as a C-string.
 *   @param bytes The property's value as a pointer. OSData will copy this data.
 *   @param length The property's size in bytes, for OSData.
 *   @result true on success or false on a resource failure. */

	virtual bool setProperty( const char *       aKey,
	    void *             bytes,
	    unsigned int       length);

/*! @function removeProperty
 *   @abstract Synchronized method to remove a property from a registry entry's property table.
 *   @discussion This method will remove a property from a registry entry's property table, using the OSDictionary::removeObject semantics. This method is synchronized with other IORegistryEntry accesses to the property table.
 *   @param aKey The property's name as an OSSymbol. */

	virtual void removeProperty( const OSSymbol * aKey);

/*! @function removeProperty
 *   @abstract Synchronized method to remove a property from a registry entry's property table.
 *   @discussion This method will remove a property from a registry entry's property table, using the OSDictionary::removeObject semantics. This method is synchronized with other IORegistryEntry accesses to the property table.
 *   @param aKey The property's name as an OSString. */

	virtual void removeProperty( const OSString * aKey);

/*! @function removeProperty
 *   @abstract Synchronized method to remove a property from a registry entry's property table.
 *   @discussion This method will remove a property from a registry entry's property table, using the OSDictionary::removeObject semantics. This method is synchronized with other IORegistryEntry accesses to the property table.
 *   @param aKey The property's name as a C-string. */

	virtual void removeProperty( const char * aKey);

/*! @function getProperty
 *   @abstract Synchronized method to obtain a property from a registry entry's property table.
 *   @discussion This method will lookup a property in a registry entry's property table, using the OSDictionary::getObject semantics. This method is synchronized with other IORegistryEntry accesses to the property table.
 *   @param aKey The property's name as an OSSymbol.
 *   @result The property value found, or zero. */

	virtual OSObject * getProperty( const OSSymbol * aKey) const;

/*! @function getProperty
 *   @abstract Synchronized method to obtain a property from a registry entry's property table.
 *   @discussion This method will lookup a property in a registry entry's property table, using the OSDictionary::getObject semantics. This method is synchronized with other IORegistryEntry accesses to the property table.
 *   @param aKey The property's name as an OSString.
 *   @result The property value found, or zero. */

	virtual OSObject * getProperty( const OSString * aKey) const;

/*! @function getProperty
 *   @abstract Synchronized method to obtain a property from a registry entry's property table.
 *   @discussion This method will lookup a property in a registry entry's property table, using the OSDictionary::getObject semantics. This method is synchronized with other IORegistryEntry accesses to the property table.
 *   @param aKey The property's name as a C-string.
 *   @result The property value found, or zero. */

	virtual OSObject * getProperty( const char * aKey) const;

/*! @function getProperty
 *   @abstract Synchronized method to obtain a property from a registry entry or one of its parents (or children) in the hierarchy.
 *   @discussion This method will search for a property, starting first with this registry entry's property table, then iterating recusively through either the parent registry entries or the child registry entries of this entry. Once the first occurrence is found, it will lookup and return the value of the property, using the OSDictionary::getObject semantics. The iteration keeps track of entries that have been recursed into previously to avoid loops. This method is synchronized with other IORegistryEntry accesses to the property table(s).
 *   @param aKey The property's name as an OSSymbol.
 *   @param plane The plane to iterate over, eg. gIOServicePlane.
 *   @param options kIORegistryIterateRecursively may be set to recurse automatically into the registry hierarchy. Without this option, this method degenerates into the standard getProperty() call. kIORegistryIterateParents may be set to iterate the parents of the entry, in place of the children.
 *   @result The property value found, or zero. */

	virtual OSObject * getProperty( const OSSymbol *        aKey,
	    const IORegistryPlane * plane,
	    IOOptionBits            options =
	    kIORegistryIterateRecursively |
	    kIORegistryIterateParents) const;

/*! @function getProperty
 *   @abstract Synchronized method to obtain a property from a registry entry or one of its parents (or children) in the hierarchy.
 *   @discussion This method will search for a property, starting first with this registry entry's property table, then iterating recusively through either the parent registry entries or the child registry entries of this entry. Once the first occurrence is found, it will lookup and return the value of the property, using the OSDictionary::getObject semantics. The iteration keeps track of entries that have been recursed into previously to avoid loops. This method is synchronized with other IORegistryEntry accesses to the property table(s).
 *   @param aKey The property's name as an OSString.
 *   @param plane The plane to iterate over, eg. gIOServicePlane.
 *   @param options kIORegistryIterateRecursively may be set to recurse automatically into the registry hierarchy. Without this option, this method degenerates into the standard getProperty() call. kIORegistryIterateParents may be set to iterate the parents of the entry, in place of the children.
 *   @result The property value found, or zero. */

	virtual OSObject * getProperty( const OSString *        aKey,
	    const IORegistryPlane * plane,
	    IOOptionBits            options =
	    kIORegistryIterateRecursively |
	    kIORegistryIterateParents) const;

/*! @function getProperty
 *   @abstract Synchronized method to obtain a property from a registry entry or one of its parents (or children) in the hierarchy.
 *   @discussion This method will search for a property, starting first with this registry entry's property table, then iterating recusively through either the parent registry entries or the child registry entries of this entry. Once the first occurrence is found, it will lookup and return the value of the property, using the OSDictionary::getObject semantics. The iteration keeps track of entries that have been recursed into previously to avoid loops. This method is synchronized with other IORegistryEntry accesses to the property table(s).
 *   @param aKey The property's name as a C-string.
 *   @param plane The plane to iterate over, eg. gIOServicePlane.
 *   @param options kIORegistryIterateRecursively may be set to recurse automatically into the registry hierarchy. Without this option, this method degenerates into the standard getProperty() call. kIORegistryIterateParents may be set to iterate the parents of the entry, in place of the children.
 *   @result The property value found, or zero. */

	virtual OSObject * getProperty( const char *            aKey,
	    const IORegistryPlane * plane,
	    IOOptionBits            options =
	    kIORegistryIterateRecursively |
	    kIORegistryIterateParents) const;

/*! @function copyProperty
 *   @abstract Synchronized method to obtain a property from a registry entry's property table.
 *   @discussion This method will lookup a property in a registry entry's property table, using the OSDictionary::getObject semantics, and return a reference to the caller. This method is synchronized with other IORegistryEntry accesses to the property table.
 *   @param aKey The property's name as an OSSymbol.
 *   @result The property value found, or zero. It should be released by the caller. */

	virtual OSObject * copyProperty( const OSSymbol * aKey) const;

/*! @function copyProperty
 *   @abstract Synchronized method to obtain a property from a registry entry's property table.
 *   @discussion This method will lookup a property in a registry entry's property table, using the OSDictionary::getObject semantics, and return a reference to the caller. This method is synchronized with other IORegistryEntry accesses to the property table.
 *   @param aKey The property's name as an OSString.
 *   @result The property value found, or zero. It should be released by the caller. */

	virtual OSObject * copyProperty( const OSString * aKey) const;

/*! @function copyProperty
 *   @abstract Synchronized method to obtain a property from a registry entry's property table.
 *   @discussion This method will lookup a property in a registry entry's property table, using the OSDictionary::getObject semantics, and return a reference to the caller. This method is synchronized with other IORegistryEntry accesses to the property table.
 *   @param aKey The property's name as a C-string.
 *   @result The property value found, or zero. It should be released by the caller. */

	virtual OSObject * copyProperty( const char * aKey) const;

/*! @function dictionaryWithProperties
 *   @abstract Synchronized method to obtain copy a registry entry's property table.
 *   @discussion This method will copy a registry entry's property table, using the OSDictionary::withDictionary semantics. This method is synchronized with other IORegistryEntry accesses to the property table. Since OSDictionary will only copy property values by reference, synchronization is not guaranteed to any collection values.
 *   @result The created dictionary, or zero on a resource value. It should be released by the caller. */

	virtual OSDictionary * dictionaryWithProperties( void ) const;

/*! @function serializeProperties
 *   @abstract Synchronized method to serialize a registry entry's property table.
 *   @discussion This method will serialize a registry entry's property table, using the OSDictionary::serialize semantics. This method is synchronized with other IORegistryEntry accesses to the property table. Many non-kernel clients of IOKit read information from the registry via properties, and will invoke this method in a registry entry to create a serialization of all the entry's properties, which is then reconstructed in the client's task as a CFDictionary. This method may be intercepted by subclasses to update their properties or implement a different serialization method, though it is usually better to implement such functionality by creating objects in the property table and implementing their serialize methods, avoiding any need to implement serializeProperties.
 *   @param serialize The OSSerialize instance representing the serialization request.
 *   @result True on success, false otherwise. */

	virtual bool serializeProperties( OSSerialize * serialize ) const;

/* Unsynchronized(!) property table access */

/*! @function getPropertyTable
 *   @abstract Unsynchronized accessor to a registry entry's property table.
 *   @discussion This method will return a pointer to the live property table as an OSDictionery. Its use is not recommended in most cases, instead use the synchronized accessors and helper functions of IORegistryEntry to access properties. It can only safely be used by one thread, which usually means it can only be used before a registry entry is entered into the registry.
 *   @result A pointer to the property table as an OSDictionary. The pointer is valid while the registry entry is retained, and should not be released by the caller. */

/* inline */ OSDictionary * getPropertyTable( void ) const;
/* { return(fPropertyTable); } */

/* Set properties from user level, to be overridden if supported */

/*! @function setProperties
 *   @abstract Optionally supported external method to set properties in a registry entry.
 *   @discussion This method is not implemented by IORegistryEntry, but is available to kernel and non-kernel clients to set properties in a registry entry. IOUserClient provides connection based, more controlled access to this functionality and may be more appropriate for many uses, since there is no differentiation between clients available to this method.
 *   @param properties Any OSObject subclass, to be interpreted by the implementing method - for example an OSDictionary, OSData etc. may all be appropriate.
 *   @result An IOReturn code to be returned to the caller. */

	virtual IOReturn setProperties( OSObject * properties );

/* Topology */

/*! @function getParentIterator
 *   @abstract Returns an iterator over an registry entry's parent entries in a specified plane.
 *   @param plane The plane object.
 *   @result Returns an iterator over the parents of the registry entry, or zero if there is a resource failure. The iterator must be released when the iteration is finished. All objects returned by the iteration are retained while the iterator is valid, though they may no longer be attached during the iteration. */

	virtual OSIterator * getParentIterator( const IORegistryPlane * plane )
	const;
	virtual void applyToParents( IORegistryEntryApplierFunction applier,
	    void * context,
	    const IORegistryPlane * plane ) const;

/*! @function getParentEntry
 *   @abstract Returns an registry entry's first parent entry in a plane.
 *   @discussion This function will return the parent to which a registry entry was first attached. Since the majority of registry entrys have only one provider, this is a useful simplification.
 *   @param plane The plane object.
 *   @result Returns the first parent of the registry entry, or zero if the entry is not attached into the registry in that plane. The parent is retained while the entry is attached, and should not be released by the caller. */

	virtual IORegistryEntry * getParentEntry( const IORegistryPlane * plane ) const;

/*! @function getChildIterator
 *   @abstract Returns an iterator over an registry entry's child entries in a plane.
 *   @discussion This method creates an iterator which will return each of a registry entry's child entries in a specified plane.
 *   @param plane The plane object.
 *   @result Returns an iterator over the children of the entry, or zero if there is a resource failure. The iterator must be released when the iteration is finished. All objects returned by the iteration are retained while the iterator is valid, though they may no longer be attached during the iteration. */

	virtual OSIterator * getChildIterator( const IORegistryPlane * plane )
	const;

#if XNU_KERNEL_PRIVATE
	uint32_t getChildCount( const IORegistryPlane * plane ) const;
	OSArray * copyPropertyKeys(void) const;
#endif

	virtual void applyToChildren( IORegistryEntryApplierFunction applier,
	    void * context,
	    const IORegistryPlane * plane ) const;

/*! @function getChildEntry
 *   @abstract Returns an registry entry's first child entry in a plane.
 *   @discussion This function will return the child which first attached to a registry entry.
 *   @param plane The plane object.
 *   @result Returns the first child of the registry entry, or zero if the entry is not attached into the registry in that plane. The child is retained while the entry is attached, and should not be released by the caller. */

	virtual IORegistryEntry * getChildEntry( const IORegistryPlane * plane ) const;

/*! @function isChild
 *   @abstract Determines whether a registry entry is the child of another in a plane.
 *   @discussion This method called in the parent entry determines if the specified entry is a child, in a plane. Additionally, it can check if the child is the only child of the parent entry.
 *   @param child The possible child registry entry.
 *   @param plane The plane object.
 *   @param onlyChild If true, check also if the child is the only child.
 *   @result If the child argument is not a child of the registry entry, false is returned. If onlyChild is true and the child is not the only child of the entry, false is returned, otherwise true is returned. */

	virtual bool isChild( IORegistryEntry * child,
	    const IORegistryPlane * plane,
	    bool onlyChild = false ) const;

/*! @function isParent
 *   @abstract Determines whether a registry entry is the parent of another in a plane.
 *   @discussion This method called in the child entry determines if the specified entry is a parent, in a plane. Additionally, it can check if the parent is the only parent of the child entry.
 *   @param parent The possible parent registry entry.
 *   @param plane The plane object.
 *   @param onlyParent If true, check also if the parent is the only parent.
 *   @result If the parent argument is not a parent of the registry entry, false is returned. If onlyParent is true and the parent is not the only parent of the entry, false is returned, otherwise true is returned. */

	virtual bool isParent( IORegistryEntry * parent,
	    const IORegistryPlane * plane,
	    bool onlyParent = false ) const;

/*! @function inPlane
 *   @abstract Determines whether a registry entry is attached in a plane.
 *   @discussion This method determines if the entry is attached in a plane to any other entry.  It can also be used to determine if the entry is a member of any plane.
 *   @param plane The plane object, 0 indicates any plane.
 *   @result If the entry has a parent in the given plane or if plane = 0 then if entry has any parent; return true, otherwise false. */

	virtual bool inPlane( const IORegistryPlane * plane = NULL) const;

/*! @function getDepth
 *   @abstract Counts the maximum number of entries between an entry and the registry root, in a plane.
 *   @discussion This method counts the number of entries between and entry and the registry root, in a plane, for each parent of the entry and returns the maximum value.
 *   @param plane The plane object.
 *   @result The maximum number of entries between the entry and the root. Zero is returned if the entry is not attached in the plane. */

	virtual unsigned int getDepth( const IORegistryPlane * plane ) const;

/* Attach / detach */

/*! @function attachToParent
 *   @abstract Attaches a entry to a parent entry in a plane.
 *   @discussion This is the usual method of entering an entry into the registry. It is a no-op and success if the entry is already attached to the parent. Attaching the  entry into the registry retains both the child and parent while they are attached. This method will call attachToChild in the parent entry if it is not being called from attachToChild.
 *   @param parent The registry entry to attach to.
 *   @param plane The plane object.
 *   @result true on success, or false on a resource failure, or if the parent is the same as the child. */

	virtual bool attachToParent( IORegistryEntry * parent,
	    const IORegistryPlane * plane );

/*! @function detachFromParent
 *   @abstract Detaches an entry from a parent entry in a plane.
 *   @discussion This is the usual method of removing an entry from the registry. It is a no-op if the entry is not attached to the parent. Detaching the entry will release both the child and parent. This method will call detachFromChild in the parent entry if it is not being called from detachFromChild.
 *   @param parent The registry entry to detach from.
 *   @param plane The plane object. */

	virtual void detachFromParent( IORegistryEntry * parent,
	    const IORegistryPlane * plane );

/*! @function attachToChild
 *   @abstract Method called in the parent entry when a child attaches.
 *   @discussion This method is called in the parent entry when a child attaches, to make overrides possible. This method will also call attachToParent in the child entry if it is not being called from attachToParent. It is a no-op and success if the entry is already a child. Attaching the  entry into the registry retains both the child and parent while they are attached.
 *   @param child The registry entry being attached.
 *   @param plane The plane object.
 *   @result true on success, or false on a resource failure, or if the parent is the same as the child. */

	virtual bool attachToChild( IORegistryEntry * child,
	    const IORegistryPlane * plane );

/*! @function detachFromChild
 *   @abstract Detaches a child entry from its parent in a plane.
 *   @discussion This method is called in the parent entry when a child detaches, to make overrides possible. It is a no-op if the entry is not a child of the parent. Detaching the entry will release both the child and parent. This method will call detachFromParent in the child entry if it is not being called from detachFromParent.
 *   @param child The registry entry to detach.
 *   @param plane The plane object. */

	virtual void detachFromChild( IORegistryEntry * child,
	    const IORegistryPlane * plane );

/*! @function detachAbove
 *   @abstract Detaches an entry from all its parent entries in a plane.
 *   @discussion This method calls detachFromParent in the entry for each of its parent entries in the plane.
 *   @param plane The plane object. */

	virtual void detachAbove( const IORegistryPlane * plane );

/*! @function detachAll
 *   @abstract Detaches an entry and all its children recursively in a plane.
 *   @discussion This method breaks the registry connections for a subtree. detachAbove is called in the entry, and all child entries and their children in the plane.
 *   @param plane The plane object. */

	virtual void detachAll( const IORegistryPlane * plane );

/* Name, location and path accessors */

/*! @function getName
 *   @abstract Returns the name assigned to the registry entry as a C-string.
 *   @discussion Entries can be named in a particular plane, or globally. If the entry is named in plane and the plane is specified that name will be returned, otherwise the global name is returned. The global name defaults to the entry's meta class name if it has not been named.
 *   @param plane The plane object, or zero for the global name.
 *   @result A C-string name, valid while the entry is retained. */

	virtual const char * getName( const IORegistryPlane * plane = NULL ) const;

/*! @function copyName
 *   @abstract Returns the name assigned to the registry entry as an OSSymbol.
 *   @discussion Entries can be named in a particular plane, or globally. If the entry is named in plane and the plane is specified that name will be returned, otherwise the global name is returned. The global name defaults to the entry's meta class name if it has not been named.
 *   @param plane The plane object, or zero for the global name.
 *   @result A reference to an OSSymbol for the name, which should be released by the caller. */

	virtual const OSSymbol * copyName(
		const IORegistryPlane * plane = NULL ) const;

/*! @function compareNames
 *   @abstract Compares the name of the entry with one or more names, and optionally returns the matching name.
 *   @discussion This method is called during IOService name matching and elsewhere to compare the entry's global name with a list of names, or a single name. A list of names may be passed as any OSCollection of OSStrings, while a single name may be passed an OSString, in the name parameter. compareNames will call the compareName method for each name, for overrides.
 *   @param name The name or names to compare with as any OSCollection (eg. OSArray, OSSet, OSDictionary) of OSStrings, or a single name may be passed an OSString.
 *   @param matched If the caller wants the successfully matched name returned, pass a non-zero pointer for the matched parameter and an OSString will be returned here. It should be released by the caller.
 *   @result True if one of the names compared true with the entry's global name. */

	virtual bool compareNames( OSObject * name, OSString ** matched = NULL ) const;

/*! @function compareName
 *   @abstract Compares the name of the entry with one name, and optionally returns the matching name.
 *   @discussion This method is called during IOService name matching and elsewhere from the compareNames method. It should be overridden to provide non-standard name matching.
 *   @param name The name to compare with as an OSString.
 *   @param matched If the caller wants the successfully matched name returned, pass a non-zero pointer for the matched parameter and an OSString will be returned here. It should be released by the caller. Generally, this will be the same as the name parameter, but may not be if wildcards are used.
 *   @result True if the name compared true with the entry's global name. */

	virtual bool compareName( OSString * name, OSString ** matched = NULL ) const;

/*! @function setName
 *   @abstract Sets a name for the registry entry, in a particular plane, or globally.
 *   @discussion Entries can be named in a particular plane, or globally. If the plane is specified the name applies only to that plane, otherwise the global name is set. The global name defaults to the entry's meta class name if it has not been named.
 *   @param name An OSSymbol which will be retained.
 *   @param plane The plane object, or zero to set the global name. */

	virtual void setName( const OSSymbol * name,
	    const IORegistryPlane * plane = NULL );

/*! @function setName
 *   @abstract Sets a name for the registry entry, in a particular plane, or globally.
 *   @discussion Entries can be named in a particular plane, or globally. If the plane is specified the name applies only to that plane, otherwise the global name is set. The global name defaults to the entry's meta class name if it has not been named.
 *   @param name A const C-string name which will be copied.
 *   @param plane The plane object, or zero to set the global name. */

	virtual void setName( const char * name,
	    const IORegistryPlane * plane = NULL );

/*! @function getLocation
 *   @abstract Returns the location string assigned to the registry entry as a C-string.
 *   @discussion Entries can given a location string in a particular plane, or globally. If the entry has had a location set in a plane and the plane is specified that location string will be returned, otherwise the global location string is returned. If no global location string has been set, zero is returned.
 *   @param plane The plane object, or zero for the global name.
 *   @result A C-string location string, valid while the entry is retained, or zero. */

	virtual const char * getLocation( const IORegistryPlane * plane = NULL ) const;

/*! @function copyLocation
 *   @abstract Returns the location string assigned to the registry entry as an OSSymbol.
 *   @discussion Entries can given a location string in a particular plane, or globally. If the entry has had a location set in a plane and the plane is specified that location string will be returned, otherwise the global location string is returned. If no global location string has been set, zero is returned.
 *   @param plane The plane object, or zero for the global name.
 *   @result A reference to an OSSymbol for the location if one exists, which should be released by the caller, or zero. */

	virtual const OSSymbol * copyLocation(
		const IORegistryPlane * plane = NULL ) const;

/*! @function setLocation
 *   @abstract Sets a location string for the registry entry, in a particular plane, or globally.
 *   @discussion Entries can be given a location string in a particular plane, or globally. If the plane is specified the location applies only to that plane, otherwise the global location is set. The location string may be used during path lookups of registry entries, to distinguish between sibling entries with the same name. The default IORegistryEntry parsing of location strings expects a list of hex numbers separated by commas, though subclasses of IORegistryEntry might do their own parsing.
 *   @param location A C-string location string which will be copied, or an OSSymbol which will be retained.
 *   @param plane The plane object, or zero to set the global location string. */

	virtual void setLocation( const OSSymbol * location,
	    const IORegistryPlane * plane = NULL );
	virtual void setLocation( const char * location,
	    const IORegistryPlane * plane = NULL );

/*! @function getPath
 *   @abstract Create a path for a registry entry.
 *   @discussion The path for a registry entry is copied to the caller's buffer. The path describes the entry's attachment in a particular plane, which must be specified. The path begins with the plane name followed by a colon, and then followed by '/' separated path components for each of the entries between the root and the registry entry. Each component is constructed with the getPathComponent method called in each entry. An alias may also exist for the entry, which are described as properties in a registry entry found at /aliases in the plane. If a property value interpreted as a path in a call to IORegistryEntry::fromPath yields the entry, then the property name is used as the entry's path.
 *   @param path A char buffer allocated by the caller.
 *   @param length An in/out parameter - the caller sets the length of the buffer available, and getPath returns the total length of the path copied to the buffer.
 *   @param plane The plane object.
 *   @result getPath will fail if the entry is not attached in the plane, or if the buffer is not large enough to contain the path. */

	virtual bool getPath( char * path, int * length,
	    const IORegistryPlane * plane) const;

/*! @function getPathComponent
 *   @abstract Create a path component for a registry entry.
 *   @discussion Each component of a path created with getPath is created with getPathComponent. The default implementation concatenates the entry's name in the the plane, with the "at" symbol and the location string of the entry in the plane if it has been set.
 *   @param path A char buffer allocated by the caller.
 *   @param length An in/out parameter - the caller sets the length of the buffer available, and getPathComponent returns the total length of the path component copied to the buffer.
 *   @param plane The plane object.
 *   @result true if the path fits into the supplied buffer or false on a overflow. */

	virtual bool getPathComponent( char * path, int * length,
	    const IORegistryPlane * plane ) const;

/*! @function fromPath
 *   @abstract Looks up a registry entry by path.
 *   @discussion This function parses paths to lookup registry entries. The path may begin with the <plane name>: created by getPath, or the plane may be set by the caller. If there are characters remaining unparsed after an entry has been looked up, this may be considered an invalid lookup, or those characters may be passed back to the caller and the lookup successful.
 *   @param path A C-string path.
 *   @param plane The plane to lookup up the path, or zero, in which case the path must begin with the plane name.
 *   @param residualPath If the path may contain residual characters after the last path component, the residual will be copied back to the caller's residualPath buffer. If there are residual characters and no residual buffer is specified, fromPath will fail.
 *   @param residualLength An in/out parameter - the caller sets the length of the residual buffer available, and fromPath returns the total length of the residual path copied to the buffer. If there is no residualBuffer (residualPath = 0) then residualLength may be zero also.
 *   @param fromEntry The lookup will proceed rooted at this entry if non-zero, otherwise it proceeds from the root of the plane.
 *   @result A retained registry entry is returned on success, or zero on failure. The caller should release the entry. */

	static IORegistryEntry * fromPath(  const char * path,
	    const IORegistryPlane * plane = NULL,
	    char * residualPath = NULL,
	    int * residualLength = NULL,
	    IORegistryEntry * fromEntry = NULL );

/*! @function fromPath
 *   @abstract Looks up a registry entry by relative path.
 *   @discussion This function looks up a entry below the called entry by a relative path. It is just a convenience that calls IORegistryEntry::fromPath with this as the fromEntry parameter.
 *   @param path See IORegistryEntry::fromPath.
 *   @param plane See IORegistryEntry::fromPath.
 *   @param residualPath See IORegistryEntry::fromPath.
 *   @param residualLength See IORegistryEntry::fromPath.
 *   @result See IORegistryEntry::fromPath. */

	virtual IORegistryEntry * childFromPath( const char * path,
	    const IORegistryPlane * plane = NULL,
	    char * residualPath = NULL,
	    int * residualLength = NULL );

/*! @function dealiasPath
 *   @abstract Strips any aliases from the head of path and returns the full path.
 *   @discussion If the path specified begins with an alias found in the /aliases entry, the value of the alias is returned, and a pointer into the passed in path after the alias is passed back to the caller. If an alias is not found, zero is returned and the path parameter is unchanged.
 *   @param opath An in/out paramter - the caller passes in a pointer to a C-string pointer to a path. If an alias is found, dealiasPath returns a pointer into the path just beyond the end of the alias.
 *   @param plane A plane object must be specified.
 *   @result A C-string pointer to the value of the alias if one is found, or zero if not. */

	static const char * dealiasPath( const char ** opath,
	    const IORegistryPlane * plane );

/*! @function makePlane
 *   @abstract Constructs an IORegistryPlane object.
 *   @discussion Most planes in IOKit are created by the OS, although other planes may be created.
 *   @param name A C-string name for the new plane, to be copied.
 *   @result A new instance of an IORegistryPlane, or zero on failure. */

	static const IORegistryPlane * makePlane( const char * name );

/*!    @abstract Returns an ID for the registry entry that is global to all tasks.
 *   @discussion The entry ID returned by getRegistryEntryID can be used to identify a registry entry across all tasks. A registry entry may be looked up by its entry ID by creating a matching dictionary with IORegistryEntryIDMatching() in user space, or <code>IOService::registryEntryIDMatching()</code> in the kernel, to be used with the IOKit matching functions. The ID is valid only until the machine reboots.
 *   @result An ID for the registry entry, assigned when the entry is first attached in the registry. */

	uint64_t getRegistryEntryID( void );

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* * * * * * * * * * * * internals * * * * * * * * * * * */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * */

	virtual bool init( IORegistryEntry * from,
	    const IORegistryPlane * inPlane );

#ifdef XNU_KERNEL_PRIVATE
public:
#else
private:
#endif
	static LIBKERN_RETURNS_NOT_RETAINED IORegistryEntry * initialize( void );

#ifdef XNU_KERNEL_PRIVATE
	SInt32 getRegistryEntryGenerationCount( void ) const;
	void setName(const OSString * name,
	    const IORegistryPlane * plane = NULL);
#endif

private:
	inline bool arrayMember( OSArray * set,
	    const IORegistryEntry * member,
	    unsigned int * index = NULL ) const;

	bool makeLink( IORegistryEntry * to,
	    unsigned int relation,
	    const IORegistryPlane * plane ) const;
	void breakLink( IORegistryEntry * to,
	    unsigned int relation,
	    const IORegistryPlane * plane ) const;

	APPLE_KEXT_COMPATIBILITY_VIRTUAL
	OSArray * getParentSetReference( const IORegistryPlane * plane )
	const;

	APPLE_KEXT_COMPATIBILITY_VIRTUAL
	OSArray * getChildSetReference( const IORegistryPlane * plane )
	const;

	APPLE_KEXT_COMPATIBILITY_VIRTUAL
	IORegistryEntry * getChildFromComponent( const char ** path,
	    const IORegistryPlane * plane );

	APPLE_KEXT_COMPATIBILITY_VIRTUAL
	LIBKERN_RETURNS_NOT_RETAINED const OSSymbol * hasAlias(
		const IORegistryPlane * plane,
		char * opath = NULL, int * length = NULL ) const;

	APPLE_KEXT_COMPATIBILITY_VIRTUAL
	const char * matchPathLocation( const char * cmp,
	    const IORegistryPlane * plane );
};

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*! @class IORegistryIterator : public OSIterator
 *   @abstract An iterator over the registry.
 *   @discussion An iterator that can traverse the children or parents of a registry entry in a plane, and recurse. Access to the registry is protected against multiple threads, but an IORegistryIterator instance is for use by one thread only. */

class IORegistryIterator : public OSIterator
{
	OSDeclareAbstractStructors(IORegistryIterator);

private:
	struct IORegCursor {
		IORegCursor          *  next;
		IORegistryEntry      *  current;
		OSIterator           *  iter;
	};
	IORegCursor                 start;
	IORegCursor           *     where;
	IORegistryEntry       *     root;
	OSOrderedSet          *     done;
	const IORegistryPlane *     plane;
	IOOptionBits                options;

	virtual void free( void ) APPLE_KEXT_OVERRIDE;

public:
/*! @function iterateOver
 *   @abstract Create an iterator rooted at a given registry entry.
 *   @discussion This method creates an IORegistryIterator that is set up with options to iterate children or parents of a root entry, and to recurse automatically into entries as they are returned, or only when instructed. The iterator object keeps track of entries that have been recursed into previously to avoid loops.
 *   @param start The root entry to begin the iteration at.
 *   @param plane A plane object must be specified.
 *   @param options kIORegistryIterateRecursively may be set to recurse automatically into each entry as it is returned. This option affects the behaviour of the getNextObject method, which is defined in the OSIterator superclass. Other methods will override this behaviour. kIORegistryIterateParents may be set to iterate the parents of each entry, by default the children are iterated.
 *   @result A created IORegistryIterator instance, to be released by the caller when it has finished with it. */

	static IORegistryIterator * iterateOver( IORegistryEntry * start,
	    const IORegistryPlane * plane,
	    IOOptionBits options = 0 );

/*! @function iterateOver
 *   @abstract Create an iterator rooted at the registry root.
 *   @discussion This method creates an IORegistryIterator that is set up with options to iterate children of the registry root entry, and to recurse automatically into entries as they are returned, or only when instructed. The iterator object keeps track of entries that have been recursed into previously to avoid loops.
 *   @param plane A plane object must be specified.
 *   @param options kIORegistryIterateRecursively may be set to recurse automatically into each entry as it is returned. This option affects the behaviour of the getNextObject method, which is defined in the OSIterator superclass. Other methods will override this behaviour. kIORegistryIterateParents may be set to iterate the parents of each entry, by default the children are iterated.
 *   @result A created IORegistryIterator instance, to be released by the caller when it has finished with it. */

	static IORegistryIterator * iterateOver( const IORegistryPlane * plane,
	    IOOptionBits options = 0 );

/*! @function getNextObject
 *   @abstract Return the next object in the registry iteration.
 *   @discussion This method calls either getNextObjectFlat or getNextObjectRecursive depending on the options the iterator was created with. This implements the OSIterator defined getNextObject method. The object returned is retained while the iterator is pointing at it (its the current entry), or recursing into it. The caller should not release it.
 *   @result The next registry entry in the iteration (the current entry), or zero if the iteration has finished at this level of recursion. The entry returned is retained while the iterator is pointing at it (its the current entry), or recursing into it. The caller should not release it. */

	virtual IORegistryEntry * getNextObject( void ) APPLE_KEXT_OVERRIDE;

/*! @function getNextObjectFlat
 *   @abstract Return the next object in the registry iteration, ignoring the kIORegistryIterateRecursively option.
 *   @discussion This method returns the next child, or parent if the kIORegistryIterateParents option was used to create the iterator, of the current root entry. The object returned is retained while the iterator is pointing at it (its the current entry), or recursing into it. The caller should not release it.
 *   @result The next registry entry in the iteration (the current entry), or zero if the iteration has finished at this level of recursion, or the iteration is invalid (see isValid). The entry returned is retained while the iterator is pointing at it (its the current entry), or recursing into it. The caller should not release it. */

	virtual IORegistryEntry * getNextObjectFlat( void );

/*! @function getNextObjectRecursive
 *   @abstract Return the next object in the registry iteration, and enter it.
 *   @discussion If the iterator has a current entry, and the iterator has not already entered previously, enterEntry is called to recurse into it, ie. make it the new root, and the next child, or parent if the kIORegistryIterateParents option was used to create the iterator, at this new level of recursion is returned. If there is no current entry at this level of recursion, exitEntry is called and the process repeats, until the iteration returns to the entry the iterator was created with and zero is returned. The object returned is retained while the iterator is pointing at it (its the current entry), or recursing into it. The caller should not release it.
 *   @result The next registry entry in the iteration (the current entry), or zero if its finished, or the iteration is invalid (see isValid). The entry returned is retained while the iterator is pointing at it (its the current entry), or recursing into it. The caller should not release it. */

	virtual IORegistryEntry * getNextObjectRecursive( void );

/*! @function getCurrentEntry
 *   @abstract Return the current entry in the registry iteration.
 *   @discussion This method returns the current entry, last returned by getNextObject et al. The object returned is retained while the iterator is pointing at it (its the current entry), or recursing into it. The caller should not release it. If the iteration is no longer valid (see isValid), the current entry is zero.
 *   @result The current registry entry in the iteration, or zero if the last iteration returned zero, or the iteration is invalid (see isValid). The entry returned is retained while the iterator is pointing at it (its the current entry), or recursing into it. The caller should not release it. */

	virtual IORegistryEntry * getCurrentEntry( void );

/*! @function enterEntry
 *   @abstract Recurse into the current entry in the registry iteration.
 *   @discussion This method makes the current entry, ie. the last entry returned by getNextObject et al., the root in a new level of recursion. */

	virtual void enterEntry( void );

/*! @function enterEntry
 *   @abstract Recurse into the current entry in the registry iteration.
 *   @discussion This method recurses into an entry as with enterEntry, but also switches from the current plane to a new one set by the caller.
 *   @param plane The new plane to switch into. */

	virtual void enterEntry( const IORegistryPlane * plane );

/*! @function exitEntry
 *   @abstract Exits a level of recursion, restoring the current entry.
 *   @discussion This method undoes an enterEntry, restoring the current entry. If there are no more levels of recursion to exit false is returned, otherwise true is returned.
 *   @result true if a level of recursion was undone, false if no recursive levels are left in the iteration. */

	virtual bool exitEntry( void );

/*! @function reset
 *   @abstract Exits all levels of recursion, restoring the iterator to its state at creation.
 *   @discussion This method exits all levels of recursion, and restores the iterator to its state at creation. */

	virtual void reset( void ) APPLE_KEXT_OVERRIDE;

/*! @function isValid
 *   @abstract Checks that no registry changes have invalidated the iteration.
 *   @discussion If a registry iteration is invalidated by changes to the registry, it will be made invalid, the currentEntry will be considered zero, and further calls to getNextObject et al. will return zero. The iterator should be reset to restart the iteration when this happens.
 *   @result false if the iterator has been invalidated by changes to the registry, true otherwise. */

	virtual bool isValid( void ) APPLE_KEXT_OVERRIDE;

/*! @function iterateAll
 *   @abstract Iterates all entries (with getNextObject) and returns a set of all returned entries.
 *   @discussion This method will reset, then iterate all entries in the iteration (with getNextObject).
 *   @result A set of entries returned by the iteration. The caller should release the set when it has finished with it. Zero is returned on a resource failure. */

	virtual OSOrderedSet * iterateAll( void );
};

#endif /* _IOKIT_IOREGISTRYENTRY_H */
