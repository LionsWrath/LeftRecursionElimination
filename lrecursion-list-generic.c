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

#define NCHAR 100

#define LAMBDA 'e'
#define FINAL '$'
#define INITIAL 'S'

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

char * createString(unsigned int size) {
    char *str = malloc(size * sizeof(char));
    str[0] = '\0'; 

    return str;
}

void printRule(FILE *out, Rule *r) {
    fprintf(out, "      %c -> %s\n", r->l, r->prod);
}

//------------------------------------------------------------Data Structure

typedef struct Element {
    void *data;
    void *next;
} Element;

typedef struct Queue {
    Element *begin, *end;
    unsigned int size;
    void (*deallocator)(void*);
} Queue;

Queue * initializeQueue(void (*deallocator)(void*)) {
    Queue * ptr = malloc(sizeof(Queue));
    ptr->begin = NULL;
    ptr->end = NULL;
    ptr->size = 0;
    ptr->deallocator = deallocator;

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

// This function cannot add on the beginning
// No need to adjust the begin then (this is bad, actually)
bool pushToPosition(Queue *q, void *d, void *pos) {
    Element *e = createElement(d);
    Element *t = getElement(q, pos);

    if (t == NULL) return false;
    if (t->next == NULL) {
        t->next = e;
        e->next = NULL;
        
        q->end = e;
    } else {
        e->next = t->next;
        t->next = e;
    }

    q->size++;
    return true;
}

// Only free the data if exists a deallocator
bool erase(Queue *q, void *d) {
    if (isQueueEmpty(q)) return false;
    
    Element *behind = NULL;
    Element *element = q->begin; 
    while (element != NULL) {
        if (element->data != d) {
            behind = element;
            element = element->next;
        } else {
            // Running the deallocator
            if (q->deallocator != NULL) 
                q->deallocator(element->data);
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
    return true;
}

// High chance of memory leak
// Can receive a deallocator for the data
void clearQueue(Queue *q) {
    Element *e = q->begin;
    Element *next;

    while (e != NULL) {
        next = e->next;

        if (q->deallocator != NULL) q->deallocator(e->data);

        free(e);
        e = next;
    }
    q->begin = NULL;
    q->end = NULL;
    q->size = 0;
}

// This clear the src Queue, as it makes no sense to not do it
// All values data from src would be invalid otherwise
bool eraseBasedOnQueue(Queue *q, Queue *src) {
    bool changed = true;
    
    Element *e = src->begin;
    while (e != NULL) {
        bool result = erase(q, e->data);
        changed = changed && result;
        e = e->next;
    }
    
    clearQueue(src);
    return changed;
}

//-----------------------------------------------------------FREE FUNCTIONS

// deallocator of Queue
void freeQueue(Queue *q) {
    clearQueue(q);
    free(q);
}

// deallocator of Rule
void freeRule(void *rule) {
    Rule *r = (struct Rule*) rule;
    free(r->prod);
    free(rule);
}

// deallocator of Number
void freeNumber(void *number) {
    free(number);
}

//-----------------------------------------------------------SOME VARIABLES

Queue *g;
Queue *order; 
Queue *availableNT;

char const allNonTerminals[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

bool hasFinal = false;

//-------------------------------------------------------------------READ

void printGrammar(FILE *out, Queue *grammar) {

    fprintf(out, "  Number of Rules: %u\n", grammar->size);

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

// Need to free others values after
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

            char *production = createString(NCHAR);
            strcpy(production, token);

            push(q, createRule(actual, production));
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

    fprintf(out, "\nUsed Order: ");

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

// This sets the maximum available NT as 26
void populateAvailableNT(Queue * available, Queue *order) {
    int i;
    int size = strlen(allNonTerminals);

    for (i=0; i<size; i++) {
        if (!numberExists(order, allNonTerminals[i]))
            push(available, createNumber(allNonTerminals[i])); 
    }
}

// this already sets the NT as used
char getUnusedNonTerminal(Queue * available) {
    if (isQueueEmpty(available)) {
        fprintf(stderr, "Fora do número máximo de Não Terminais, abortado.\n");   
        exit(EXIT_FAILURE);
    }
    
    Number *n = available->begin->data;
    char c = n->value;
    erase(available, n);

    return c;
}

bool isEProduction(Rule *rule) {
    if (rule->prod[0] == LAMBDA) return true;
    return false;
}

bool hasFinalSymbol(Rule *rule) {
    int size = strlen(rule->prod);
    if (rule->prod[size - 1] == FINAL) return true;
    return false;
}

void eliminateFinalSymbol(Rule *rule) {
    int size = strlen(rule->prod);
    rule->prod[size - 1] = '\0';
}

void createFinalSymbol(Rule *rule) {
    int size = strlen(rule->prod);
    rule->prod[size] = FINAL;
    rule->prod[size + 1] = '\0';
}

bool preserveFinalSymbols(Queue *grammar, unsigned int initial) {
    Element *e = grammar->begin;

    while (e != NULL) {
        Rule *r = e->data;

        if (hasFinalSymbol(r) && r->l == initial)
            eliminateFinalSymbol(r);
        if (hasFinalSymbol(r) && r->l != initial) {
            fprintf(stderr, "Símbolo final em produção não inicial! Abortado.");
            exit(EXIT_FAILURE);
        }

        e = e->next;
    }
}

void putFinalSymbols(Queue *grammar, unsigned int initial) {
    Element *e = grammar->begin;

    while (e != NULL) {
        Rule *r = e->data;

        if (r->l == initial)
           createFinalSymbol(r); 

        e = e->next;
    }
}

Queue * findBetas(Queue *grammar, Queue *beta, Number *nl) {
    Element *e = grammar->begin;

    while (e != NULL) {
        Rule *r = (struct Rule*) e->data;
        
        if (r->l == nl->value) 
	    if (nl->value != r->prod[0]) {
                push(beta, r);
	    }
        e = e->next;
    }

    return beta;
}

Queue * findAlphas(Queue *grammar, Queue *alpha, Number *nl) {
    Element *e = grammar->begin;

    while (e != NULL) {
        Rule *r = (struct Rule*) e->data;     
        if (r->l == nl->value) {
            // Eliminating A->A
            if (nl->value == r->prod[0] && r->prod[1] == '\0') {
                erase(grammar, r);
            } 
	    if (nl->value == r->prod[0]) {
                push(alpha, r);
	    }
        }
        e = e->next;
    }

    return alpha;
}

void appendChar(char *str, char c) {
    int size = strlen(str);
    
    str[size] = c;
    str[size + 1] = '\0';
}

Queue * addBetas(Queue *grammar, Queue *beta, char newNT, Rule *pos) {
    char *newProduction;

    Element *e = beta->begin;
    while (e != NULL) {
        Rule *r = e->data;
        newProduction = createString(NCHAR);

        if (!isEProduction(r))
            strcpy(newProduction, r->prod);
        appendChar(newProduction, newNT); 

        Rule *newRule = createRule(pos->l, newProduction);
        pushToPosition(grammar, newRule, pos); 

        pos = newRule;
        e = e->next;
    }
}

Queue * addAlphas(Queue *grammar, Queue *alpha, char newNT, Rule *pos) {
    char *newProduction;

    Element *e = alpha->begin;
    while (e != NULL) {
        Rule *r = e->data;
        newProduction = createString(NCHAR);

        strcpy(newProduction, r->prod + 1);
        appendChar(newProduction, newNT); 

        Rule *newRule = createRule(newNT, newProduction);
        pushToPosition(grammar, newRule, pos);

        pos = newRule;
        e = e->next;
    }
}

Queue * addEProduction(Queue *grammar, char newNT, Rule *pos) {
    char *eProduction;
    
    eProduction = createString(NCHAR);

    eProduction[0] = LAMBDA;
    eProduction[1] = '\0'; 

    pushToPosition(grammar, createRule(newNT, eProduction), pos);
}

// Add abailableNT as parameter after
void imediateElimination(Queue *grammar, Rule *rule) {
    Queue *betaRules = initializeQueue(NULL);
    Queue *alphaRules = initializeQueue(NULL);
    
    Element *e = grammar->begin;
    while (e != NULL) {
        
        Rule *r = (struct Rule*) e->data;     
        if (r->l == r->prod[0] && r->l == rule->l) {
            findBetas(grammar, betaRules, e->data);
            findAlphas(grammar, alphaRules, e->data); 
          
            char newNT = getUnusedNonTerminal(availableNT);
            
            addEProduction(grammar, newNT, r);
            addAlphas(grammar, alphaRules, newNT, r);
            addBetas(grammar, betaRules, newNT, r); 

            eraseBasedOnQueue(grammar, betaRules);
            eraseBasedOnQueue(grammar, alphaRules);
        }
        e = e->next;
    }
   
    freeQueue(betaRules);
    freeQueue(alphaRules);
}

void substituteMatches(Queue *grammar, Queue *matches, Number *n) {
    char *newProduction;     
    Queue *newRules = initializeQueue(NULL);

    // For each match
    Element *e = matches->begin;
    while (e != NULL) {
        Rule *m = e->data;

        // Find all production of n in the grammar
        Element *t = grammar->begin;
        while (t != NULL) {
            Rule *r = t->data;
            if (r->l == n->value) {
                newProduction = createString(NCHAR);

                strcpy(newProduction, r->prod);
                strcat(newProduction, m->prod + 1);

                push(newRules, createRule(m->l, newProduction));
            }

            t = t->next;
        }

        appendToPosition(grammar, newRules, m);
        clearQueue(newRules);
        
        erase(grammar, m);
        e = e->next; 
    }

    freeQueue(newRules);
}

Queue * findAllMatches(Queue *grammar, Queue *matches, Number *nl, Number *nr) {
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

void globalElimination(Queue *grammar, Queue *order) {
    Element *e = order->begin;
    Queue *matches = initializeQueue(NULL); 

    while (e != NULL) {
        Element *t = order->begin;
        while (t != e) {
            findAllMatches(grammar, matches, e->data, t->data);
            
            if (!isQueueEmpty(matches)) { 
                substituteMatches(grammar, matches, t->data);
            }

            imediateElimination(grammar, e->data);
            clearQueue(matches);
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
 
    // Load Grammar
    g = initializeQueue(freeRule);
    readGrammar(filename, g);
    
    fprintf(out, "Original Grammar:\n\n");
    printGrammar(out, g);

    preserveFinalSymbols(g, INITIAL);

    // Create the order and list of available NT
    order = initializeQueue(freeNumber);
    availableNT = initializeQueue(freeNumber);
    
    orderNT(g, order);
    populateAvailableNT(availableNT, order);
    
    printOrder(out, order);

    fprintf(out, "After removing:\n\n");

    globalElimination(g, order);

    putFinalSymbols(g, INITIAL);

    printGrammar(out, g);

    freeQueue(g);
    freeQueue(order);

    return 0;
}
