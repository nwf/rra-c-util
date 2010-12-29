/*
 * PAM option parsing test suite.
 *
 * Written by Russ Allbery <rra@stanford.edu>
 * Copyright 2010 Board of Trustees, Leland Stanford Jr. University
 *
 * See LICENSE for licensing terms.
 */

#include <config.h>
#include <portable/pam.h>
#include <portable/system.h>

#include <syslog.h>

#include <pam-util/args.h>
#include <pam-util/options.h>
#include <pam-util/vector.h>
#include <tests/fakepam/testing.h>
#include <tests/tap/basic.h>

/* The configuration struct we will use for testing. */
struct pam_config {
    struct vector *cells;
    bool debug;
    bool ignore_root;
    long minimum_uid;
    char *program;
};

#define K(name) (#name), offsetof(struct pam_config, name)

/* The rules specifying the configuration options. */
struct option options[] = {
    { K(cells),       true,  LIST   (NULL)  },
    { K(debug),       true,  BOOL   (false) },
    { K(ignore_root), false, BOOL   (true)  },
    { K(minimum_uid), true,  NUMBER (0)     },
    { K(program),     true,  STRING (NULL)  },
};
const size_t optlen = sizeof(options) / sizeof(options[0]);

/*
 * A macro used to parse the various ways of spelling booleans.  This reuses
 * the argv_bool variable, setting it to the first value provided and then
 * calling putil_args_parse() on it.  It then checks whether the provided
 * config option is set to the expected value.
 */
#define TEST_BOOL(a, c, v)                                              \
    do {                                                                \
        argv_bool[0] = (a);                                             \
        status = putil_args_parse(args, 1, argv_bool, options, optlen); \
        ok(status, "Parse of %s", (a));                                 \
        is_int((v), (c), "...and value is correct");                    \
        ok(pam_output() == NULL, "...and no output");                   \
    } while (0);

/*
 * A macro used to test error reporting from putil_args_parse().  This reuses
 * the argv_err variable, setting it to the first value provided and then
 * calling putil_args_parse() on it.  It then recovers the error message and
 * expects it to match the severity and error message given.
 */
#define TEST_ERROR(a, p, e)                                             \
    do {                                                                \
        argv_err[0] = (a);                                              \
        status = putil_args_parse(args, 1, argv_err, options, optlen);  \
        ok(status, "Parse of %s", (a));                                 \
        asprintf(&expected, "%d %s", (p), (e));                         \
        seen = pam_output();                                            \
        is_string(expected, seen, "...error for %s", (a));              \
        free(seen);                                                     \
        free(expected);                                                 \
    } while (0);


/*
 * Allocate and initialize a new struct config.
 */
static struct pam_config *
config_new(void)
{
    struct pam_config *config;

    config = calloc(1, sizeof(struct pam_config));
    if (config == NULL)
        sysbail("cannot allocate memory");
    config->cells = NULL;
    config->program = NULL;
    return config;
}


/*
 * Free a struct config and all of its members.
 */
static void
config_free(struct pam_config *config)
{
    if (config->cells != NULL)
        vector_free(config->cells);
    if (config->program != NULL)
        free(config->program);
    free(config);
}


