#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#import <Foundation/Foundation.h>

extern char **environ;

int string_ends_with(const char *str, const char *suffix) {
    int str_len = strlen(str);
    int suffix_len = strlen(suffix);

    return (str_len >= suffix_len) && (0 == strcmp(str + (str_len-suffix_len), suffix));
}

int is_directory(const char *path) {
    struct stat statbuf;
    if (stat(path, &statbuf) != 0)
       return 0;
    return S_ISDIR(statbuf.st_mode);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "usage: open <file> [args]\n\nsupported files:\n    executable program/shell script\n    MacOS application (folder ending in .app)\n");
        return 1;
    }
    if (is_directory(argv[1])) {
        if (string_ends_with(argv[1], ".app") || string_ends_with(argv[1], ".app/")) {
            NSString *plist = [NSString stringWithFormat:@"%@/Contents/Info.plist", [NSString stringWithUTF8String:argv[1]]];
            NSDictionary *dict = [NSDictionary dictionaryWithContentsOfFile:plist];
            if (dict) {
                NSString *executable = [dict objectForKey:@"CFBundleExecutable"];
                if (executable) {
                    const char *executablePath = [[NSString stringWithFormat:@"%@/Contents/MacOS/%@", [NSString stringWithUTF8String:argv[1]], executable] UTF8String];
                    execve(executablePath, argv + 1, environ);
                    fprintf(stderr, "unable to run %s: ", executablePath);
                    perror("");
                } else {
                    fprintf(stderr, "%s/Contents/Info.plist does not contain CFBundleExecutable\n", argv[1]);
                }
            } else {
                fprintf(stderr, "%s/Contents/Info.plist is invald or does not exist\n", argv[1]);
            }
        } else {
            fprintf(stderr, "%s is not supported\n", argv[1]);
        }
    } else if (access(argv[1], F_OK) != -1) {
        fprintf(stderr, "%s does not exist\n", argv[1]);
    } else {
        execve(argv[1], argv + 1, environ);
        fprintf(stderr, "unable to run %s: ", argv[1]);
        perror("");
    }
    return 1;
}
