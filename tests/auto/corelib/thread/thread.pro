TEMPLATE=subdirs
SUBDIRS=\
    qatomicint \
    qatomicinteger \
    qresultstore \
    qfuture \
    qfuturesynchronizer \
    qmutex \
    qmutexlocker \
    qreadlocker \
    qreadwritelock \
    qsemaphore \
    qthread \
    qthreadonce \
    qthreadpool \
    qthreadstorage \
    qwaitcondition \
    qwritelocker

# XXXAR: TODO: add back once compiler has been fixed:   qatomicpointer

qtHaveModule(concurrent) {
    SUBDIRS += \
        qfuturewatcher
}
