AC_PREREQ([2.69])
AC_INIT([partialZipBrowser], [1.0], [tihmstar@gmail.com])
AM_INIT_AUTOMAKE([foreign])

AC_CONFIG_SRCDIR([pzb/pzb.hpp])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
CXXFLAGS+=" -std=c++11"

# Check for operating system
AC_MSG_CHECKING([whether we need platform-specific build settings])
case ${host_os} in
  *darwin*)
	CXXFLAGS+=" -stdlib=libc++"
    ;;
esac

echo ${host_os}
echo CXXFLAGS

AC_PROG_INSTALL

# Checks for libraries.
PKG_CHECK_MODULES(libfragmentzip, libfragmentzip >= 1.0)

# Checks for header files.
AC_CHECK_HEADERS([string.h termios.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_SIZE_T
AC_TYPE_UINT32_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([mkdir strstr])

AC_CONFIG_FILES([Makefile pzb/Makefile])
AC_OUTPUT
