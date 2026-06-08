#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define ROWS 40
#define COLS 80
#define MAX_OBJECTS 50

// Canvas
char canvas[ROWS][COLS];

// Object types
typedef enum {
    CIRCLE, RECTANGLE, LINE, TRIANGLE
} ShapeType;

// Object structure
typedef struct {
    ShapeType type;
    int active;
    // Common params
    int x1, y1, x2, y2, x3, y3;
    int radius;
    char ch; // character used ('*' or '_')
} Shape;

Shape objects[MAX_OBJECTS];
int object_count = 0;

// ─────────────────────────────────────────────
// Canvas utilities
// ─────────────────────────────────────────────

void clear_canvas() {
    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++)
            canvas[r][c] = ' ';
}

void set_pixel(int row, int col, char ch) {
    if (row >= 0 && row < ROWS && col >= 0 && col < COLS)
        canvas[row][col] = ch;
}

void display_canvas() {
    // Top border
    printf("+");
    for (int c = 0; c < COLS; c++) printf("-");
    printf("+\n");

    for (int r = 0; r < ROWS; r++) {
        printf("|");
        for (int c = 0; c < COLS; c++)
            printf("%c", canvas[r][c]);
        printf("|\n");
    }

    // Bottom border
    printf("+");
    for (int c = 0; c < COLS; c++) printf("-");
    printf("+\n");
}

// ─────────────────────────────────────────────
// Drawing functions
// ─────────────────────────────────────────────

// Draw a line using Bresenham's algorithm
void draw_line(int r1, int c1, int r2, int c2, char ch) {
    int dr = abs(r2 - r1), dc = abs(c2 - c1);
    int sr = (r1 < r2) ? 1 : -1;
    int sc = (c1 < c2) ? 1 : -1;
    int err = dr - dc;

    while (1) {
        set_pixel(r1, c1, ch);
        if (r1 == r2 && c1 == c2) break;
        int e2 = 2 * err;
        if (e2 > -dc) { err -= dc; r1 += sr; }
        if (e2 <  dr) { err += dr; c1 += sc; }
    }
}

// Draw a rectangle (outline)
void draw_rectangle(int r1, int c1, int r2, int c2, char ch) {
    // Normalize
    if (r1 > r2) { int t = r1; r1 = r2; r2 = t; }
    if (c1 > c2) { int t = c1; c1 = c2; c2 = t; }

    for (int c = c1; c <= c2; c++) {
        set_pixel(r1, c, ch);
        set_pixel(r2, c, ch);
    }
    for (int r = r1; r <= r2; r++) {
        set_pixel(r, c1, ch);
        set_pixel(r, c2, ch);
    }
}

// Draw a circle using midpoint circle algorithm
void draw_circle(int cr, int cc, int radius, char ch) {
    int x = 0, y = radius;
    int d = 1 - radius;

    while (x <= y) {
        set_pixel(cr + y, cc + x, ch);
        set_pixel(cr - y, cc + x, ch);
        set_pixel(cr + y, cc - x, ch);
        set_pixel(cr - y, cc - x, ch);
        set_pixel(cr + x, cc + y, ch);
        set_pixel(cr - x, cc + y, ch);
        set_pixel(cr + x, cc - y, ch);
        set_pixel(cr - x, cc - y, ch);

        if (d < 0)
            d += 2 * x + 3;
        else {
            d += 2 * (x - y) + 5;
            y--;
        }
        x++;
    }
}

// Draw a triangle using three line segments
void draw_triangle(int r1, int c1, int r2, int c2, int r3, int c3, char ch) {
    draw_line(r1, c1, r2, c2, ch);
    draw_line(r2, c2, r3, c3, ch);
    draw_line(r3, c3, r1, c1, ch);
}

// ─────────────────────────────────────────────
// Render all active objects onto canvas
// ─────────────────────────────────────────────

void render_all() {
    clear_canvas();
    for (int i = 0; i < object_count; i++) {
        if (!objects[i].active) continue;
        Shape *s = &objects[i];
        switch (s->type) {
            case CIRCLE:
                draw_circle(s->y1, s->x1, s->radius, s->ch);
                break;
            case RECTANGLE:
                draw_rectangle(s->y1, s->x1, s->y2, s->x2, s->ch);
                break;
            case LINE:
                draw_line(s->y1, s->x1, s->y2, s->x2, s->ch);
                break;
            case TRIANGLE:
                draw_triangle(s->y1, s->x1, s->y2, s->x2, s->y3, s->x3, s->ch);
                break;
        }
    }
}

