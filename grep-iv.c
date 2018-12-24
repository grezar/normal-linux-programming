#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <regex.h>
#include <unistd.h>

static void do_grep(regex_t *pat, FILE *f);

static int opt_invert = 0;
static int opt_ignorecase = 0;

int
main(int argc, char *argv[])
{
    int opt;
    while((opt = getopt(argc, argv, "iv")) != -1) {
        switch (opt) {
            case 'i':
                opt_ignorecase = 1;
                break;
            case 'v':
                opt_invert = 1;
                break;
            case '?':
                fprintf(stderr, "Usage: %s [-iv] [<file>...]\n", argv[0]);
                exit(1);
        }
    }
    argc -= optind;
    argv += optind;

    if (argc < 1) {
        fputs("no pattern\n", stderr);
        exit(1);
    }
    char *pattern = argv[0];
    argc--;
    argv++;

    int regex_mode = REG_EXTENDED | REG_NOSUB | REG_NEWLINE;
    if (opt_ignorecase) regex_mode |= REG_ICASE;
    regex_t regex;
    int err = regcomp(&regex, pattern, regex_mode);
    if (err != 0) {
        char buf[1024];

        regerror(err, &regex, buf, sizeof buf);
        puts(buf);
        exit(1);
    }
    if (argc == 0) {
        do_grep(&regex, stdin);
    }
    else {
        for (int i = 0; i < argc; i++) {
            FILE *f;

            f = fopen(argv[i], "r");
            if(!f) {
                perror(argv[i]);
                exit(1);
            }
            do_grep(&regex, f);
            fclose(f);
        }
    }
    regfree(&regex);
    exit(0);
}

static void
do_grep(regex_t *pat, FILE *src)
{
    char buf[4096];
    int matched;

    while (fgets(buf, sizeof buf, src)) {
        matched = (regexec(pat, buf, 0, NULL, 0) == 0);
        if (opt_invert) {
            matched = !matched;
        }
        if (matched) {
            fputs(buf, stdout);
        }
    }
}
