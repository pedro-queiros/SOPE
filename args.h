#define BUFFER_SIZE     256

struct Args {
    int all;
    int bytes;
    int block_size;
    int count_links; //set to true by default
    int dereference;
    int separate_dirs;
    int max_depth; //-1 if no depth limit
    char path[BUFFER_SIZE];
};
