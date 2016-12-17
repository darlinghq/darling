/*
 * Darling - libquarantine
 * Copyright (c) 2016 Lubos Dolezel, All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3.0 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.
 */

#ifndef _QUARANTINE_H_
#define _QUARANTINE_H_

#include <stdint.h>
#include <stddef.h>

#define QTN_NOT_QUARANTINED (-1)

#define QTN_FLAG_SANDBOX 1
#define QTN_FLAG_HARD 2

typedef struct _qtn_file_s *qtn_file_t;

extern qtn_file_t _qtn_file_alloc();
extern void _qtn_file_free(qtn_file_t file);

extern qtn_file_t _qtn_file_clone(qtn_file_t file);

extern int _qtn_file_init_with_fd(qtn_file_t file, int fd);
extern int _qtn_file_apply_to_fd(qtn_file_t file, int fd);
extern int _qtn_file_apply_to_path(qtn_file_t file, const char* path);

extern int _qtn_file_init_with_path(qtn_file_t file, const char *path);

extern int _qtn_file_init_with_data(qtn_file_t file, const void *, size_t);
extern int _qtn_file_to_data(qtn_file_t file, void *, size_t*);
extern int _qtn_file_set_flags(qtn_file_t file, uint32_t flags);
extern uint32_t _qtn_file_get_flags(qtn_file_t file);

extern const char *_qtn_error(int err);

extern const char *_qtn_xattr_name;

extern int __esp_enabled();
extern int __esp_check_ns(const char *a, void *b);
extern int __esp_notify_ns(const char *a, void *b);

#define QTN_SERIALIZED_DATA_MAX 4096

#define qtn_file_alloc _qtn_file_alloc
#define qtn_file_free _qtn_file_free

#define qtn_file_clone _qtn_file_clone
#define qtn_file_set_flags _qtn_file_set_flags
#define qtn_file_get_flags _qtn_file_get_flags

#define qtn_file_init_with_fd _qtn_file_init_with_fd
#define qtn_file_apply_to_fd _qtn_file_apply_to_fd
#define qtn_file_apply_to_path _qtn_file_apply_to_path

#define qtn_file_init_with_path _qtn_file_init_with_path

#define qtn_file_init_with_data _qtn_file_init_with_data
#define qtn_file_to_data _qtn_file_to_data

#define qtn_error _qtn_error

#define qtn_xattr_name _qtn_xattr_name

#endif

