#include "knn.h"

/****************************************************************************/
/* For all the remaining functions you may assume all the images are of the */
/*     same size, you do not need to perform checks to ensure this.         */
/****************************************************************************/

/**************************** A1 code ****************************************/

/* Same as A1, you can reuse your code if you want! */
double distance(Image *a, Image *b) {

  int size = a -> sx * a -> sy;

  int normsq = 0;
  for(int i = 0; i < size; i++){
    normsq += (a -> data[i] - b -> data[i]) * (a -> data[i] - b -> data[i]);
  }

  return sqrt(normsq);

  return 0; 
}

/* Same as A1, you can reuse your code if you want! */
int knn_predict(Dataset *data, Image *input, int K) {
  double* smallest = (double*)malloc(sizeof(double) * K);
  unsigned char* smallest_labels = (unsigned char*) malloc(sizeof(unsigned char) * K);

  for(int i = 0; i < data -> num_items; i++){
    double dist = distance(&(data -> images[i]), input);
    if(i < K){

      smallest[i] = dist;
      smallest_labels[i] = data -> labels[i];
    }else{
      int largest_index = 0;
      for(int j = 0; j < K; j++){

        if(smallest[j] > smallest[largest_index]){
          largest_index = j;
        }

      }
      if(dist < smallest[largest_index]){
        smallest[largest_index] = dist;
        smallest_labels[largest_index] = data -> labels[i];
      }
    }
  }
  int current_count = 0;
  unsigned char current_label = smallest_labels[0];

  int most_count = 0;
  unsigned char most_label = smallest_labels[0];

  for(int i = 0; i < K; i++){
    current_label = smallest_labels[i];
    for(int j = 0; j < K; j++){
      if(smallest_labels[j] == current_label){
        current_count++;
      }
    }
    if(current_count > most_count){
      most_label = current_label;
      most_count = current_count;
    }
    current_count = 0;
  }

  free(smallest);
  free(smallest_labels);

  return most_label;
}

/**************************** A2 code ****************************************/

/* Same as A2, you can reuse your code if you want! */
Dataset *load_dataset(const char *filename) {
  int num_images;
  FILE* file = fopen(filename, "rb");

  if(file == NULL)
    return NULL;

  (void)!fread(&num_images, sizeof(int), 1, file);

  Dataset* dataset = (Dataset*) malloc(sizeof(Dataset));
  dataset -> images = (Image*) malloc(num_images * sizeof(Image));
  dataset -> labels = (unsigned char*)malloc(sizeof(unsigned char) * num_images);
  dataset -> num_items = num_images;

  for(int i = 0; i < num_images; i++){
    unsigned char label, data[784];

    (void)! fread(&label, sizeof(unsigned char), 1, file);
    (void)! fread(data, sizeof(unsigned char), 784, file);

    dataset -> images[i].sx = 28;
    dataset -> images[i].sy = 28;
    dataset -> images[i].data = (unsigned char*) malloc(sizeof(unsigned char) * 784);
    for(int j = 0; j < 784; j++){
      dataset -> images[i].data[j] = data[j];
    }
    dataset -> labels[i] = label;

  }
  fclose(file);

  return dataset;
}

/* Same as A2, you can reuse your code if you want! */
void free_dataset(Dataset *data) {
  for(int i = 0; i < data -> num_items; i++){
    free(data -> images[i].data);
  }
  free(data -> labels);
  free(data -> images);
  free(data);
}


/************************** A3 Code below *************************************/

/**
 * NOTE ON AUTOTESTING:
 *    For the purposes of testing your A3 code, the actual KNN stuff doesn't
 *    really matter. We will simply be checking if (i) the number of children
 *    are being spawned correctly, and (ii) if each child is recieving the 
 *    expected parameters / input through the pipe / sending back the correct
 *    result. If your A1 code didn't work, then this is not a problem as long
 *    as your program doesn't crash because of it
 */

/**
 * This function should be called by each child process, and is where the 
 * kNN predictions happen. Along with the training and testing datasets, the
 * function also takes in 
 *    (1) File descriptor for a pipe with input coming from the parent: p_in
 *    (2) File descriptor for a pipe with output going to the parent:  p_out
 * 
 * Once this function is called, the child should do the following:
 *    - Read an integer `start_idx` from the parent (through p_in)
 *    - Read an integer `N` from the parent (through p_in)
 *    - Call `knn_predict()` on testing images `start_idx` to `start_idx+N-1`
 *    - Write an integer representing the number of correct predictions to
 *        the parent (through p_out)
 */
void child_handler(Dataset *training, Dataset *testing, int K, 
                   int p_in, int p_out) {
  // TODO: Compute number of correct predictions from the range of data 
  //      provided by the parent, and write it to the parent through `p_out`.


  int start_idx = 0;
  int N = 0;
  int num_correct = 0;

  if(read(p_in, &start_idx, sizeof(int)) < 0){
    perror("read");
    exit(1);
  }

  if(read(p_in, &N, sizeof(int)) < 0){
    perror("read");
    exit(1);
  }

  for(int i = start_idx; i < start_idx+N; i++){
    int predicted = knn_predict(training, &(testing -> images[i]), K);
    // printf("start: %d, end: %d, pred: %d, cor: %d\n", start_idx,start_idx+N-1,  predicted, testing-> labels[i]);
    if(predicted == testing -> labels[i]){
      num_correct++;
      // printf("CORRECT: %d\n", predicted);
    }else{
      // printf("WRONG: %d, %d\n", predicted, testing -> labels[i]);
    }
  }

  if(write(p_out, &num_correct, sizeof(int)) < 0){
    perror("write");
    exit(1);
  }

  return;
}