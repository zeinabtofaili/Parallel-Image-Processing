#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#include "image.h"

// Run command: ./task1 <image.ppm>
int main(int argc, char** argv) {
  MPI_Init(&argc, &argv);

  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  Image * image;
  if(rank == 0) {
    image = read_image(argv[1]);
  }

  /* Your solution here */
  Pixel* pixels;
  int width=0, height=0;

  if(rank==0){
    width = image->width;
    height = image->height;
    pixels = image->data;
  }

  MPI_Bcast(&width, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&height, 1, MPI_INT, 0, MPI_COMM_WORLD);

  int width_pixels_per_proccess = width /size;
  int wlow = rank * width_pixels_per_proccess;
  int whigh = wlow + width_pixels_per_proccess -1;

  if(rank==size-1){
    whigh = width;
  }

  Pixel *pix = (Pixel *)malloc(sizeof(Pixel)* height * width_pixels_per_proccess);

  MPI_Scatter(pixels, sizeof(Pixel)*height*width_pixels_per_proccess, MPI_UNSIGNED_CHAR, pix, sizeof(Pixel)*height*width_pixels_per_proccess, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

  for(int i=0; i< height ; i++){
    for(int j=0; j< whigh ; j++){
      Pixel *p = &(pix[i *width_pixels_per_proccess + j]);
      p->r= (p->r + p->g + p->b)/3;
      p->g = p->r;
      p->b = p->r;
    }
  }

  MPI_Gather(pix, sizeof(Pixel)*height*width_pixels_per_proccess, MPI_UNSIGNED_CHAR, pixels, sizeof(Pixel)*height*width_pixels_per_proccess, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

  if(rank == 0) {
    write_image("grayscale.ppm", image);
  }
  free(pix);
  MPI_Finalize();
  return 0;
}