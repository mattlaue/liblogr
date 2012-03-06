#include <logr.h>
#include <assert.h>

int
main(int argc, char **argv)
{
    int rc;
    logr_t *logr = logr_getlogger();

    rc = logr_set_timestamp_format(logr, "%F-%T");
    assert(rc == 0);

    rc = logr_set_prefix_format(logr, "[%{timestamp}s] ");
    assert(rc == 0);

    rc = logr_open(logr, "/tmp/foo.log");
    assert(rc == 0);

    logr_err("Hello %s!\n", "World");
    return 0;
}
