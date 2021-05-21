#include "knn.h"

// Makefile included in starter:
//    To compile:               make
//    To decompress dataset:    make datasets
//
// Example of running validation (K = 3, 8 processes):
//    ./classifier 3 datasets/training_data.bin datasets/testing_data.bin 8

/*****************************************************************************/
/* This file should only contain code for the parent process. Any code for   */
/*      the child process should go in `knn.c`. You've been warned!          */
/*****************************************************************************/

/**
 * main() takes in 4 command line arguments:
 *   - K:  The K value for kNN
 *   - training_data: A binary file containing training image / label data
 *   - testing_data: A binary file containing testing image / label data
 *   - num_procs: The number of processes to be used in validation
 * 
 * You need to do the following:
 *   - Parse the command line arguments, call `load_dataset()` appropriately.
 *   - Create the pipes to communicate to and from children
 *   - Fork and create children, close ends of pipes as needed
 *   - All child processes should call `child_handler()`, and exit after.
 *   - Parent distributes the testing set among childred by writing:
 *        (1) start_idx: The index of the image the child should start at
 *        (2)    N:      Number of images to process (starting at start_idx)
 *     Each child should gets N = ceil(test_set_size / num_procs) images
 *      (The last child might get fewer if the numbers don't divide perfectly)
 *   - Parent waits for children to exit, reads results through pipes and keeps
 *      the total sum.
 *   - Print out (only) one integer to stdout representing the number of test 
 *      images that were correctly classified by all children.
 *   - Free all the data allocated and exit.
 */
int main(int argc, char *argv[]) {
  // TODO: Handle command line arguments
  if(argc != 5){
    exit(1);
  }

  int K = atoi(argv[1]);
  Dataset* dataset_train = load_dataset(argv[2]);
  Dataset* dataset_test = load_dataset(argv[3]);
  int num_procs = atoi(argv[4]);
  int test_set_size = dataset_test -> num_items;
  // int test_set_size = 1000;
  int N = ceil((double)test_set_size / (double)num_procs);

  int total_correct = 0;


  int pid;

  for(int i = 0; i < num_procs; i++){
    // p will be parent write to child
    int p[2];
    // q will be child write to parent
    int q[2];
    if(pipe(p) == -1){
      perror("pipe");
      exit(1);
    }
    if(pipe(q) == -1){
      perror("pipe");
      exit(1);
    }

    pid = fork();
    if(pid < 0){
      perror("fork");
      exit(1);
    
    // handle child case
    }else if(pid == 0){
      if(close(p[1]) == -1){
        perror("close");
        exit(1);
      }
      if(close(q[0]) == -1){
        perror("close");
        exit(1);
      }

      child_handler(dataset_train, dataset_test, K, p[0], q[1]);

      if(close(p[0]) == -1){
        perror("close");
        exit(1);
      }
      if(close(q[1]) == -1){
        perror("close");
        exit(1);
      }
      free_dataset(dataset_train);
      free_dataset(dataset_test);
      exit(0);

    // handle parent case
    }else{

      if(close(p[0]) == -1){
        perror("close");
        exit(1);
      }
      if(close(q[1]) == -1){
        perror("close");
        exit(1);
      }
      int start_idx = i*N;
      if(i == num_procs-1){
        N = test_set_size-(i*N);
      }

      if(write(p[1], &start_idx, sizeof(int)) < 0){
        perror("write");
        exit(1);
      }

      if(write(p[1], &N, sizeof(int)) < 0){
        perror("write");
        exit(1);
      }


      int child_correct = 0;

      if(read(q[0], &child_correct, sizeof(int)) < 0){
        perror("read");
        exit(1);
      }


      total_correct += child_correct;

      if(close(p[1]) == -1){
        perror("close");
        exit(1);
      }
      if(close(q[0]) == -1){
        perror("close");
        exit(1);
      }
    }
  }


  free_dataset(dataset_train);
  free_dataset(dataset_test);


  // Print out answer
  printf("%d\n", total_correct);
  return 0;
}
