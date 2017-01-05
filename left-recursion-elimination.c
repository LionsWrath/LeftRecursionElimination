//--------------------------------------------------------------------------
//    Adriano Ferrari Cardoso         RA: 77274
//    Andrey Souto Maior              RA: 78788
//    Caio Henrique Segawa Tonetti    RA: 79064
//    Lucas Franco Bernardes          RA: 80824
//--------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <ctype.h>

#define NCHAR 15
#define NRULE 40

#define SETSIZE 100

#define LAMBDA 'e'
#define FINAL '$'
#define START 'S'

#define ERROR -1

//Grammar Related Sets
char *lRules, **rRules;
char *nlRules, **nrRules;
size_t numRules = 0;

char *ordering;
size_t numNT = 0;

//-------------------------------------------------------------------MEMORY

void allocDataset(size_t nlines, size_t nchar) {
    int i;
    lRules = malloc(nlines * sizeof(char));
    rRules = malloc(nlines * sizeof(char*));
    for (i=0; i<nlines; i++)
        rRules[i] = malloc(nchar * sizeof(char)); 
}

void allocRemoved(size_t nlines, size_t nchar) {
    int i;
    nlRules = malloc(nlines * sizeof(char));
    nrRules = malloc(nlines * sizeof(char*));
    for (i=0; i<nlines; i++)
        nrRules[i] = malloc(nchar * sizeof(char)); 
}

void allocOrder(size_t nlines) {
    ordering = malloc(nlines * sizeof(char));
    ordering[0] = '\0';
}

//-------------------------------------------------------------------READ

void printGrammar(FILE *out) {
    int i;
    fprintf(out, "Number of Rules: %zu\n", numRules);
    for (i=0; i<numRules; i++) 
        fprintf(out, "    %c -> %s\n", lRules[i], rRules[i]); 
}

char* cleanString(char str[], char ch) {
   int i, j = 0, size;
   char *newStr, ch1;

   size = strlen(str);
   newStr = malloc(size * sizeof(char));

   for (i = 0; i < size; i++) {
      if (str[i] != ch) {
         ch1 = str[i];
         newStr[j] = ch1;
         j++;
      }
   }
   newStr[j] = '\0';

   return newStr;
}

void readGrammar(char *filename) {
    FILE *input;
    char *line = NULL, *token = NULL;
    size_t len = 0;
    ssize_t read;

    if ((input = fopen(filename, "r")) == NULL) {
        fprintf(stderr, "Não foi possível abrir o arquivo da gramática!\n");
        exit(EXIT_FAILURE);
    } 
    
    while ((read = getline(&line, &len, input)) != -1) {
        line = cleanString(line, ' ');
        char actual = line[0];
        line = &line[2];
        while ((token = strsep(&line, "|")) != NULL) {
            lRules[numRules] = actual;
          
            //Remove trailing newline
            size_t idx = strcspn(token, "\n");
            memmove(&token[idx], &token[idx + 1], strlen(token) - idx);
            
            strcpy(rRules[numRules], token);
            
            numRules++;
        }
    }

    free(line);
    fclose(input);
}

//------------------------------------------------------------------ELIMINATION

bool isTerminal(char symbol) {
    return !isupper(symbol);
}

bool isEmpty(char set[]) {
    if (strlen(set) == 0) return true;
    return false;
}

bool addToSet(char set[], char symbol) {
    int i;

    //Verify if already exists
    for(i=0; set[i] != '\0'; i++)
        if(set[i] == symbol) return false;
            
    set[i] = symbol;
    set[i+1] = '\0';

    return true;
}

void printOrder(FILE *out) {
    int i;

    fprintf(out, "Order: ");
    for (i=0; i<numNT; i++) {
        fprintf(out, "%c ", ordering[i]);
    }
    fprintf(out, "\n");
}

void orderNT(char orderSet[]) {
    int i;

    for (i=0;i<numRules; i++) {
        if (addToSet(orderSet, lRules[i])) 
            numNT++;        
    }
}

void imediateRemoval() {

}

void globalRemoval() {
    int i;

    for (i=0; i<numNT; i++) {
        // fase de substituição
        imediateRemoval();
    }
}

int main(int argc, char *argv[]) {
    char filename[100], output[100];
    int oc, i;
    FILE *out = stdout;

    while ((oc = getopt(argc, argv, "f:o:")) != -1) {
        switch (oc) {
            case 'f':
                strcpy(filename, optarg);
                break;
            case 'o':
                strcpy(output, optarg);
                if ((out = fopen(output, "w")) == NULL) {
                    fprintf(stderr, "Não foi possível abrir o arquivo de resultado!\n");
                    exit(EXIT_FAILURE);
                } 
                break;
            case '?':
            default:
                fprintf(stderr, "Sem arquivo de entrada.");
                exit(EXIT_FAILURE);
                break;
        }
    }
 
    //Load Grammar
    allocDataset(NRULE, NCHAR);
    readGrammar(filename);
    printGrammar(out);

    allocOrder(NRULE);
    orderNT(ordering);
    printOrder(out);

    return 0;
}