// ─────────────────────────────────────────────
// Object management
// ─────────────────────────────────────────────

int add_object(Shape s) {
    if (object_count >= MAX_OBJECTS) {
        printf("Max objects reached!\n");
        return -1;
    }
    s.active = 1;
    objects[object_count] = s;
    printf("Object added with ID: %d\n", object_count);
    return object_count++;
}

void delete_object(int id) {
    if (id < 0 || id >= object_count || !objects[id].active) {
        printf("Invalid object ID.\n");
        return;
    }
    objects[id].active = 0;
    printf("Object %d deleted.\n", id);
}

void list_objects() {
    printf("\n%-5s %-12s %-5s %-30s\n", "ID", "Type", "Char", "Parameters");
    printf("%-5s %-12s %-5s %-30s\n", "----", "-----------", "----", "-----------------------------");
    int found = 0;
    for (int i = 0; i < object_count; i++) {
        if (!objects[i].active) continue;
        found = 1;
        Shape *s = &objects[i];
        const char *type_name[] = {"Circle", "Rectangle", "Line", "Triangle"};
        printf("%-5d %-12s %-5c ", i, type_name[s->type], s->ch);
        switch (s->type) {
            case CIRCLE:
                printf("center=(%d,%d) r=%d", s->x1, s->y1, s->radius); break;
            case RECTANGLE:
                printf("(%d,%d) to (%d,%d)", s->x1, s->y1, s->x2, s->y2); break;
            case LINE:
                printf("(%d,%d) to (%d,%d)", s->x1, s->y1, s->x2, s->y2); break;
            case TRIANGLE:
                printf("(%d,%d),(%d,%d),(%d,%d)", s->x1,s->y1,s->x2,s->y2,s->x3,s->y3); break;
        }
        printf("\n");
    }
    if (!found) printf("No active objects.\n");
}

// ─────────────────────────────────────────────
// Modify an object
// ─────────────────────────────────────────────

void modify_object(int id) {
    if (id < 0 || id >= object_count || !objects[id].active) {
        printf("Invalid object ID.\n");
        return;
    }
    Shape *s = &objects[id];
    printf("Modifying object %d. Enter new parameters:\n", id);

    printf("Character to use (* or _): ");
    char ch; scanf(" %c", &ch);
    s->ch = ch;

    switch (s->type) {
        case CIRCLE:
            printf("New center col row: "); scanf("%d %d", &s->x1, &s->y1);
            printf("New radius: ");         scanf("%d", &s->radius);
            break;
        case RECTANGLE:
            printf("New top-left col row: ");    scanf("%d %d", &s->x1, &s->y1);
            printf("New bottom-right col row: "); scanf("%d %d", &s->x2, &s->y2);
            break;
        case LINE:
            printf("New start col row: "); scanf("%d %d", &s->x1, &s->y1);
            printf("New end col row: ");   scanf("%d %d", &s->x2, &s->y2);
            break;
        case TRIANGLE:
            printf("New point1 col row: "); scanf("%d %d", &s->x1, &s->y1);
            printf("New point2 col row: "); scanf("%d %d", &s->x2, &s->y2);
            printf("New point3 col row: "); scanf("%d %d", &s->x3, &s->y3);
            break;
    }
    printf("Object %d modified.\n", id);
}

// ─────────────────────────────────────────────
// Input helpers
// ─────────────────────────────────────────────

char get_char() {
    char ch;
    printf("Character to use (* or _): ");
    scanf(" %c", &ch);
    return ch;
}

void menu_add_circle() {
    Shape s = {0}; s.type = CIRCLE;
    printf("Enter center col row (e.g. 40 20): "); scanf("%d %d", &s.x1, &s.y1);
    printf("Enter radius: "); scanf("%d", &s.radius);
    s.ch = get_char();
    add_object(s);
}

