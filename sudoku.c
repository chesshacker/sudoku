#include <stdio.h>
#include <assert.h>

typedef struct square_t square_t;
typedef struct puzzle_t puzzle_t;

#define  ALL_POSSIBLE  0x01FF

struct square_t {
   char rn;
   char cn;
   char value;
   char pcount;
   unsigned int pflag;
   
   square_t** row;
   square_t** col;
   square_t** box;
};

struct puzzle_t {
   square_t sqr[9][9];
   square_t* row[9][10];
   square_t* col[9][10];
   square_t* box[9][10];
};

static unsigned int nums[9];

#define num2char(n)   ((n)==0)?'-':((n)+'0')

void solve(puzzle_t* p, int useTrial, int quiet);

void initStatic() {
   int i;
   
   for (i=0; i<9; i++)
      nums[i] = 1<<i;
}

void initPuzzle(puzzle_t* p) {
   int i, j, br, bc;
   
   for (i=0; i<9; i++) {
      for (j=0; j<9; j++) {
         p->sqr[i][j].value = 0;
         p->sqr[i][j].rn = i;
         p->sqr[i][j].cn = j;
         p->sqr[i][j].pcount = 9;
         p->sqr[i][j].pflag = ALL_POSSIBLE;
         
         p->row[i][j] = &p->sqr[i][j];
         p->col[j][i] = &p->sqr[i][j];
         br = (i/3)+(j/3)*3; 
         bc = (i%3)*3+(j%3);
         p->box[br][bc] = &p->sqr[i][j];
         
         p->sqr[i][j].row = &p->row[i][0];
         p->sqr[i][j].col = &p->col[j][0];
         p->sqr[i][j].box = &p->box[br][0];
      }
      p->row[i][9] = NULL;
      p->col[i][9] = NULL;
      p->box[i][9] = NULL;
   }
}

void updateGroup_NoBlankAllowed(square_t** group, char value) {
   int i;
   square_t* aSqr;
   unsigned int flag = nums[value-1];
   unsigned int flagc = (~flag & ALL_POSSIBLE);
   
   for (i=0; i<9; i++) {
      aSqr = group[i];
      if ((aSqr->pflag & flag)!=0) {
         aSqr->pflag = aSqr->pflag & flagc;
         aSqr->pcount--;
      }
   }
}

void updateGroups_NoBlankAllowed(square_t* sqr, char value) {
   updateGroup_NoBlankAllowed(sqr->row, value);
   updateGroup_NoBlankAllowed(sqr->col, value);
   updateGroup_NoBlankAllowed(sqr->box, value);
}

void updateSquare_NoBlankAllowed(square_t* sqr, char value) {
   unsigned int flag = nums[value-1];
   unsigned int flagc = (~flag & ALL_POSSIBLE);
   
   if ((sqr->pflag & flag)!=0) {
      sqr->pflag = sqr->pflag & flagc;
      sqr->pcount--;
   }
}

void updatePossible(puzzle_t* p, int i, int j, char value) {
   p->sqr[i][j].value = value;
   p->sqr[i][j].pcount = 0;
   p->sqr[i][j].pflag = 0x0;
   updateGroups_NoBlankAllowed(&p->sqr[i][j], value);
}

void copyPuzzle(puzzle_t* cp, puzzle_t* op) {
   int i, j;
   char val;
   
   initPuzzle(cp);
   for (i=0; i<9; i++) {
      for (j=0; j<9; j++) {
         val = op->sqr[i][j].value;
         if (val > 0)
            updatePossible(cp,i,j,val);
      }
   }
}

void readPuzzle(FILE* stream, puzzle_t* p) {
   int i, j;
   char c;
   
   for (i=0; i<9; i++) {
      for (j=0; j<9; j++) {
         c = getc(stream);
         c = (c=='-') ? 0 : (c - '0');
         assert(c >= 0 && c <= 9);
         if (c > 0)
            updatePossible(p,i,j,c);
      }
      assert(getc(stream)=='\n');
   }
}

void printPuzzle(puzzle_t* p) {
   int i, j;
   
   for (i=0; i<9; i++) {
      for (j=0; j<9; j++) {
         putc(num2char(p->sqr[i][j].value),stdout);
      }
      putc('\n',stdout);
   }   
}

/*
void printPossible(int i, int j) {
   square_t** sqPtr;
   int k;
   
   sqPtr = sqr[i][j].row;
   printf("square (%d, %d):\n  row: ", i, j);
   for(k=0; k<9; k++) {
      putc( num2char((*sqPtr)->value), stdout);
      sqPtr++;
   }
   sqPtr = sqr[i][j].col;
   printf("\n  col: ");
   for(k=0; k<9; k++) {
      putc( num2char((*sqPtr)->value), stdout);
      sqPtr++;
   }
   sqPtr = sqr[i][j].box;
   printf("\n  box: ");
   for(k=0; k<9; k++) {
      putc( num2char((*sqPtr)->value), stdout);
      sqPtr++;
   }
   printf("\n  possible: ");
   for(k=0; k<9; k++) {
      if((sqr[i][j].pflag & nums[k])!=0)
         putc(num2char(k+1), stdout);
      else
         putc('-',stdout);
   }
   putc('\n',stdout);
}
*/

