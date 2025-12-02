#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>

const double epsilon = (double)1e-6;

bool floatCompare(double a,double b){
    return fabs(a-b) <= epsilon;
}

// Struktury pomocnicze
typedef struct {
    double x;
    double y;
} point;

typedef struct {
    point p1;
    point p2;
} line;

void getLine(line* l){
    scanf("%lf%lf%lf%lf", &l->p1.x, &l->p1.y, &l->p2.x, &l->p2.y);
}

typedef struct {
    point p1;
    point p2;
} rectangle;

void getRectangle(rectangle* rect){
    scanf("%lf%lf%lf%lf", &rect->p1.x, &rect->p1.y, &rect->p2.x, &rect->p2.y);
}

typedef struct {
    point center;
    double radius;
} circle;

void getCircle(circle* circ){
    scanf("%lf%lf%lf", &circ->center.x, &circ->center.y, &circ->radius);
}

typedef enum {
    Rectangle,
    Circle,
} FigureType;

typedef struct {
    FigureType type;
    union {
        rectangle rect;
        circle circ;
    };
    line* folds;
    size_t foldsSize;
} figure;

// Kopiuje figure skladana i dodaje zlozenie
void getFoldedFigure(figure* base, figure* f){
    f->type = base->type;
    if (base->type == Rectangle){
        f->rect = base->rect;
    }
    else {
        f->circ = base->circ;
    }
    f->folds = malloc((base->foldsSize + 1) * sizeof(line));
    f->foldsSize = base->foldsSize + 1;
    if (base->foldsSize > 0){
        memcpy(f->folds, base->folds, base->foldsSize * sizeof(line));
    }
    // Ostatnie zlozenie tej kartki
    getLine(&f->folds[f->foldsSize - 1]);
}

void getfigure(figure* plane, figure* f){
    char type;
    int base;
    scanf(" %c", &type);
    switch(type){
    case 'P':
        getRectangle(&f->rect);
        f->type = Rectangle; 
        f->folds = NULL;
        f->foldsSize = 0;       
        break;
    case 'K':
        getCircle(&f->circ);
        f->type = Circle;
        f->folds = NULL;
        f->foldsSize = 0;
        break;
    case 'Z':
        scanf("%d", &base);
        getFoldedFigure(&plane[base-1], f);
        break;
    }
}

// Funkcje do obliczania ilosci warstw w punkcie

point reflectPoint(point* p, line* l){
    // Rownanie ogolne prostej
    double a = l->p1.y - l->p2.y;
    double b = l->p2.x - l->p1.x;
    double c = l->p1.x * l->p2.y - l->p1.y * l->p2.x;
    double d = (a * p->x + b * p->y + c)/(a * a + b * b);
    return (point){ 
        .x = p->x - 2 * a * d, 
        .y = p->y - 2 * b * d
    };
}

// Sprawdza czy punkt jest po tej stronie prostej, na ktora skladamy 
// (- jesli nie, 0 jesli na, + jesli na dobrej)
double isOnFoldedSideOfLine(point* p, line* l){
    return (l->p2.x - l->p1.x) * (p->y - l->p1.y) - (l->p2.y - l->p1.y) * (p->x - l->p1.x); 
}

bool isInRectangle(point* p, rectangle* rect){
    return ((p->x + epsilon >= rect->p1.x) && (p->y + epsilon >= rect->p1.y) &&
            (p->x - epsilon <= rect->p2.x) && (p->y - epsilon <= rect->p2.y));
}

bool isInCircle(point* p, circle* circ){
    return (((p->x - circ->center.x) * (p->x - circ->center.x) + 
             (p->y - circ->center.y) * (p->y - circ->center.y)) <= circ->radius * circ->radius + epsilon);
}

bool isInFigure(point* p, figure* fig){
    return (fig->type == Rectangle ? isInRectangle(p, &fig->rect) : isInCircle(p, &fig->circ));
}

// Funkcja zliczajaca ile razy pinezka przechodzi przez figure
// rekurencyjnie liczac ile razy przecina figure przed zlozeniem
// i ile razy jej odbicie przecina figure przed zlozeniem
int countLayerIntersections(point* pin, figure* fig, int topLayer){
    if (topLayer == -1){
        return (int)isInFigure(pin, fig);
    }

    double mark = isOnFoldedSideOfLine(pin, &fig->folds[topLayer]);
    if (floatCompare(mark, 0)){
        return countLayerIntersections(pin, fig, topLayer - 1);
    }
    if (mark < 0){
        return 0;
    }
    else {
        point reflection = reflectPoint(pin, &fig->folds[topLayer]);
        return (countLayerIntersections(pin, fig, topLayer - 1) +
                countLayerIntersections(&reflection, fig, topLayer - 1));
    }
}

int main(){
    int n, q;
    scanf("%d%d", &n, &q);
    figure* plane = malloc((size_t)n * sizeof(figure));
    if (plane == NULL){
        exit(1);
    }

    for (int i = 0; i<n; i++){
        getfigure(plane, &plane[i]);
    }

    int* answer = malloc((size_t)q * sizeof(int));
    for (int i = 0; i<q; i++){
        int figureNumber;
        point pin;
        scanf("%d%lf%lf", &figureNumber, &pin.x, &pin.y);
        answer[i] = countLayerIntersections
                (&pin, &plane[figureNumber - 1], (int)plane[figureNumber - 1].foldsSize - 1); 
    }

    for (int i = 0; i<q; i++){
        printf("%d\n", answer[i]);
    }

    // Zwalniamy alokowana pamiec
    free(answer);
    for (int i = 0; i<n; i++){
        if (plane[i].foldsSize){
            free(plane[i].folds);
        }
    }
    free(plane);
    return 0;
}
