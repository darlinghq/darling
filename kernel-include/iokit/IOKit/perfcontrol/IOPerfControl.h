/*
 * Copyright (c) 2017 Apple Inc. All rights reserved.
 */

#pragma once

#ifdef KERNEL_PRIVATE
#ifdef __cplusplus

#include <IOKit/IOService.h>
#include <stdatomic.h>
#include <kern/bits.h>

struct thread_group;

enum{
	kIOPerfControlClientWorkUntracked = 0,
};

/*!
 * @class IOPerfControlClient : public OSObject
 * @abstract Class which implements an interface allowing device drivers to participate in performance control.
 * @discussion TODO
 */
class IOPerfControlClient final : public OSObject
{
	OSDeclareDefaultStructors(IOPerfControlClient);

protected:
	virtual bool init(IOService *driver, uint64_t maxWorkCapacity);

public:
/*!
 * @function copyClient
 * @abstract Return a retained reference to a client object, to be released by the driver. It may be
 * shared with other drivers in the system.
 * @param driver The device driver that will be using this interface.
 * @param maxWorkCapacity The maximum number of concurrent work items supported by the device driver.
 * @returns An instance of IOPerfControlClient.
 */
	static IOPerfControlClient *copyClient(IOService *driver, uint64_t maxWorkCapacity);

/*!
 * @function registerDevice
 * @abstract Inform the system that work will be dispatched to a device in the future.
 * @discussion The system will do some one-time setup work associated with the device, and may block the
 * current thread during the setup. Devices should not be passed to work workSubmit, workSubmitAndBegin,
 * workBegin, or workEnd until they have been successfully registered. The unregistration process happens
 * automatically when the device object is deallocated.
 * @param device The device object. Some platforms require device to be a specific subclass of IOService.
 * @returns kIOReturnSuccess or an IOReturn error code
 */
	virtual IOReturn registerDevice(IOService *driver, IOService *device);

/*!
 * @function unregisterDevice
 * @abstract Inform the system that work will be no longer be dispatched to a device in the future.
 * @discussion This call is optional as the unregistration process happens automatically when the device
 * object is deallocated. This call may block the current thread and/or acquire locks. It should not be
 * called until after all submitted work has been ended using workEnd.
 * @param device The device object. Some platforms require device to be a specific subclass of IOService.
 */
	virtual void unregisterDevice(IOService *driver, IOService *device);

/*!
 * @struct WorkSubmitArgs
 * @discussion Drivers may submit additional device-specific arguments related to the submission of a work item
 * by passing a struct with WorkSubmitArgs as its first member. Note: Drivers are responsible for publishing
 * a header file describing these arguments.
 */
	struct WorkSubmitArgs {
		uint32_t version;
		uint32_t size;
		uint64_t submit_time;
		uint64_t reserved[4];
		void *driver_data;
	};

/*!
 * @function workSubmit
 * @abstract Tell the performance controller that work was submitted.
 * @param device The device that will execute the work. Some platforms require device to be a
 * specific subclass of IOService.
 * @param args Optional device-specific arguments related to the submission of this work item.
 * @returns A token representing this work item, which must be passed to workEnd when the work is finished
 * unless the token equals kIOPerfControlClientWorkUntracked. Failure to do this will result in memory leaks
 * and a degradation of system performance.
 */
	virtual uint64_t workSubmit(IOService *device, WorkSubmitArgs *args = nullptr);

/*!
 * @struct WorkBeginArgs
 * @discussion Drivers may submit additional device-specific arguments related to the start of a work item
 * by passing a struct with WorkBeginArgs as its first member. Note: Drivers are responsible for publishing
 * a header file describing these arguments.
 */
	struct WorkBeginArgs {
		uint32_t version;
		uint32_t size;
		uint64_t begin_time;
		uint64_t reserved[4];
		void *driver_data;
	};

/*!
 * @function workSubmitAndBegin
 * @abstract Tell the performance controller that work was submitted and immediately began executing.
 * @param device The device that is executing the work. Some platforms require device to be a
 * specific subclass of IOService.
 * @param submitArgs Optional device-specific arguments related to the submission of this work item.
 * @param beginArgs Optional device-specific arguments related to the start of this work item.
 * @returns A token representing this work item, which must be passed to workEnd when the work is finished
 * unless the token equals kIOPerfControlClientWorkUntracked. Failure to do this will result in memory leaks
 * and a degradation of system performance.
 */
	virtual uint64_t workSubmitAndBegin(IOService *device, WorkSubmitArgs *submitArgs = nullptr,
	    WorkBeginArgs *beginArgs = nullptr);

/*!
 * @function workBegin
 * @abstract Tell the performance controller that previously submitted work began executing.
 * @param device The device that is executing the work. Some platforms require device to be a
 * specific subclass of IOService.
 * @param args Optional device-specific arguments related to the start of this work item.
 */
	virtual void workBegin(IOService *device, uint64_t token, WorkBeginArgs *args = nullptr);

/*!
 * @struct WorkEndArgs
 * @discussion Drivers may submit additional device-specific arguments related to the end of a work item
 * by passing a struct with WorkEndArgs as its first member. Note: Drivers are responsible for publishing
 * a header file describing these arguments.
 */
	struct WorkEndArgs {
		uint32_t version;
		uint32_t size;
		uint64_t end_time;
		uint64_t reserved[4];
		void *driver_data;
	};

/*!
 * @function workEnd
 * @abstract Tell the performance controller that previously started work finished executing.
 * @param device The device that executed the work. Some platforms require device to be a
 * specific subclass of IOService.
 * @param args Optional device-specific arguments related to the end of this work item.
 * @param done Optional Set to false if the work has not yet completed. Drivers are then responsible for
 * calling workBegin when the work resumes and workEnd with done set to True when it has completed.
 */
	virtual void workEnd(IOService *device, uint64_t token, WorkEndArgs *args = nullptr, bool done = true);

/*!
 * @struct PerfControllerInterface
 * @discussion Function pointers necessary to register a performance controller. Not for general driver use.
 */
	struct PerfControllerInterface {
		struct WorkState {
			uint64_t thread_group_id;
			void *thread_group_data;
			void *work_data;
			uint32_t work_data_size;
		};

