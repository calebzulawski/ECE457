/* copycat.c
 * Caleb Zulawski
 * 
 * Entrance point of the program.
 */

#include "copycat.h"

int main(int argc, char* argv[]) {
    Options options;
    
    cc_parse_args(argc, argv, &options);
    cc_log(&options);
    cc_copy(&options);

    return 0;
}