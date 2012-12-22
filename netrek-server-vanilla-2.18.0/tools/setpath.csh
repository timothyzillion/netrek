set LIBDIR=/usr/local/games/netrek-server-vanilla/lib
set ROOT=`${LIBDIR}/tools/getpath`
setenv PATH ${ROOT}/bin:${ROOT}/lib:${ROOT}/lib/tools:${PATH}

# designed for people who use who use /bin/csh or /bin/tcsh style shell
# type "source setpath.csh" to use
