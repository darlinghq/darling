/*
     File: CAThreadSafeList.h
 Abstract: Part of CoreAudio Utility Classes
  Version: 1.1
 
 Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
 Inc. ("Apple") in consideration of your agreement to the following
 terms, and your use, installation, modification or redistribution of
 this Apple software constitutes acceptance of these terms.  If you do
 not agree with these terms, please do not use, install, modify or
 redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and
 subject to these terms, Apple grants you a personal, non-exclusive
 license, under Apple's copyrights in this original Apple software (the
 "Apple Software"), to use, reproduce, modify and redistribute the Apple
 Software, with or without modifications, in source and/or binary forms;
 provided that if you redistribute the Apple Software in its entirety and
 without modifications, you must retain this notice and the following
 text and disclaimers in all such redistributions of the Apple Software.
 Neither the name, trademarks, service marks or logos of Apple Inc. may
 be used to endorse or promote products derived from the Apple Software
 without specific prior written permission from Apple.  Except as
 expressly stated in this notice, no other rights or licenses, express or
 implied, are granted by Apple herein, including but not limited to any
 patent rights that may be infringed by your derivative works or by other
 works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
 MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
 THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
 FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
 OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
 OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
 MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
 AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
 STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
 
 Copyright (C) 2014 Apple Inc. All Rights Reserved.
 
*/
#ifndef __CAThreadSafeList_h__
#define __CAThreadSafeList_h__

#include "CAAtomicStack.h"

//  linked list of T's
//	T must define operator ==
template <class T>
class TThreadSafeList {
private:
	enum EEventType { kAdd, kRemove, kClear };
	class Node {
	public:
		Node *		mNext;
		EEventType	mEventType;
		T			mObject;
		
		Node *&	next() { return mNext; }
	};

public:
	class iterator {
	public:
		iterator() { }
		iterator(Node *n) : mNode(n) { }
		
		bool operator == (const iterator &other) const { return this->mNode == other.mNode; }
		bool operator != (const iterator &other) const { return this->mNode != other.mNode; }
		
		T & operator * () const { return mNode->mObject; }
		
		iterator & operator ++ () { mNode = mNode->next(); return *this; }	// preincrement
		iterator operator ++ (int) { iterator tmp = *this; mNode = mNode->next(); return tmp; } // postincrement
		
	private:
		Node *		mNode;
	};
	
	TThreadSafeList() { }
	~TThreadSafeList()
	{
		mActiveList.free_all();
		mPendingList.free_all();
		mFreeList.free_all();
	}
	
	// These may be called on any thread
	
	void	deferred_add(const T &obj)	// can be called on any thread
	{
		Node *node = AllocNode();
		node->mEventType = kAdd;
		node->mObject = obj;
		mPendingList.push_atomic(node);
		//mPendingList.dump("pending after add");
	}
	
	void	deferred_remove(const T &obj)	// can be called on any thread
	{
		Node *node = AllocNode();
		node->mEventType = kRemove;
		node->mObject = obj;
		mPendingList.push_atomic(node);
		//mPendingList.dump("pending after remove");
	}
	
	void	deferred_clear()					// can be called on any thread
	{
		Node *node = AllocNode();
		node->mEventType = kClear;
		mPendingList.push_atomic(node);
	}
	
	// These must be called from only one thread
	
	void	update()		// must only be called from one thread
	{
		NodeStack reversed;
		Node *event, *node, *next;
		bool workDone = false;
		
		// reverse the events so they are in order
		event = mPendingList.pop_all();
		while (event != NULL) {
			next = event->mNext;
			reversed.push_NA(event);
			event = next;
			workDone = true;
		}
		if (workDone) {
			//reversed.dump("pending popped");
			//mActiveList.dump("active before update");
			
			// now process them
			while ((event = reversed.pop_NA()) != NULL) {
				switch (event->mEventType) {
				case kAdd:
					{
						Node **pnode;
						bool needToInsert = true;
						for (pnode = mActiveList.phead(); *pnode != NULL; pnode = &node->mNext) {
							node = *pnode;
							if (node->mObject == event->mObject) {
								//printf("already active!!!\n");
								FreeNode(event);
								needToInsert = false;
								break;
							}
						}
						if (needToInsert) {
							// link the new event in at the end of the active list
							*pnode = event;
							event->mNext = NULL;
						}
					}
					break;
				case kRemove:
					// find matching node in the active list, remove it
					for (Node **pnode = mActiveList.phead(); *pnode != NULL; ) {
						node = *pnode;
						if (node->mObject == event->mObject) {
							*pnode = node->mNext;	// remove from linked list
							FreeNode(node);
							break;
						}
						pnode = &node->mNext;
					}
					// dispose the request node
					FreeNode(event);
					break;
				case kClear:
					for (node = mActiveList.head(); node != NULL; ) {
						next = node->mNext;
						FreeNode(node);
						node = next;
					}
					FreeNode(event);
					break;
				default:
					//printf("invalid node type %d!\n", event->mEventType);
					break;
				}
			}
			//mActiveList.dump("active after update");
		}
	}
	
	iterator begin() const {
		//mActiveList.dump("active at begin");
		return iterator(mActiveList.head());
	}
	iterator end() const { return iterator(NULL); }

	
private:
	Node *	AllocNode()
	{
		Node *node = mFreeList.pop_atomic();
		if (node == NULL)
			node = (Node *)CA_malloc(sizeof(Node));
		return node;
	}
	
	void	FreeNode(Node *node)
	{
		mFreeList.push_atomic(node);
	}

private:
	class NodeStack : public TAtomicStack<Node> {
	public:
		void free_all() {
			Node *node;
			while ((node = this->pop_NA()) != NULL)
				free(node);
		}
		
		Node **	phead() { return &this->mHead; }
		Node *	head() const { return this->mHead; }
	};

	NodeStack	mActiveList;	// what's actually in the container - only accessed on one thread
	NodeStack	mPendingList;	// add or remove requests - threadsafe
	NodeStack	mFreeList;		// free nodes for reuse - threadsafe
};

#endif // __CAThreadSafeList_h__
