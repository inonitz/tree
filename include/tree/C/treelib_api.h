#ifndef _TREELIB_API_H_
#define _TREELIB_API_H_


#ifdef TREELIB_STATIC_DEFINE /* Define if compiling as a static library (-DTREELIB_STATIC_DEFINE) */
#	define TREELIB_API
#	define TREELIB_NO_EXPORT
#else
#	ifndef TREELIB_API
#		ifdef TREELIB_EXPORTS /* We are building this library */
#			if defined _WIN32 || defined _WIN64 || defined __CYGWIN__ || defined __MINGW64__
#				if defined __GNUC__ || defined __clang__
#					define TREELIB_API __attribute__ ((dllexport))
#				else
#					define TREELIB_API __declspec(dllexport)
#				endif
#			else 
#				if (defined __GNUC__ && __GNUC__ >= 4) || defined __clang__
#					define TREELIB_API __attribute__ ((visibility ("default")))
#				endif
#			endif
#    	else /* We are using this library */
#			if defined _WIN32 || defined _WIN64 || defined __CYGWIN__ || defined __MINGW64__
#				if defined __GNUC__ || defined __clang__
#					define TREELIB_API __attribute__ ((dllimport))
#				else
#					define TREELIB_API __declspec(dllimport)
#				endif
#			else 
#				if defined __GNUC__ && __GNUC__ >= 4
#					define TREELIB_API
#				endif
#			endif
#       endif
#	else /* Should Only reach here for non-*nix, un-supported platforms */
#       warning "Platform Unsupported - Either Not a derivative of Unix // Not Windows"
#		define TREELIB_API
#   endif
#	ifndef TREELIB_NO_EXPORT
#       if defined __GNUC__ && __GNUC__ >= 4 /* Symbols exported by default on *nix systems */
#           define TREELIB_NO_EXPORT __attribute__((visibility ("hidden")))
#       else /* (DLL) Symbols on platforms like windows must be exported manually [__declspec(dllexport)] */
#		    define TREELIB_NO_EXPORT 
#       endif
#	endif
#endif


#ifndef TREELIB_DEPRECATED
#   if defined(__cplusplus)
#       if __cplusplus >= 201402L /* [[deprecated]] Supported since C++14 */
#           define TREELIB_DEPRECATED [[deprecated]]
#           define TREELIB_DEPRECATED_MSG(MSG) [[deprecated(MSG)]]
#       endif
#   else
#       if defined _WIN32 || defined _WIN64
#           if defined __GNUC__ || defined __clang__ /* Cygwin, MinGW32/64 */
#               define TREELIB_DEPRECATED          __attribute__((deprecated))
#               define TREELIB_DEPRECATED_MSG(MSG) __attribute__((deprecated(MSG)))
#           else
#               define TREELIB_DEPRECATED          __declspec(deprecated)
#               define TREELIB_DEPRECATED_MSG(MSG) __declspec(deprecated(MSG))
#           endif
#       elif defined __GNUC__ || defined __clang__
#           define TREELIB_DEPRECATED __attribute__((deprecated))
#           define TREELIB_DEPRECATED_MSG(MSG) __attribute__((deprecated(MSG)))
#       else /* Should Only reach here for non-*nix, un-supported platforms */
#           define TREELIB_DEPRECATED
#           define TREELIB_DEPRECATED_MSG(MSG)
#       endif
#   endif
#endif


#ifndef TREELIB_DEPRECATED_EXPORT
#  define TREELIB_DEPRECATED_EXPORT TREELIB_API TREELIB_DEPRECATED
#endif


#ifndef TREELIB_DEPRECATED_NO_EXPORT
#  define TREELIB_DEPRECATED_NO_EXPORT TREELIB_NO_EXPORT TREELIB_DEPRECATED
#endif


/* NOLINTNEXTLINE(readability-avoid-unconditional-preprocessor-if) */
#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef TREELIB_NO_DEPRECATED
#    define TREELIB_NO_DEPRECATED
#  endif
#endif

#endif /* _TREELIB_API_H_ */