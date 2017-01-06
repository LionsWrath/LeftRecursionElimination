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

#define NCHAR 100

#define SETSIZE 100

#define LAMBDA 'e'
#define FINAL '$'
#define START 'S'

#define ERROR -1

//------------------------------------------------------------Data Structure

typedef struct Rule {
    unsigned int l;
    char *prod;
    struct Rule *next;
} Rule;

typedef struct Queue {
    Rule *begin, *end;
    unsigned int size;
} Queue;

bool isQueueEmpty(Queue *q) {
    if (q->begin == NULL) return true;
    return false;
}

bool exists(Queue *q, Rule *r) {
    if (isQueueEmpty(q)) return false;
    
    Rule *rule = q->begin;
    while (rule != NULL) {
        if (rule != r) rule = rule->next;
        else return true;
    }
    return false;
}

bool leftExists(Queue *q, Rule *r) {
    if (isQueueEmpty(q)) return false;
    
    Rule *rule = q->begin;
    while (rule != NULL) {
        if (rule->l != r->l) rule = rule->next;
        else return true;
    }
    return false;
}

Queue * initializeQueue() {
    Queue * ptr = malloc(sizeof(Queue));
    ptr->begin = NULL;
    ptr->end = NULL;
    ptr->size = 0;
    
    return ptr;
}

void push(Queue *q, Rule *r) {
    if (q->begin == NULL) {
        q->begin = r;
        q->end = r;
        q->end->next = NULL;                //Pode omitir isso se deixar null na criação de uma regra
        q->size++;
        return;
    }
    q->end->next = r;
    q->end = r;
    q->end->next = NULL;
    q->size++;
}

// Push a left only rule with no prod - without repetition
void insertLeftOnly(Queue *q, Rule *r) {
    if (!leftExists(q, r))
        push(q, r);
}

bool erase(Queue *q, Rule *r) {
    if (isQueueEmpty(q)) return false;
    
    Rule *behind = NULL;
    Rule *rule = q->begin; 
    while (rule != NULL) {
        if (rule != r) {
            behind = rule;
            rule = rule->next;
        } else {
            if (rule == q->begin) {
                q->begin = q->begin->next;
                // Free aqui ou nem - pergunta
                return true;
            }
            if (rule == q->end) {
                q->end = behind;
                q->end->next = NULL; 
                return true;
            }
            behind->next = rule->next;
            r->next = NULL;
            return true;
        }
    }
    return false;
} 

Rule * createRule(unsigned int left, char production[]) {
    Rule * ptr = malloc(sizeof(Rule));
    ptr->l = left;
    ptr->prod = production;
    ptr->next = NULL;
    
    return ptr;
}

void printRule(FILE *out, Rule *r) {
    fprintf(out, "  %c -> %s\n", r->l, r->prod);
}

//-----------------------------------------------------------SOME VARIABLES

Queue *g;
Queue *order; // Use the queue itself as the order, null on prod

//-------------------------------------------------------------------MEMORY

//-------------------------------------------------------------------READ

void printGrammar(FILE *out, Queue *grammar) {

    fprintf(out, "Number of Rules: %u\n", grammar->size);

    Rule *r = grammar->begin;    
    while (r != NULL) {
        printRule(out, r);
        r = r->next;
    }
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

void readGrammar(char *filename, Queue *q) {
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
            //Remove trailing newline
            size_t idx = strcspn(token, "\n");
            memmove(&token[idx], &token[idx + 1], strlen(token) - idx);
            
            push(q, createRule(actual, token));
        }
    }

    free(line);
    fclose(input);
}

//------------------------------------------------------------------ELIMINATION

void printOrder(FILE *out, Queue *order) {

    fprintf(out, "\nOrder: ");

    Rule *r = order->begin;
    while (r != NULL) {
        fprintf(out, "%c ", r->l);
        r = r->next;
    }
    fprintf(out, "\n");
}

void orderNT(Queue *q, Queue *orderSet) {
    int i;

    Rule *r = q->begin;
    while (r != NULL) {
        insertLeftOnly(orderSet, createRule(r->l, NULL));
        r = r->next;
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
    g = initializeQueue();
    readGrammar(filename, g);
    printGrammar(out, g);

    order = initializeQueue();
    orderNT(g, order);
    printOrder(out, order);
    
    return 0;
}
