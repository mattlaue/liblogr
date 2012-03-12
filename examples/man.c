#include <logr.h>
#include <assert.h>

int
main(int argc, char **argv)
{
    int rc;
    logr_t *logr = logr_getlogger();

    rc = logr_set_timestamp_format(logr, "%H:%M:%S");
    assert(rc == 0);

    rc = logr_set_prefix_format(logr, "[%{timestamp}s] ");
    assert(rc == 0);

    rc = logr_open(logr, "foo.log");
    assert(rc == 0);

    logr_err("Hello %s!\n", "World");
    printf("output appended to 'foo.log'.\n");
    return 0;
}