int
main(void)
{
    pam_handle_t *pamh;
    struct pam_args *args;
    struct pam_conv conv = { NULL, NULL };
    bool status;
    struct vector *cells;
    char *program, *seen, *expected;
    const char *argv_bool[2] = { NULL, NULL };
    const char *argv_err[2] = { NULL, NULL };
    const char *argv_empty[] = { NULL };
    const char *argv_all[] = {
        "cells=stanford.edu,ir.stanford.edu", "debug", "ignore_root",
        "minimum_uid=1000", "program=/bin/true"
    };
#ifdef HAVE_KERBEROS
    char *krb5conf;
#endif

    if (pam_start("test", NULL, &conv, &pamh) != PAM_SUCCESS)
        sysbail("cannot create pam_handle_t");
    args = putil_args_new(pamh, 0);
    if (args == NULL)
        sysbail("cannot create PAM argument struct");

    plan(134);

    /* First, check just the defaults. */
    args->config = config_new();
    status = putil_args_defaults(args, options, optlen);
    ok(status, "Setting the defaults");
    ok(args->config->cells == NULL, "...cells default");
    is_int(false, args->config->debug, "...debug default");
    is_int(true, args->config->ignore_root, "...ignore_root default");
    is_int(0, args->config->minimum_uid, "...minimum_uid default");
    ok(args->config->program == NULL, "...program default");

    /* Now parse an empty set of PAM arguments.  Nothing should change. */
    status = putil_args_parse(args, 0, argv_empty, options, optlen);
    ok(status, "Parse of empty argv");
    ok(args->config->cells == NULL, "...cells still default");
    is_int(false, args->config->debug, "...debug still default");
    is_int(true, args->config->ignore_root, "...ignore_root still default");
    is_int(0, args->config->minimum_uid, "...minimum_uid still default");
    ok(args->config->program == NULL, "...program still default");

    /* Now, check setting everything. */
    status = putil_args_parse(args, 5, argv_all, options, optlen);
    ok(status, "Parse of full argv");
    ok(args->config->cells != NULL, "...cells is set");
    is_int(2, args->config->cells->count, "...with two cells");
    is_string("stanford.edu", args->config->cells->strings[0],
              "...first is stanford.edu");
    is_string("ir.stanford.edu", args->config->cells->strings[1],
              "...second is ir.stanford.edu");
    is_int(true, args->config->debug, "...debug is set");
    is_int(true, args->config->ignore_root, "...ignore_root is set");
    is_int(1000, args->config->minimum_uid, "...minimum_uid is set");
    is_string("/bin/true", args->config->program, "...program is set");
    config_free(args->config);
    args->config = NULL;

    /* Test deep copying of defaults. */
    cells = vector_new();
    if (cells == NULL)
        sysbail("cannot allocate memory");
    vector_add(cells, "foo.com");
    vector_add(cells, "bar.com");
    options[0].defaults.list = cells;
    program = strdup("/bin/false");
    if (program == NULL)
        sysbail("cannot allocate memory");
    options[4].defaults.string = program;
    args->config = config_new();
    status = putil_args_defaults(args, options, optlen);
    ok(status, "Setting defaults with new defaults");
    ok(args->config->cells != NULL, "...cells is set");
    is_int(2, args->config->cells->count, "...with two cells");
    is_string("foo.com", args->config->cells->strings[0],
              "...first is foo.com");
    is_string("bar.com", args->config->cells->strings[1],
              "...second is bar.com");
    is_string("/bin/false", args->config->program,
              "...program is /bin/false");
    status = putil_args_parse(args, 5, argv_all, options, optlen);
    ok(status, "Parse of full argv after defaults");
    ok(args->config->cells != NULL, "...cells is set");
    is_int(2, args->config->cells->count, "...with two cells");
    is_string("stanford.edu", args->config->cells->strings[0],
              "...first is stanford.edu");
    is_string("ir.stanford.edu", args->config->cells->strings[1],
              "...second is ir.stanford.edu");
    is_int(true, args->config->debug, "...debug is set");
    is_int(true, args->config->ignore_root, "...ignore_root is set");
    is_int(1000, args->config->minimum_uid, "...minimum_uid is set");
    is_string("/bin/true", args->config->program, "...program is set");
    is_string("foo.com", cells->strings[0], "...first cell after parse");
    is_string("bar.com", cells->strings[1], "...second cell after parse");
    is_string("/bin/false", program, "...string after parse");
    config_free(args->config);
    args->config = NULL;
    is_string("foo.com", cells->strings[0], "...first cell after free");
    is_string("bar.com", cells->strings[1], "...second cell after free");
    is_string("/bin/false", program, "...string after free");
    options[0].defaults.list = NULL;
    options[4].defaults.string = NULL;
    vector_free(cells);
    free(program);

    /* Should be no errors so far. */
    ok(pam_output() == NULL, "No errors so far");

    /* Test various ways of spelling booleans. */
    args->config = config_new();
    TEST_BOOL("debug", args->config->debug, true);
    TEST_BOOL("debug=false", args->config->debug, false);
    TEST_BOOL("debug=true", args->config->debug, true);
    TEST_BOOL("debug=no", args->config->debug, false);
    TEST_BOOL("debug=yes", args->config->debug, true);
    TEST_BOOL("debug=off", args->config->debug, false);
    TEST_BOOL("debug=on", args->config->debug, true);
    TEST_BOOL("debug=0", args->config->debug, false);
    TEST_BOOL("debug=1", args->config->debug, true);
    TEST_BOOL("debug=False", args->config->debug, false);
    TEST_BOOL("debug=trUe", args->config->debug, true);
    TEST_BOOL("debug=No", args->config->debug, false);
    TEST_BOOL("debug=Yes", args->config->debug, true);
    TEST_BOOL("debug=OFF", args->config->debug, false);
    TEST_BOOL("debug=ON", args->config->debug, true);
    config_free(args->config);
    args->config = NULL;

    /* Test for various parsing errors. */
    args->config = config_new();
    TEST_ERROR("debug=", LOG_ERR,
               "invalid boolean in setting: debug=");
    TEST_ERROR("debug=truth", LOG_ERR,
               "invalid boolean in setting: debug=truth");
    TEST_ERROR("minimum_uid", LOG_ERR,
               "value missing for option minimum_uid");
    TEST_ERROR("minimum_uid=", LOG_ERR,
               "value missing for option minimum_uid=");
    TEST_ERROR("minimum_uid=foo", LOG_ERR,
               "invalid number in setting: minimum_uid=foo");
    TEST_ERROR("minimum_uid=1000foo", LOG_ERR,
               "invalid number in setting: minimum_uid=1000foo");
    TEST_ERROR("program", LOG_ERR, "value missing for option program");
    TEST_ERROR("cells", LOG_ERR, "value missing for option cells");
    config_free(args->config);
    args->config = NULL;

#ifdef HAVE_KERBEROS

    /* Test for Kerberos krb5.conf option parsing. */
    krb5conf = test_file_path("data/krb5.conf");
    if (krb5conf == NULL)
        bail("cannot find data/krb5.conf");
    if (setenv("KRB5_CONFIG", krb5conf, 1) < 0)
        sysbail("cannot set KRB5_CONFIG");
    krb5_free_context(args->ctx);
    status = krb5_init_context(&args->ctx);
    if (status != 0)
        bail("cannot parse test krb5.conf file");
    args->config = config_new();
    status = putil_args_defaults(args, options, optlen);
    ok(status, "Setting the defaults");
    status = putil_args_krb5(args, "testing", options, optlen);
    ok(status, "Options from krb5.conf");
    ok(args->config->cells == NULL, "...cells default");
    is_int(true, args->config->debug, "...debug set from krb5.conf");
    is_int(true, args->config->ignore_root, "...ignore_root default");
    is_int(1000, args->config->minimum_uid,
           "...minimum_uid set from krb5.conf");
    ok(args->config->program == NULL, "...program default");
    status = putil_args_krb5(args, "other-test", options, optlen);
    ok(status, "Options from krb5.conf (other-test)");
    is_int(-1000, args->config->minimum_uid,
           "...minimum_uid set from krb5.conf other-test");

    /* Test with a realm set, which should expose more settings. */
    krb5_free_context(args->ctx);
    status = krb5_init_context(&args->ctx);
    if (status != 0)
        bail("cannot parse test krb5.conf file");
    args->realm = strdup("FOO.COM");
    if (args->realm == NULL)
        sysbail("cannot allocate memory");
    status = putil_args_krb5(args, "testing", options, optlen);
    ok(status, "Options from krb5.conf with FOO.COM");
    is_int(2, args->config->cells->count, "...cells count from krb5.conf");
    is_string("foo.com", args->config->cells->strings[0],
              "...first cell from krb5.conf");
    is_string("bar.com", args->config->cells->strings[1],
              "...second cell from krb5.conf");
    is_int(true, args->config->debug, "...debug set from krb5.conf");
    is_int(true, args->config->ignore_root, "...ignore_root default");
    is_int(1000, args->config->minimum_uid,
           "...minimum_uid set from krb5.conf");
    is_string("/bin/false", args->config->program,
              "...program from krb5.conf");

    /* Test with a different realm. */
    free(args->realm);
    args->realm = strdup("BAR.COM");
    if (args->realm == NULL)
        sysbail("cannot allocate memory");
    status = putil_args_krb5(args, "testing", options, optlen);
    ok(status, "Options from krb5.conf with BAR.COM");
    is_int(2, args->config->cells->count, "...cells count from krb5.conf");
    is_string("bar.com", args->config->cells->strings[0],
              "...first cell from krb5.conf");
    is_string("foo.com", args->config->cells->strings[1],
              "...second cell from krb5.conf");
    is_int(true, args->config->debug, "...debug set from krb5.conf");
    is_int(true, args->config->ignore_root, "...ignore_root default");
    is_int(1000, args->config->minimum_uid,
           "...minimum_uid set from krb5.conf");
    is_string("echo /bin/true", args->config->program,
              "...program from krb5.conf");
    config_free(args->config);
    args->config = config_new();
    status = putil_args_krb5(args, "other-test", options, optlen);
    ok(status, "Options from krb5.conf (other-test with realm)");
    ok(args->config->cells == NULL, "...cells is NULL");
    is_string("echo /bin/true", args->config->program,
              "...program from krb5.conf");
    config_free(args->config);
    args->config = NULL;

    /* Test error reporting from the krb5.conf parser. */
    args->config = config_new();
    status = putil_args_krb5(args, "bad-number", options, optlen);
    ok(status, "Options from krb5.conf (bad-number)");
    asprintf(&expected, "%d invalid number in krb5.conf setting for %s: %s",
             LOG_ERR, "minimum_uid", "1000foo");
    seen = pam_output();
    is_string(expected, seen, "...and correct error reported");
    free(expected);
    free(seen);
    config_free(args->config);
    args->config = NULL;
#else /* !HAVE_KERBEROS */
    skip_block(30, "Kerberos support not configured");
#endif

    putil_args_free(args);
    return 0;
}