#pragma once

#define DISALLOW_COPY(Type) Type(const Type&) = delete;
#define DISALLOW_ASSIGN(Type) Type& operator=(const Type&) = delete;

#define DISALLOW_COPY_ASSIGN(Type) \
    DISALLOW_COPY(Type)            \
    DISALLOW_ASSIGN(Type)