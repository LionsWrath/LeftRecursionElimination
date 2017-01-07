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
//----------------------------------------------------------------------Data

typedef struct Rule {
    unsigned int l;
    char *prod;
} Rule;

typedef struct Number {
    unsigned int value;
} Number;

Rule * createRule(unsigned int left, char production[]) {
    Rule *ptr = malloc(sizeof(Rule));
    ptr->l = left;
    ptr->prod = production;
    
    return ptr;
}

Number * createNumber(unsigned int value) {
    Number *ptr = malloc(sizeof(Number));
    ptr->value = value;

    return ptr;
}

void printRule(FILE *out, Rule *r) {
    fprintf(out, "  %c -> %s\n", r->l, r->prod);
}

//------------------------------------------------------------Data Structure

typedef struct Element {
    void *data;
    void *next;
} Element;

typedef struct Queue {
    Element *begin, *end;
    unsigned int size;
} Queue;

Queue * initializeQueue() {
    Queue * ptr = malloc(sizeof(Queue));
    ptr->begin = NULL;
    ptr->end = NULL;
    ptr->size = 0;
    
    return ptr;
}

Element * createElement(void *data) {
    Element *ptr = malloc(sizeof(Element));
    ptr->data = data;
    ptr->next = NULL;

    return ptr;
}

bool isQueueEmpty(Queue *q) {
    if (q->begin == NULL) return true;
    return false;
}

bool exists(Queue *q, void *d) {
    if (isQueueEmpty(q)) return false;
    
    Element *element = q->begin;
    while (element != NULL) {
        if (element->data != d) element = element->next;
        else return true;
    }
    return false;
}

// Testing
Element * getElement(Queue *q, void *d) {
    if (isQueueEmpty(q)) return NULL;
    
    Element *element = q->begin;
    while (element != NULL) {
        if (element->data == d) return element;
        element = element->next;
    }
    return NULL;
}

void push(Queue *q, void *d) {
    Element *element = createElement(d);

    if (isQueueEmpty(q)) {
        q->begin = element;
        q->end = element;
        q->size++;
        return;
    }  

    q->end->next = element;
    q->end = element;
    q->size++;
}

// Testing
bool pushToPosition(Queue *q, void *d, void *pos) {
    Element *e = createElement(d);
    Element *t = getElement(q, pos);

    if (t == NULL) return false;
    if (t->next == NULL) {
        t->next = e;
        e->next = NULL;
    } else {
        e->next = t->next;
        t->next = e;
    }
    return true;
}

// Dá free no element, não no dado
bool erase(Queue *q, void *d) {
    if (isQueueEmpty(q)) return false;
    
    Element *behind = NULL;
    Element *element = q->begin; 
    while (element != NULL) {
        if (element->data != d) {
            behind = element;
            element = element->next;
        } else {
            if (element == q->begin) {
                q->begin = q->begin->next;
                q->size--;
                free(element);
                return true;
            }
            if (element == q->end) {
                q->end = behind;
                q->end->next = NULL; 
                q->size--;
                free(element);
                return true;
            }
            behind->next = element->next;
            q->size--;
            free(element);
            return true;
        }
    }
    return false;
}

// Dont free the src Queue
void append(Queue *dst, Queue *src) { 
    Element *e = src->begin;
    while (e != NULL) {
        push(dst, e->data); 
        e = e->next; 
    }
}

// Dont free the src Queue
bool appendToPosition(Queue *dst, Queue *src, void *pos) {
    Element *e = src->begin;
    while (e != NULL) {
        if (!pushToPosition(dst, e->data, pos)) return false; 
        pos = e->data; // To put in the correct order
        e = e->next; 
    }
}

// Dont free the data, just the elements
// High chance of memory leak
void clearQueue(Queue *q) {
    Element *e = q->begin;
    Element *next;
    while (e != NULL) {
        next = e->next;
        free(e);
        e = next;
    }

    q->begin = NULL;
    q->end = NULL;
}
//-----------------------------------------------------------SOME VARIABLES