void menu_add_rectangle() {
    Shape s = {0}; s.type = RECTANGLE;
    printf("Enter top-left col row: ");    scanf("%d %d", &s.x1, &s.y1);
    printf("Enter bottom-right col row: "); scanf("%d %d", &s.x2, &s.y2);
    s.ch = get_char();
    add_object(s);
}

void menu_add_line() {
    Shape s = {0}; s.type = LINE;
    printf("Enter start col row: "); scanf("%d %d", &s.x1, &s.y1);
    printf("Enter end col row: ");   scanf("%d %d", &s.x2, &s.y2);
    s.ch = get_char();
    add_object(s);
}

void menu_add_triangle() {
    Shape s = {0}; s.type = TRIANGLE;
    printf("Enter point1 col row: "); scanf("%d %d", &s.x1, &s.y1);
    printf("Enter point2 col row: "); scanf("%d %d", &s.x2, &s.y2);
    printf("Enter point3 col row: "); scanf("%d %d", &s.x3, &s.y3);
    s.ch = get_char();
    add_object(s);
}

// ─────────────────────────────────────────────
// Demo: pre-load some shapes
// ─────────────────────────────────────────────

void load_demo() {
    // Circle
    Shape c = {0}; c.type = CIRCLE; c.x1 = 15; c.y1 = 10; c.radius = 7; c.ch = '*';
    add_object(c);

    // Rectangle
    Shape r = {0}; r.type = RECTANGLE; r.x1 = 30; r.y1 = 5; r.x2 = 55; r.y2 = 20; r.ch = '*';
    add_object(r);

    // Line
    Shape l = {0}; l.type = LINE; l.x1 = 0; l.y1 = 25; l.x2 = 79; l.y2 = 35; l.ch = '_';
    add_object(l);

    // Triangle
    Shape t = {0}; t.type = TRIANGLE;
    t.x1 = 40; t.y1 = 22;
    t.x2 = 25; t.y2 = 38;
    t.x3 = 55; t.y3 = 38;
    t.ch = '*';
    add_object(t);
}

// ─────────────────────────────────────────────
// Main menu
// ─────────────────────────────────────────────

int main() {
    clear_canvas();
    printf("╔══════════════════════════════════════╗\n");
    printf("║   2D Graphics Editor  (* and _)      ║\n");
    printf("╚══════════════════════════════════════╝\n");
    printf("Canvas: %d rows x %d cols\n\n", ROWS, COLS);

    // Load demo shapes
    load_demo();
    render_all();

    int choice;
    while (1) {
        printf("\n┌─────────────────────────────┐\n");
        printf("│          MAIN MENU          │\n");
        printf("├─────────────────────────────┤\n");
        printf("│  1. Display canvas          │\n");
        printf("│  2. List objects            │\n");
        printf("│  3. Add circle              │\n");
        printf("│  4. Add rectangle           │\n");
        printf("│  5. Add line                │\n");
        printf("│  6. Add triangle            │\n");
        printf("│  7. Delete object           │\n");
        printf("│  8. Modify object           │\n");
        printf("│  9. Clear all objects       │\n");
        printf("│  0. Exit                    │\n");
        printf("└─────────────────────────────┘\n");
        printf("Choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                render_all();
                display_canvas();
                break;
            case 2:
                list_objects();
                break;
            case 3:
                menu_add_circle();
                render_all();
                display_canvas();
                break;
            case 4:
                menu_add_rectangle();
                render_all();
                display_canvas();
                break;
            case 5:
                menu_add_line();
                render_all();
                display_canvas();
                break;
            case 6:
                menu_add_triangle();
                render_all();
                display_canvas();
                break;
            case 7: {
                int id;
                printf("Enter object ID to delete: ");
                scanf("%d", &id);
                delete_object(id);
                render_all();
                display_canvas();
                break;
            }
            case 8: {
                int id;
                printf("Enter object ID to modify: ");
                scanf("%d", &id);
                modify_object(id);
                render_all();
                display_canvas();
                break;
            }
            case 9:
                object_count = 0;
                clear_canvas();
                printf("All objects cleared.\n");
                break;
            case 0:
                printf("Goodbye!\n");
                return 0;
            default:
                printf("Invalid choice. Try again.\n");
        }
    }
}
