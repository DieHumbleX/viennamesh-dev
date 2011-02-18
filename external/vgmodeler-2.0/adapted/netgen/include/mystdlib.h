#ifndef FILE_MYSTDLIB
#define FILE_MYSTDLIB


#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>

#ifdef OLDCINCLUDE

// e.g., CC compiler on SGI
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <malloc.h>
#include <ctype.h>
#include <time.h>

#else
 
// new standard
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <cctype>
#include <ctime>
#endif



#include <new>
#include <string>
#include <typeinfo>

#ifdef PARALLEL
#undef SEEK_SET
#undef SEEK_CUR
#undef SEEK_END
#include <mpi.h>
#endif

#ifdef METIS
namespace metis { extern "C" {
#include <metis.h>
} }


#endif



#ifndef NO_PARALLEL_THREADS
#ifndef WIN32
#include <pthread.h>
#endif
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef MAXDOUBLE
#define MAXDOUBLE 1.797693E+308
#endif

#ifndef MINDOUBLE
#define MINDOUBLE 2.225074E-308
#endif


/*** Windows headers ***/
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#ifndef NO_PARALLEL_THREADS
#include <afxwin.h>
#include <afxmt.h>
#endif
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#include <winnt.h>
#endif /* WIN32 */


/*
extern void* operator new(std::size_t) throw (std::bad_alloc);
extern void* operator new[](std::size_t) throw (std::bad_alloc);
extern void operator delete(void*) throw();
extern void operator delete[](void*) throw();
*/


extern int mem_alloc;
extern int mem_total_alloc;
extern int mem_max_alloc;
extern int mem_total_alloc_array;
extern int mem_total_alloc_table;


using namespace std;

#endif