Queue *g;
Queue *order; 

//-------------------------------------------------------------------READ

void printGrammar(FILE *out, Queue *grammar) {

    fprintf(out, "Number of Rules: %u\n", grammar->size);

    Element *e = grammar->begin;    
    while (e != NULL) {
        printRule(out, e->data);
        e = e->next;
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

//----------------------------------------------------------------------ORDER

bool numberExists(Queue *q, unsigned int value) {
 
    Element *e = order->begin;
    while (e != NULL) {
        Number *n = (struct Number*) e->data;
        if (n->value == value) return true;
        
        e = e->next;
    }
    return false; 
}

void insertNumber(Queue *q, unsigned int value) {
    if (!numberExists(q, value))
        push(q, createNumber(value));
}

void printOrder(FILE *out, Queue *order) {

    fprintf(out, "\nOrder: ");

    Element *e = order->begin;
    while (e != NULL) {
        Number *n = (struct Number*) e->data;
        fprintf(out, "%c ", n->value);
        e = e->next;
    }
    fprintf(out, "\n");
}

void orderNT(Queue *q, Queue *orderSet) {
    int i;

    Element *e = q->begin;
    while (e != NULL) {
        Number *n = (struct Number*) e->data;
        insertNumber(orderSet, n->value);
        e = e->next;
    }
}

//-----------------------------------------------------------------ELIMINATION

Queue * findAllMatches(Queue *grammar, Number *nl, Number *nr) {
    Queue *matches = initializeQueue();

    fprintf(stderr, "Ai = %c, Aj = %c\n", nl->value, nr->value);
    Element *e = grammar->begin;
    while (e != NULL) {
        Rule *r = (struct Rule*) e->data;     
        if (r->l == nl->value) 
           if (r->prod[0] == nr->value)
               push(matches, r);
        e = e->next;
    }

    return matches;
}

void imediateElimination(Queue *grammar) {

}

void substituteMatches(Queue *grammar, Queue *matches, Number *n) {
    char *newProduction;     
    Queue *newRules = initializeQueue();

    // Para cada match
    Element *e = matches->begin;
    while (e != NULL) {
        Rule *m = e->data;

        // Achar todas as produções de n na gramática
        Element *t = grammar->begin;
        while (t != NULL) {
            Rule *r = t->data;
            if (r->l == n->value) {
                newProduction = malloc(NCHAR * sizeof(char));
                newProduction[0] = '\0';

                strcpy(newProduction, r->prod);
                strcat(newProduction, m->prod + 1);

                fprintf(stdout, "           Prod: %s\n", newProduction);

                // Adicionar nova regra na gramatica
                push(newRules, createRule(m->l, newProduction));
            }

            t = t->next;
        }

        // Unite the newRules to the grammar
        appendToPosition(grammar, newRules, m);
        clearQueue(newRules); // Clear the newRules
        erase(grammar, m);  // Eliminar match da gramática
        // Free the matched rule - memory leak otherwise
        e = e->next; 
    }

    free(newRules);
    // Free the Queue
}

void globalElimination(Queue *grammar, Queue *order) {
    Element *e = order->begin;

    while (e != NULL) {
        Element *t = order->begin;
        while (t != e) {
            // Find all Matches
            // Change to an external match initialization
            // But internal clearing - memory leak on multiple Queue creations
            Queue *matches = findAllMatches(grammar, e->data, t->data);
            
            if (!isQueueEmpty(matches)) { 
                printGrammar(stdout, matches);
                // Substituir matches por todas as produções de Aj
                substituteMatches(grammar, matches, t->data);
            }

            // Imediate left recursion elimination
            imediateElimination(grammar);   
            // Free the Queue "matches" structure
            t = t->next;
        }
        e = e->next;   
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

    globalElimination(g, order);
    printGrammar(out, g);

    return 0;
}