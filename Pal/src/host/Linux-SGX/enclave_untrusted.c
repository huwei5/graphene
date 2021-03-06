/* -*- mode:c; c-file-style:"k&r"; c-basic-offset: 4; tab-width:4; indent-tabs-mode:nil; mode:auto-fill; fill-column:78; -*- */
/* vim: set ts=4 sw=4 et tw=78 fo=cqt wm=0: */

/* Copyright (C) 2014 OSCAR lab, Stony Brook University
   This file is part of Graphene Library OS.

   Graphene Library OS is free software: you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation, either version 3 of the
   License, or (at your option) any later version.

   Graphene Library OS is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#include <pal_internal.h>
#include <pal_security.h>
#include <pal_error.h>
#include <linux_list.h>
#include <api.h>

#include "enclave_ocalls.h"

#define allocator pal_sec.untrusted_allocator
#define untrusted_slabmgr (allocator.slabmgr)
#define system_lock()   _DkMutexLock(allocator.lock)
#define system_unlock() _DkMutexUnlock(allocator.lock)

#define PAGE_SIZE (allocator.alignment)

static inline void * __malloc (int size)
{
    void * addr = NULL;

    ocall_alloc_untrusted(size, &addr);
    return addr;
}

#define system_malloc(size) __malloc(size)

static inline void __free (void * addr, int size)
{
    ocall_unmap_untrusted(addr, size);
}

#define system_free(addr, size) __free(addr, size)

#include "slabmgr.h"

void * malloc_untrusted (int size)
{
    void * ptr = slab_alloc(untrusted_slabmgr, size);

    /* the slab manger will always remain at least one byte of padding,
       so we can feel free to assign an offset at the byte prior to
       the pointer */
    if (ptr)
        *(((unsigned char *) ptr) - 1) = 0;

    return ptr;
}

void free_untrusted (void * ptr)
{
    ptr -= *(((unsigned char *) ptr) - 1);
    slab_free(untrusted_slabmgr, ptr);
}
