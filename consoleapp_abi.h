//////////////////////////////////////////////////////////////////////////////
// Tycho Game Library
// Copyright (C) 2014 Martin Slater
// Created : Tuesday, 10 June 2014 06:57:53 PM
//////////////////////////////////////////////////////////////////////////////
#if _MSC_VER > 1000
#pragma once
#endif  // _MSC_VER

#ifndef _TEMPLATE_ABI_H_51DE81E8_5144_4B63_BC70_C6AB8110F146
#define _TEMPLATE_ABI_H_51DE81E8_5144_4B63_BC70_C6AB8110F146

//////////////////////////////////////////////////////////////////////////////
// INCLUDES
//////////////////////////////////////////////////////////////////////////////
#include "core/core_abi.h"

#if TYCHO_PC

// DLL interface
#ifdef TYCHO_CONSOLEAPP_EXPORTS
#define TYCHO_CONSOLEAPP_ABI __declspec(dllexport)
#else
#define TYCHO_CONSOLEAPP_ABI __declspec(dllimport)
#endif 

// disable a few warnings globally. should move these into specific cpp's to avoid polluting
// user header files
#pragma warning(disable : 4251) // class needs to have dll-interface to be used by clients of class ''
#pragma warning(disable : 4355) // 'this' : used in base member initializer list

#else // TYCHO_PC

#define TYCHO_CONSOLEAPP_ABI

#endif // TYCHO_GC

#ifdef __cplusplus
#include "core/memory/new.h"
#include "core/memory.h"
#include "core/debug/assert.h"
#endif 

#endif // _TEMPLATE_ABI_H_51DE81E8_5144_4B63_BC70_C6AB8110F146
