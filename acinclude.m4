dnl   amSynth (c)2001 Nick Dowell
dnl   ---------------------------
dnl - Macros for configure script

AC_DEFUN(AC_PROFILING,
[
  AC_ARG_ENABLE(profile,
		[  --enable-profile	  enable profiling compiler options])

  if test "${enable_profile}" = "yes" ; then
  	AC_MSG_RESULT([yes])
  	if test "$GCC" = "yes" ; then
  		CXXFLAGS="${CXXFLAGS} -pg"
		AC_MSG_RESULT([yes])
  	else
  		AC_MSG_RESULT(we do not have gcc, hope we have a nice profiler...)
  		CXXFLAGS="${CXXFLAGS} -p"
  	fi
  else
	AC_MSG_RESULT([no])
  fi
])

AC_DEFUN(AC_DEBUGGING,
[
  AC_ARG_ENABLE(debug,
		[  --enable-debug	  enable debugging compiler options])

  if test "${enable_debug}" = "yes" ; then
  	AC_MSG_RESULT([yes])
  	CXXFLAGS="${CXXFLAGS} -g -Wall -D_DEBUG"
	COMPOPSTR="debugging ${COMPOPSTR}"
	OPTION_DEBUG="yes"
  else
	AC_MSG_RESULT([no])
  fi

])

AC_DEFUN(AC_OPTIMISE,
[
  AC_ARG_WITH(	O,
    		[  --with-O=level      set the compiler optimisation level (default=6)],
  		opt="$withval", opt="6")
  case "$OPTION_DEBUG" in
    yes)
	AC_MSG_RESULT([none (debugging enabled)])
	;;
    *)
	CXXFLAGS="${CXXFLAGS} -ffast-math -fno-exceptions -fomit-frame-pointer -O$opt"
	AC_MSG_RESULT([$opt])
	;;
  esac
])

AC_DEFUN(AC_CPU,
[
  AC_ARG_WITH(	cpu,
    		[  --with-cpu=cputype      optimise for cpu _xxx_ (i386,i486,i586,i686,k6 default=i586)],
  		cpu="$withval", cpu="i586")
  case "$OPTION_DEBUG" in
    yes)
	AC_MSG_RESULT([none (debugging enabled)])
	;;
    *)
	CXXFLAGS="${CXXFLAGS} -march=$cpu"
	AC_MSG_RESULT([$cpu])
	;;
  esac
])