int solveEasyOnes(puzzle_t* p, int quiet) {
   int i, j, result = 0, cnt=1;
   
   while (cnt>0) {
      cnt = 0;
      for (i=0; i<9; i++) {
         for (j=0; j<9; j++) {
            if (p->sqr[i][j].pcount==1) {
               char val = 1;
               unsigned int flag = p->sqr[i][j].pflag;
               while (flag >>= 1)
                  val++;  // val = log2(flag)
               updatePossible(p,i,j,val);
               if (!quiet)
                  printf("set sqr[%d,%d]=%d (only value possible)\n",i,j,val);
               cnt++;
            }
         }
      }
      result += cnt;
   }
   return result;
}

square_t* findSingle(int value, square_t** group) {
   square_t* result=NULL;
   unsigned int flag = nums[value-1];
   
   while (*group != NULL) {
      if (((*group)->pflag & flag)!=0) {
         if (result==NULL) 
            result = *group;
         else
            return NULL;
      }
      group++;
   }
   return result;
}

int solveForNumbers(puzzle_t* p, int quiet) {
   int i,j,result=0;
   
   for (i=1; i<=9; i++) { // i = number searching for
      for (j=0; j<9; j++) { // j = row#, col#, box#
         square_t* aSqr;
         if ((aSqr = findSingle(i,&p->row[j][0])) != NULL) {
            if (!quiet)
               printf("set sqr[%d,%d]=%d (only space for number in row)\n",
                   aSqr->rn, aSqr->cn, i);
            updatePossible(p,aSqr->rn,aSqr->cn,i);
            result++;
         }
         if ((aSqr = findSingle(i,&p->col[j][0])) != NULL) {
            if (!quiet)
               printf("set sqr[%d,%d]=%d (only space for number in column)\n",
                   aSqr->rn, aSqr->cn, i);
            updatePossible(p,aSqr->rn,aSqr->cn,i);
            result++;
         }
         if ((aSqr = findSingle(i,&p->box[j][0])) != NULL) {
            if (!quiet)
               printf("set sqr[%d,%d]=%d (only space for number in box)\n",
                   aSqr->rn, aSqr->cn, i);
            updatePossible(p,aSqr->rn,aSqr->cn,i);
            result++;
         }
      }
   }
   return result;
}

int isValid(puzzle_t* p) {
   int i, j;
   
   for (i=0; i<9; i++) {
      for (j=0; j<9; j++) {
         if (p->sqr[i][j].pcount=='\0' && p->sqr[i][j].value=='\0')
            return 0;
      }
   }
   return 1;
}

int solveByTrialError(puzzle_t* orig, int trialNumber, int quiet) {
   int i, j, cnt=0;
   puzzle_t p;
   square_t* aSqr;
   
   trialNumber--;
   copyPuzzle(&p,orig);
   for (i=0; i<9; i++) {
      for (j=0; j<9; j++) {
         aSqr = &p.sqr[i][j];
         if (aSqr->pcount > 1) {
            char val = 0;
            unsigned int flag = aSqr->pflag;
            do {
               while ((flag & 0x1)==0) {
                  flag >>= 1;
                  val++;
               }
               flag >>= 1;
               val++;
               updatePossible(&p,i,j,val);
               solve(&p,trialNumber,1);
               if (!isValid(&p)) {
                  if (!quiet)
                     printf("test sqr[%d,%d]=%d FAILED!\n",i,j,val);
                  updateSquare_NoBlankAllowed(&orig->sqr[i][j], val);
                  cnt++;
               }
               copyPuzzle(&p,orig);
            } while (flag >= 1);
         }
         if (cnt > 0)
            return cnt;
      }
   }
   return cnt;
}

void solve(puzzle_t* p, int useTrialNumber, int quiet) {
   int cnt = 1;
   
   while (cnt > 0) {
      cnt = 0;
      cnt += solveEasyOnes(p, quiet);
      cnt += solveForNumbers(p, quiet);
      if (cnt==0 && useTrialNumber>0)
         cnt += solveByTrialError(p,useTrialNumber, quiet);
   }
}

int main (int argc, const char * argv[]) {
   puzzle_t puzzle;
   FILE* inFile;
   
   inFile = fopen("puzzle.txt","r");
   initStatic();
   initPuzzle(&puzzle);
   readPuzzle(inFile, &puzzle);
   solve(&puzzle,1,0);
   printPuzzle(&puzzle);
   return 0;
}
