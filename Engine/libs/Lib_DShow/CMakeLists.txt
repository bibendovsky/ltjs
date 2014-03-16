set(LTJS_TARGET ltjs_lib_dshow)

ltjs_add_default_options()

add_definitions(-D_WIN32_DCOM)
add_definitions(-DWINVER=0x500)
add_compile_options($<$<CONFIG:DEBUG>:-DDEBUG>)

include_directories(
    .
)

set(
    HDRS
    dllsetup.h
    dsschedule.h
    fourcc.h
    measure.h
    msgthrd.h
    mtype.h
    outputq.h
    pstream.h
    pullpin.h
    refclock.h
    reftime.h
    renbase.h
    seekpt.h
    source.h
    streams.h
    strmctl.h
    sysclock.h
    transfrm.h
    transip.h
    videoctl.h
    vtrans.h
    winctrl.h
    winutil.h
    wxdebug.h
    wxlist.h
    wxutil.h
)

set(
    SRCS
    amextra.cpp
    amfilter.cpp
    amvideo.cpp
    combase.cpp
    cprop.cpp
    ctlutil.cpp
    ddmm.cpp
    dllentry.cpp
    dllsetup.cpp
    mtype.cpp
    outputq.cpp
    pstream.cpp
    pullpin.cpp
    refclock.cpp
    renbase.cpp
    schedule.cpp
    seekpt.cpp
    source.cpp
    strmctl.cpp
    sysclock.cpp
    transfrm.cpp
    transip.cpp
    videoctl.cpp
    vtrans.cpp
    winctrl.cpp
    winutil.cpp
    wxdebug.cpp
    wxlist.cpp
    wxutil.cpp
)

add_library(
    ${LTJS_TARGET} STATIC
    ${HDRS}
    ${SRCS}
)

if (MSVC)
    target_link_libraries(
        ${LTJS_TARGET}
        strmiids
    )
endif ()
