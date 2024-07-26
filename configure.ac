AC_PREREQ([2.69])
AC_INIT([partialZipBrowser], m4_esyscmd([git rev-list --count HEAD | tr -d '\n']), [tihmstar@gmail.com])
AM_INIT_AUTOMAKE(partialZipBrowser, m4_esyscmd([git rev-list --count HEAD | tr -d '\n']))


AC_CONFIG_SRCDIR([pzb/pzb.hpp])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
CXXFLAGS+=" -std=c++11"
CXXFLAGS+=" -D PZB_VERSION_COMMIT_COUNT=\\\"$(git rev-list --count HEAD | tr -d '\n')\\\""
CXXFLAGS+=" -D PZB_VERSION_COMMIT_SHA=\\\"$(git rev-parse HEAD | tr -d '\n')\\\""


AC_CANONICAL_HOST
# Check for operating system
AC_MSG_CHECKING([whether we need platform-specific build settings])
case $host_os in
  darwin* )
	CXXFLAGS+=" -stdlib=libc++"
        ;;
esac

AC_PROG_INSTALL

# Checks for libraries.
PKG_CHECK_MODULES(libfragmentzip, libfragmentzip >= 76)

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
