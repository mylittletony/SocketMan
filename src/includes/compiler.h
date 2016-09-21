#ifndef __compiler_h__
#define __compiler_h__

#ifdef __GNUC__
#define UNUSED(__decl__) __decl__ __attribute((unused))
#else
#define UNUSED(__decl__) __decl__
#endif

#endif

