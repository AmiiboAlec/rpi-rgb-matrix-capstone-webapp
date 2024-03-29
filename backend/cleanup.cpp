#include <stdio.h>

int main (int argc, char **argv) {
    remove("toplevel");
    remove("bottomlevel");
    remove("smfb"); //may need to add more later, but this should be fine for now
}