#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT(libeffect, VERSION, BUG-REPORT-ADDRESS)
AC_CONFIG_SRCDIR([sources/L_ParticleEffect.h])
AM_INIT_AUTOMAKE
AM_CONFIG_HEADER([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

# Checks for libraries.

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([float.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE

# Checks for library functions.
AC_PROG_RANLIB
AC_CONFIG_FILES([Makefile
                 sources/Makefile])
AC_OUTPUT
