// struct corresponding to one RGB pixel
typedef struct {
  unsigned char r, g, b;
} Pixel;

// struct corresponding to a width x height image
typedef struct {
     int width, height;
     Pixel* data;
} Image;

// reads an Image from file 'filename'
Image* read_image(const char * filename);

// writes 'image' to file 'filename'
void write_image(const char* filename, Image* image);
