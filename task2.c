#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include "image.h"

// Run command: ./task2 <image.ppm> <n>
int main(int argc, char** argv) {
  MPI_Init(&argc, &argv);

  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  Image * image;
  int n = atoi(argv[2]);

  if(rank == 0) {
    image = read_image(argv[1]);
  }

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
  int whigh = wlow + width_pixels_per_proccess;

  if(rank==size-1){
    whigh = width;
  }
  /*Although the size of this array is the same as pixels, we will only fill a small part of it for use*/
  /*Therefore, no unnecessary data will be transferred */
  Pixel *pix = (Pixel *)malloc(sizeof(Pixel)* height *width);

  if(rank ==0){
    /*send from process zero to the other processes their part of the image*/
    /*make sure to include the boundary pixels so that they can blur correctly.*/
    for(int r=1; r<size; r++){
        MPI_Send(pixels+r*(height-n)*(width_pixels_per_proccess),
          sizeof(Pixel)*height*(width_pixels_per_proccess+2*n), 
          MPI_UNSIGNED_CHAR, r, 0, MPI_COMM_WORLD);
    }
    /*since the blurring will be done using the pix array,*/
    /*let process zero have access to it by filling it with pixels data in process zero ONLY*/
    for(int i=0; i< height; i++){
      for(int j=0; j<width; j++){
        int index = i*width+j;
        pix[index] = pixels[index];
      }
    }
  }else{/*Now receive the small image parts by the other processes*/
    MPI_Recv(pix, sizeof(Pixel)* height * (width_pixels_per_proccess+2*n), MPI_UNSIGNED_CHAR, 0, 0,
     MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  }
  /*do the blurring*/
  for(int i=0; i<height; i++){
    for(int j= 0; j<width; j++){
      int red=0, green=0, blue=0;
      for(int k=1; k<=n; k++){
        int A = i - k; if (A < 0) A = 0; /*above*/
        int B = i + k; if (B > height - 1) B = height - 1; /*below*/
        int L = j - k; if (L < 0) L = 0; /*left*/
        int R = j + k; if (R > width - 1) R = width - 1; /*right*/

        Pixel *p = &(pix[i *width + j]);
        Pixel *pr = &(pix[i *width + R]); /*pixel to the right*/
        Pixel *pl = &(pix[i *width + L]); /*pixel to the left*/
        Pixel *pa = &(pix[A *width + j]); /*pixel above*/
        Pixel *pb = &(pix[B *width + j]); /*pixel below*/

        red+= (pr->r + pl->r + pa->r +pb->r);
        green+= (pr->g + pl->g + pa->g +pb->g);
        blue+= (pr->b + pl->b + pa->b +pb->b);

        if(k==n){
          p->r=((p->r+red)/(5+4*(k-1)));
          p->g=((p->g+green)/(5+4*(k-1)));
          p->b=((p->b+blue)/(5+4*(k-1)));
        }
      }
    }
  }
  /*gather all the small parts into the original pixels array after being blurred*/
  MPI_Gather(pix, sizeof(Pixel)*height*width_pixels_per_proccess, MPI_UNSIGNED_CHAR, pixels,
   sizeof(Pixel)*height*width_pixels_per_proccess, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

  if(rank == 0) {
    write_image("blurred.ppm", image);
  }
  free(pix);
  MPI_Finalize();
  return 0;
}