		using RegisterDeviceFunction = IOReturn (*)(IOService *);
		using WorkCanSubmitFunction = bool (*)(IOService *, WorkState *, WorkSubmitArgs *);
		using WorkSubmitFunction = void (*)(IOService *, uint64_t, WorkState *, WorkSubmitArgs *);
		using WorkBeginFunction = void (*)(IOService *, uint64_t, WorkState *, WorkBeginArgs *);
		using WorkEndFunction = void (*)(IOService *, uint64_t, WorkState *, WorkEndArgs *, bool);

		uint64_t version;
		RegisterDeviceFunction registerDevice;
		RegisterDeviceFunction unregisterDevice;
		WorkCanSubmitFunction workCanSubmit;
		WorkSubmitFunction workSubmit;
		WorkBeginFunction workBegin;
		WorkEndFunction workEnd;
	};

	struct IOPerfControlClientShared {
		atomic_uint_fast8_t maxDriverIndex;
		PerfControllerInterface interface;
		IOLock *interfaceLock;
		OSSet *deviceRegistrationList;
	};

/*!
 * @function registerPerformanceController
 * @abstract Register a performance controller to receive callbacks. Not for general driver use.
 * @param interface Struct containing callback functions implemented by the performance controller.
 * @returns kIOReturnSuccess or kIOReturnError if the interface was already registered.
 */
	virtual IOReturn registerPerformanceController(PerfControllerInterface interface);

private:
	struct WorkTableEntry {
		struct thread_group *thread_group;
		bool started;
		uint8_t perfcontrol_data[32];
	};

	static constexpr size_t kMaxWorkTableNumEntries = 1024;
	static constexpr size_t kWorkTableIndexBits = 24;
	static constexpr size_t kWorkTableMaxSize = (1 << kWorkTableIndexBits) - 1; // - 1 since
	// kIOPerfControlClientWorkUntracked takes number 0
	static constexpr size_t kWorkTableIndexMask = mask(kWorkTableIndexBits);

	uint64_t allocateToken(thread_group *thread_group);
	void deallocateToken(uint64_t token);
	bool getEntryForToken(uint64_t token, WorkTableEntry &entry);
	void markEntryStarted(uint64_t token, bool started);
	inline uint64_t tokenToGlobalUniqueToken(uint64_t token);

	uint8_t driverIndex;
	IOPerfControlClientShared *shared;
	WorkTableEntry *workTable;
	size_t workTableLength;
	size_t workTableNextIndex;
	IOSimpleLock *workTableLock;
};

#endif /* __cplusplus */
#endif /* KERNEL_PRIVATE */
