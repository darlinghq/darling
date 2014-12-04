/*
 *	abi.h
 *
 *		by Ian Ollmann
 *
 *	Copyright (c) 2007, Apple Inc. All rights reserved.
 *
 *	Assembly code prefix header for use in armLibm
 */


#define	ENTRY(name)									\
  .globl _##name;									\
  .align 2;											\
  _##name##:								      

#define	PRIVATE_ENTRY(name)							\
  .private_extern _##name;							\
  .align 2;											\
  _##name##:								      
