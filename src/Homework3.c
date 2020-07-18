#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

typedef int bool;

#define TRUE 1
#define FALSE 0
#define NUM_THREADS 27

char fileName[] = "SudokuPuzzle.txt";
int sudokuPuzzle[9][9];

int tid_column[9];
int tid_row[9];
int tid_subgrid[9];

bool validity_columns[9];
bool validity_rows[9];
bool validity_subGrids[9];

void generatePuzzleArray();
void *worker(void *param);

typedef struct {
  int topRow;
  int bottomRow;
  int leftColumn;
  int rightColumn;
} parameters;

int main(int argc, char *argv[])
{
  int count;
  pthread_t tid[NUM_THREADS];
  parameters workerParams[NUM_THREADS];
  
  generatePuzzleArray();

  //Generate worker thread parameters for columns
  for (count = 0; count<9; count++)
  {
    workerParams[count].topRow = 0;
    workerParams[count].bottomRow = 8;
    workerParams[count].leftColumn = count;
    workerParams[count].rightColumn = count;
  }

  //Generate worker thread parameters for rows
  for (count = 9; count<18; count++)
  {
    workerParams[count].topRow = count - 9;
    workerParams[count].bottomRow = count - 9;
    workerParams[count].leftColumn = 0;
    workerParams[count].rightColumn = 8;
  }

  int subGridCol, subGridRow;
  count = 18;

  for (subGridRow = 0; subGridRow<3; subGridRow++)
  {
    for (subGridCol = 0; subGridCol<3; subGridCol++)
    {
      workerParams[count].topRow = (subGridRow * 3);
      workerParams[count].bottomRow = (subGridRow * 3) + 2;
      workerParams[count].leftColumn = (subGridCol * 3);
      workerParams[count].rightColumn = (subGridCol * 3) + 2;
      count++;
    }
  }

  for (count = 0; count < NUM_THREADS; count++)
  {
    pthread_create(&(tid[count]), NULL, worker, &(workerParams[count]));
  }

  for (count = 0; count < NUM_THREADS; count++)
  {
    pthread_join(tid[count], NULL);
  }
  
  printf("\n");

  bool sudokuResult = TRUE;

  for (count = 0; count < 9; count++)
  {
    if (validity_columns[count] == FALSE) { sudokuResult = FALSE; }
    if (validity_rows[count] == FALSE) { sudokuResult = FALSE; }
    if (validity_subGrids[count] == FALSE) { sudokuResult = FALSE; }
  }

  for(count = 0; count<9; count++)
  {
    if (validity_columns[count] == TRUE) 
    { 
       printf("Column:  %lX   valid.\n", (unsigned long)tid_column[count]); 
    }else{
       printf("Column:  %lX   invalid.\n", (unsigned long)tid_column[count]);
    }
  }

  printf("\n");

  for(count = 0; count<9; count++)
  {
    if (validity_rows[count] == TRUE) { 
       printf("Row:  %lX   valid.\n", (unsigned long)tid_row[count]); 
    }else{
       printf("Row:  %lX   invalid.\n", (unsigned long)tid_row[count]);
    }
  }
  
  printf("\n");

  for(count = 0; count<9; count++)
  {
    if(validity_subGrids[count] == TRUE) { 
       printf("Subgrid:  %lX   valid.\n", (unsigned long)tid_subgrid[count]); 
    }else{
       printf("Subgrid:  %lX   invalid.\n", (unsigned long)tid_subgrid[count]);
    }
  }

  printf("\n");

  char* result = (sudokuResult == TRUE) ? "valid!" : "invalid!";
  printf("\n\nSudoku Puzzle: %s\n", result);

}

void generatePuzzleArray()
{
  char fileBuffer[82];
  const  int ROW_LENGTH = 9, COL_SUBGRID_LENGTH = 3, SUBGRID_ROW_QTY = 27, ZERO_INT_VALUE = 48;

  int i = 0;
  
  FILE *sudokuFile;
  sudokuFile = fopen("SudokuPuzzle.txt", "r");
  
  while(fscanf(sudokuFile, "%s", &fileBuffer[i]) == 1)
  {
    sudokuPuzzle[i / ROW_LENGTH][i % ROW_LENGTH] = fileBuffer[i] - ZERO_INT_VALUE;
    i++;
  }

  for(i = 0; i<81; i++)
  {
    if(i % ROW_LENGTH == 0) { printf("\n"); }
    if(i % SUBGRID_ROW_QTY == 0) { printf("\n"); }
    if(i % COL_SUBGRID_LENGTH == 0) { printf(" "); }
    printf("%d ", sudokuPuzzle[i / ROW_LENGTH][i % ROW_LENGTH]); 
  }
  
  printf("\n\n");

  fclose(sudokuFile);

}

void *worker(void *param)
{
  parameters *setupStruct;
  pthread_t self;

  setupStruct = (parameters *)param;

  self = pthread_self();

  int selfCheck[] = { 1,2,3,4,5,6,7,8,9 };
  int i, j, counter, found;
  int topCheck, bottomCheck, leftCheck, rightCheck;

  topCheck = setupStruct->topRow;
  bottomCheck = setupStruct->bottomRow;
  leftCheck = setupStruct->leftColumn;
  rightCheck = setupStruct->rightColumn;

  bool correctSudoku = FALSE;
  int foundNums = 0;
  for (i = topCheck; i <= bottomCheck; i++)
  {
    for (j = leftCheck; j <= rightCheck; j++)
    {
      for (counter = 0; counter < 9; counter++)
      {
        if (sudokuPuzzle[i][j] == selfCheck[counter])
        {
          foundNums++;
          selfCheck[counter] = 0;
        }
      }
    }
  }

  int gridNumber = 0;

  if (foundNums == 9)
  {
    correctSudoku = TRUE;
  }

  char* result = (correctSudoku == TRUE) ? "valid!" : "invalid!";

  if (topCheck == bottomCheck)
  {
      validity_rows[topCheck] = correctSudoku;
      tid_row[topCheck] = self;
  } else if (leftCheck == rightCheck) {
      validity_columns[leftCheck] = correctSudoku;
      tid_column[leftCheck] = self;
  } else {
      gridNumber += ((topCheck / 3) * 3);
      gridNumber += (leftCheck / 3);
      validity_subGrids[gridNumber] = correctSudoku;
      tid_subgrid[gridNumber] = self;
  }

  printf("%lX TRow: %d, BRow: %d, LCol: %d, RCol: %d %s \n"
    , (unsigned long)self, topCheck, bottomCheck, leftCheck, rightCheck, result);

  pthread_exit(0);

}
