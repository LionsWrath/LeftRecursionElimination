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

char * createString(unsigned int size) {
    char *str = malloc(size * sizeof(char));
    str[0] = '\0'; 

    return str;
}

char * newNonTerminal(unsigned int n) {
	char *newProduction;
	char *nonT;

	newProduction = createString(NCHAR);
	nonT = createString(NCHAR);

	if (n > 255) {
		sprintf(nonT, "%c", n);

		strcpy(newProduction, nonT);
		strcat(newProduction, "`");
	}

	return newProduction;
}

void printRule(FILE *out, Rule *r) {
	if (r->l > 255) {
		char *newProduction = newNonTerminal(r->l);
		fprintf(out, "  %s -> %s\n", newProduction, r->prod);
	} else
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

	int i = -1;
    while (e != NULL) {
		i++;
		//fprintf(stdout, "percorrendo a grammar, iteracao: %i\n", i);
        next = e->next;

        if (q->deallocator != NULL) q->deallocator(e->data);

        free(e);
        e = next;
    }
    q->begin = NULL;
    q->end = NULL;
    q->size = 0;
}

//-----------------------------------------------------------FREE FUNCTIONS

// Implement this - verify
void freeQueue(Queue *q) {
    clearQueue(q);
    free(q);
}

// deallocator of Rule
void freeRule(void *rule) {
    Rule *r = (struct Rule*) rule;
    free(r->prod); // Estava dando problema por causa do strsep
    free(rule);
}

// deallocator of Number
void freeNumber(void *number) {
    free(number);
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

            // Verify if need to allocate a new string for each struct            
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

Queue * findAllMatches(Queue *grammar, Queue *matches, Number *nl, Number *nr) {
    //fprintf(stderr, "Ai = %c, Aj = %c\n", nl->value, nr->value);
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

Queue * findBeta(Queue *grammar, Queue *beta, Number *nl, char *prod) {
	//fprintf(stderr, "findBeta -> Ai = %c(%i), Prod = %s\n", nl->value, nl->value, prod);
	Element *e = grammar->begin;

    while (e != NULL) {
        Rule *r = (struct Rule*) e->data;     
        if (r->l == nl->value) 
        	//if (r->prod[0] == nr->value)
			if (prod != r->prod && prod[0] != r->prod[0] /*&& r->prod[0] != 'e'*/) {
            	push(beta, r);
			}
        e = e->next;
    }

    return beta;
}

Queue * newBeta(Queue *beta, Number *nl) {
	char *newProduction;
	Element *e = beta->begin;


	//unsigned int aux = nl->value + 256;
	nl->value += 256;

    while (e != NULL) {
        Rule *r = (struct Rule*) e->data;  

	
		if (r->prod[0] == LAMBDA) {
			r->prod[0] = '\0';
			newProduction = createString(NCHAR);
			strcpy(newProduction, newNonTerminal(nl->value));
			e = e->next;
			continue;
		}

		int size = strlen(r->prod);
		if (r->prod[size-1] == '`') {
			e = e->next;
			continue;
		}

		newProduction = createString(NCHAR);

		strcpy(newProduction, r->prod);
		strcat(newProduction, newNonTerminal(nl->value));

		r->prod = newProduction;

        e = e->next;
    }

    return beta;
}

Queue * newAlpha(Queue *alpha, Number *nl, Rule *r) {
	char *newProduction;

	if (r->prod[strlen(r->prod)-1] == '`') {
		r->prod[strlen(r->prod)-1] = '\0';
		r->prod[strlen(r->prod)-2] = '\0';
	}

	Rule *alphaR;
	alphaR->l = nl->value;
	alphaR->prod = r->prod+1;

	newProduction = createString(NCHAR);
	strcpy(newProduction, alphaR->prod);

	strcat(newProduction, newNonTerminal(alphaR->l));

	alphaR->prod = newProduction;

	push(alpha, createRule(alphaR->l, alphaR->prod));
	
    return alpha;
}

bool checkE(Queue *grammar, unsigned int n) {
	Element *e = grammar->begin;

	while (e != NULL) {
        Rule *r = (struct Rule*) e->data;     
        if (r->l == n) 
        	//if (r->prod[0] == nr->value)
			if (r->prod[0] == 'e') return true;	//already inserted e-production
        e = e->next;
    }
	
	return false;	//no e-production found
}

void imediateElimination(Queue *grammar) {
	char *newProduction;
	Element *e = grammar->begin;

	while (e != NULL) {
		Rule *m = e->data;
		Queue *betaRules = initializeQueue(NULL);
		Queue *alphaRules = initializeQueue(NULL);

		char *eProduction;

        Rule *r = (struct Rule*) e->data;     
        if (r->l == r->prod[0] && r->prod[1] != '`') {

			findBeta(grammar, betaRules, e->data, r->prod);
			newBeta(betaRules, e->data);

			newAlpha(alphaRules, e->data, r);
			fprintf(stderr, "APPEND: %s\n", appendToPosition(grammar, alphaRules, m) ? "True" : "False");

			clearQueue(betaRules); // Clear the betaRules
			clearQueue(alphaRules); // Clear the alphaRules

			eProduction = createString(NCHAR);
			eProduction[0] = LAMBDA;
			eProduction[1] = '\0';


			if (!checkE(grammar, m->l)) push(grammar, createRule(m->l, eProduction));

			erase(grammar, m);  // Eliminar match da gramática
			freeQueue(betaRules);
			freeQueue(alphaRules);

		}
        e = e->next;
    }
}

void substituteMatches(Queue *grammar, Queue *matches, Number *n) {
    char *newProduction;     
    Queue *newRules = initializeQueue(NULL);

    // Para cada match
    Element *e = matches->begin;
    while (e != NULL) {
        Rule *m = e->data;

        // Achar todas as produções de n na gramática
        Element *t = grammar->begin;
        while (t != NULL) {
            Rule *r = t->data;
            if (r->l == n->value) {
                newProduction = createString(NCHAR);

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
        e = e->next; 
    }

    // Free the Queue
    freeQueue(newRules);
}

void globalElimination(Queue *grammar, Queue *order) {
    Element *e = order->begin;
    Queue *matches = initializeQueue(NULL); 

    while (e != NULL) {
        Element *t = order->begin;
        while (t != e) {
            findAllMatches(grammar, matches, e->data, t->data);
            
            if (!isQueueEmpty(matches)) { 
                printGrammar(stdout, matches);
                substituteMatches(grammar, matches, t->data);
            }

            imediateElimination(grammar);  
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
 
    //Load Grammar
    g = initializeQueue(freeRule);
    readGrammar(filename, g);
    printGrammar(out, g);

    order = initializeQueue(freeNumber);
    orderNT(g, order);
    printOrder(out, order);

    globalElimination(g, order);
    printGrammar(out, g);

	//fprintf(stdout, "Terminou tudo, so falta os free\n");

    freeQueue(g);
	//fprintf(stdout, "After free g\n");
    freeQueue(order);
	//fprintf(stdout, "After free order\n");

    return 0;
}
