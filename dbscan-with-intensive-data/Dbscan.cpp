#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <set>
#include <vector>

#define DATASET_SIZE 1864620
// #define DATASET_SIZE 10000
#define DIMENTION 2
#define ELIPSON 1.5
#define MIN_POINTS 4

using namespace std;

int importDataset(char const *fname, int N, long double **dataset) {
  FILE *fp = fopen(fname, "r");

  if (!fp) {
    printf("Unable to open file\n");
    return (1);
  }

  char buf[4096];
  int rowCnt = 0;
  int colCnt = 0;
  while (fgets(buf, 4096, fp) && rowCnt < N) {
    colCnt = 0;

    char *field = strtok(buf, ",");
    long double tmp;
    sscanf(field, "%Lf", &tmp);
    dataset[rowCnt][colCnt] = tmp;

    while (field) {
      colCnt++;
      field = strtok(NULL, ",");

      if (field != NULL) {
        long double tmp;
        sscanf(field, "%Lf", &tmp);
        dataset[rowCnt][colCnt] = tmp;
      }
    }
    rowCnt++;
  }

  fclose(fp);

  return 0;
}

class DBSCAN {
 private:
  long double **dataset;
  double elipson;
  int minPoints;
  int cluster;
  long int *clusters;
  long double getDistance(long int center, long int neighbor);
  vector<int> findNeighbors(int pos);

 public:
  DBSCAN(long double **loadData);
  void run();
  void results();
};

int main(int, char **) {
  long double **dataset =
      (long double **)malloc(sizeof(long double *) * DATASET_SIZE);
  for (long int i = 0; i < DATASET_SIZE; i++) {
    dataset[i] = (long double *)malloc(sizeof(long double) * DIMENTION);
  }

  importDataset("../dataset/dataset.txt", DATASET_SIZE, dataset);

  // Initialize DBSCAN with dataset
  DBSCAN dbscan(dataset);

  // Run the DBSCAN algorithm
  dbscan.run();

  // Print the cluster results of DBSCAN
  dbscan.results();

  for (long int i = 0; i < DATASET_SIZE; i++) {
    free(dataset[i]);
  }
  free(dataset);

  return 0;
}

DBSCAN::DBSCAN(long double **loadData) {
  clusters = (long int *)malloc(sizeof(long int) * DATASET_SIZE);

  dataset = (long double **)malloc(sizeof(long double *) * DATASET_SIZE);
  for (long int i = 0; i < DATASET_SIZE; i++) {
    dataset[i] = (long double *)malloc(sizeof(long double) * DIMENTION);
  }

  elipson = ELIPSON;
  minPoints = MIN_POINTS;
  cluster = 0;

  for (int i = 0; i < DATASET_SIZE; i++) {
    dataset[i][0] = loadData[i][0];
    dataset[i][1] = loadData[i][1];
    clusters[i] = 0;
  }
}

long double DBSCAN::getDistance(long int center, long int neighbor) {
  long double dist = (dataset[center][0] - dataset[neighbor][0]) *
                         (dataset[center][0] - dataset[neighbor][0]) +
                     (dataset[center][1] - dataset[neighbor][1]) *
                         (dataset[center][1] - dataset[neighbor][1]);

  return dist;
}

void DBSCAN::run() {
  // Neighbors of the point
  vector<int> neighbors;

  for (int i = 0; i < DATASET_SIZE; i++) {
    if (clusters[i] == 0) {
      // Find neighbors of point P
      neighbors = findNeighbors(i);

      // Mark noise points
      if (neighbors.size() < minPoints) {
        clusters[i] = -1;
      } else {
        // Increment cluster and initialize it will the current point
        cluster++;

        clusters[i] = cluster;
        // Expand the neighbors of point P
        for (int j = 0; j < neighbors.size(); j++) {
          // Mark neighbour as point Q
          int dataIndex = neighbors[j];

          if (dataIndex == i) continue;

          if (clusters[dataIndex] == -1) {
            clusters[dataIndex] = cluster;
          } else if (clusters[dataIndex] == 0) {
            clusters[dataIndex] = cluster;

            // Expand more neighbors of point Q
            vector<int> moreNeighbors;
            moreNeighbors = findNeighbors(dataIndex);

            // Continue when neighbors point is higher than minPoint threshold

            if (moreNeighbors.size() >= minPoints) {
              // Check if neighbour of Q already exists in neighbour of P
              for (int x = 0; x < moreNeighbors.size(); x++) {
                neighbors.push_back(moreNeighbors[x]);
              }
            }
          }
        }
      }
    }
  }
}

void DBSCAN::results() {
  printf("Number of clusters: %d\n", cluster);
  int noises = 0;
  for (int i = 0; i < DATASET_SIZE; i++) {
    if (clusters[i] == -1) {
      noises++;
    }
  }

  printf("Noises: %d\n", noises);
}

vector<int> DBSCAN::findNeighbors(int pos) {
  vector<int> neighbors;

  for (int x = 0; x < DATASET_SIZE; x++) {
    // Compute neighbor points of a point at position "pos"
    long double distance = getDistance(pos, x);
    if (distance < elipson * elipson) {
      neighbors.push_back(x);
    }
  }

  return neighbors;
}