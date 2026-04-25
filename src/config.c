#include "project.h"
// here reading block_size from config.ini
size_t parse_config_block_size(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) return 500000;

    char line[256];
    size_t b_size = 500000;

    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '#' || line[0] == '[' || line[0] == '\n') continue;
        if (strstr(line, "block_size")) {
            char* value = strchr(line, '=');
            if (value) b_size = (size_t)atoll(value + 1);
        }
    }
    fclose(file);

    if (b_size < 100000) b_size = 100000;
    if (b_size > 900000) b_size = 900000;
    return b_size;
}

// reading  rle1_threshold from config.ini
int parse_config_threshold(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) return 4; // default = standard BZip2 minimum

    char line[256];
    int threshold = 4;

    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '#' || line[0] == '[' || line[0] == '\n') continue;
        if (strstr(line, "rle1_threshold")) {
            char* value = strchr(line, '=');
            if (value) threshold = atoi(value + 1);
        }
    }
    fclose(file);

    if (threshold < 4)   threshold = 4;   // minimum is 4 (BZip2 spec)
    if (threshold > 255) threshold = 255;  // maximum sensible value
    return threshold;
}

// reading  input file and divides into blocks
BlockManager divide_into_blocks(const char* filename, size_t block_size) {
    BlockManager manager;
    manager.block_size = block_size;
    manager.num_blocks = 0;
    manager.blocks = NULL;

    FILE* file = fopen(filename, "rb");
    if (!file) {
        printf("Error: cannot open '%s'\n", filename);
        return manager;
    }

    fseek(file, 0, SEEK_END);
    size_t file_size = (size_t)ftell(file);
    rewind(file);

    if (file_size == 0) { fclose(file); return manager; }

    manager.num_blocks = (int)((file_size + block_size - 1) / block_size);
    manager.blocks = (Block*)malloc(manager.num_blocks * sizeof(Block));

    size_t remaining = file_size;
    for (int i = 0; i < manager.num_blocks; i++) {
        size_t chunk = (remaining > block_size) ? block_size : remaining;
        manager.blocks[i].data = (unsigned char*)malloc(chunk);
        manager.blocks[i].size = fread(manager.blocks[i].data, 1, chunk, file);
        manager.blocks[i].original_size = manager.blocks[i].size;
        remaining -= chunk;
    }
    fclose(file);
    return manager;
}

int reassemble_blocks(BlockManager* manager, const char* output_filename) {
    FILE* file = fopen(output_filename, "wb");
    if (!file) {
        printf("Error: cannot create '%s'\n", output_filename);
        return -1;
    }
    for (int i = 0; i < manager->num_blocks; i++)
        fwrite(manager->blocks[i].data, 1, manager->blocks[i].size, file);
    fclose(file);
    return 0;
}

void free_block_manager(BlockManager* manager) {
    if (!manager) return;
    for (int i = 0; i < manager->num_blocks; i++)
        free(manager->blocks[i].data);
    free(manager->blocks);
    manager->blocks = NULL;
    manager->num_blocks = 0;
}