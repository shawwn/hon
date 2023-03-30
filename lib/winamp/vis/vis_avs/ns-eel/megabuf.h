/*
  Nullsoft Expression Evaluator Library (NS-EEL)
  Copyright (C) 1999-2003 Nullsoft, Inc.
  
  ns-eel.h: main application interface header

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#ifndef _MEGABUF_H_
#define _MEGABUF_H_

#ifdef __cplusplus
extern "C" {
#endif

#define MEGABUF_BLOCKS 64
#define MEGABUF_ITEMSPERBLOCK 16384

void _asm_megabuf(void);
void _asm_megabuf_end(void);
void megabuf_ppproc(void *data, int data_size, void **userfunc_data);
void megabuf_cleanup(NSEEL_VMCTX);

#ifdef __cplusplus
}
#endif


#endif