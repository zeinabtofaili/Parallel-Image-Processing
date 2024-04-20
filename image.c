#include "image.h"
#include <stdio.h>
#include <stdlib.h>

#define RGB_COLOR_DEPTH 255

Image* read_image(const char * filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "Unable to open file '%s'\n", filename);
        exit(1);
    }

    // read & check the image format
    char buffer[16];
    if (!fgets(buffer, sizeof(buffer), file)) {
        fprintf(stderr, "Invalid image file '%s'\n", filename);
        exit(1);
    }

    if (buffer[0] != 'P' || buffer[1] != '6') {
         fprintf(stderr, "Invalid image format (must be 'P6')\n");
         exit(1);
    }

    Image* image = (Image *) malloc(sizeof(Image));

    // skip comments
    char c = getc(file);
    while (c == '#') {
        while (getc(file) != '\n');
        c = getc(file);
    }
    ungetc(c, file);
    
    // read image size
    if (fscanf(file, "%d %d", &image->width, &image->height) != 2) {
         fprintf(stderr, "Invalid image size (error loading '%s')\n", filename);
         exit(1);
    }

    //read rgb component
    int rgb_depth;
    if (fscanf(file, "%d", &rgb_depth) != 1) {
         fprintf(stderr, "Invalid rgb component (error loading '%s')\n", filename);
         exit(1);
    }

    //check rgb component depth
    if (rgb_depth != RGB_COLOR_DEPTH) {
         fprintf(stderr, "'%s' does not have 8-bits components\n", filename);
         exit(1);
    }

    while (fgetc(file) != '\n') ;

    image->data = (Pixel*) malloc(image->width * image->height * sizeof(Pixel));

    //read pixel data from file
    if (fread(image->data, 3 * image->width, image->height, file) != image->height) {
         fprintf(stderr, "Error reading image '%s'\n", filename);
         exit(1);
    }

    fclose(file);
    return image;
}

void write_image(const char* filename, Image* image) {
    FILE *file;
    file = fopen(filename, "wb");
    if (!file) {
         fprintf(stderr, "Unable to open file '%s'\n", filename);
         exit(1);
    }

    fprintf(file, "P6\n");
    fprintf(file, "%d %d\n", image->width, image->height);
    fprintf(file, "%d\n", RGB_COLOR_DEPTH);
    fwrite(image->data, 3 * image->width, image->height, file);
    fclose(file);
}