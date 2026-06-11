#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Enable ANSI virtual terminal processing on Windows
#ifdef _WIN32
#include <windows.h>
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif
#endif

// Canvas limits
#define ROWS 20
#define COLS 60
#define MAX_SHAPES 100

// Shape Types
typedef enum {
    SHAPE_LINE = 1,
    SHAPE_RECTANGLE,
    SHAPE_CIRCLE,
    SHAPE_TRIANGLE
} ShapeType;

// Shape Parameter structures
typedef struct {
    int x1, y1;
    int x2, y2;
} LineParams;

typedef struct {
    int x, y;
    int w, h;
} RectParams;

typedef struct {
    int cx, cy;
    int r;
} CircleParams;

typedef struct {
    int x1, y1;
    int x2, y2;
    int x3, y3;
} TriangleParams;

// Shape wrapper
typedef struct {
    int id;
    ShapeType type;
    char draw_char;
    int is_active;
    union {
        LineParams line;
        RectParams rect;
        CircleParams circle;
        TriangleParams triangle;
    } params;
} Shape;

// Global State
Shape shapes[MAX_SHAPES];
int shape_count = 0;
int next_id = 1;
char canvas[ROWS][COLS];

// Forward declarations
void init_canvas(char grid[ROWS][COLS]);
void set_pixel(char grid[ROWS][COLS], int x, int y, char ch);
void draw_line(char grid[ROWS][COLS], int x1, int y1, int x2, int y2, char ch);
void draw_rectangle(char grid[ROWS][COLS], int x, int y, int w, int h, char ch);
void draw_circle(char grid[ROWS][COLS], int cx, int cy, int r, char ch);
void draw_triangle(char grid[ROWS][COLS], int x1, int y1, int x2, int y2, int x3, int y3, char ch);
void render_canvas(char grid[ROWS][COLS], Shape shapes_list[], int count);
void display_canvas(char grid[ROWS][COLS]);
void print_shapes_list(Shape shapes_list[], int count);
void get_string_input(char *buffer, int size);
int get_int_input(const char *prompt, int min_val, int max_val);
void add_new_shape();
void modify_existing_shape();
void delete_existing_shape();

// Helper to configure Windows console for ANSI color support
void setup_console() {
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode)) {
            dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hOut, dwMode);
        }
    }
#endif
}

int main() {
    setup_console();
    init_canvas(canvas);
    
    char choice_buf[10];
    int choice = 0;
    
    while (1) {
        // Redraw canvas and print list
        render_canvas(canvas, shapes, shape_count);
        display_canvas(canvas);
        print_shapes_list(shapes, shape_count);
        
        // Display Menu
        printf("\x1b[36m=== 2D Graphics Editor Menu ===\x1b[0m\n");
        printf("1. Add Shape\n");
        printf("2. Modify Shape\n");
        printf("3. Delete Shape\n");
        printf("4. Clear Canvas (Delete All)\n");
        printf("5. Exit\n");
        printf("\x1b[33mEnter choice (1-5): \x1b[0m");
        
        get_string_input(choice_buf, sizeof(choice_buf));
        if (sscanf(choice_buf, "%d", &choice) != 1) {
            printf("\x1b[31mInvalid selection. Press Enter to retry...\x1b[0m\n");
            get_string_input(choice_buf, sizeof(choice_buf));
            continue;
        }
        
        if (choice == 5) {
            printf("\nExiting Graphics Editor. Goodbye!\n");
            break;
        }
        
        switch (choice) {
            case 1:
                add_new_shape();
                break;
            case 2:
                modify_existing_shape();
                break;
            case 3:
                delete_existing_shape();
                break;
            case 4:
                // Clear all active shapes
                for (int i = 0; i < shape_count; i++) {
                    shapes[i].is_active = 0;
                }
                printf("\x1b[32mCanvas cleared successfully! Press Enter to continue...\x1b[0m\n");
                get_string_input(choice_buf, sizeof(choice_buf));
                break;
            default:
                printf("\x1b[31mInvalid selection (1-5). Press Enter to retry...\x1b[0m\n");
                get_string_input(choice_buf, sizeof(choice_buf));
                break;
        }
    }
    return 0;
}

// Canvas Initialization
void init_canvas(char grid[ROWS][COLS]) {
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            grid[r][c] = '_';
        }
    }
}

// Safe Pixel Writer (Bounds Checking)
void set_pixel(char grid[ROWS][COLS], int x, int y, char ch) {
    if (x >= 0 && x < COLS && y >= 0 && y < ROWS) {
        grid[y][x] = ch;
    }
}

// Bresenham's Line Drawing Algorithm
void draw_line(char grid[ROWS][COLS], int x1, int y1, int x2, int y2, char ch) {
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;
    
    while (1) {
        set_pixel(grid, x1, y1, ch);
        if (x1 == x2 && y1 == y2) break;
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}

// Rectangle drawing using border iteration
void draw_rectangle(char grid[ROWS][COLS], int x, int y, int w, int h, char ch) {
    if (w <= 0 || h <= 0) return;
    // Top and Bottom lines
    for (int col = x; col < x + w; col++) {
        set_pixel(grid, col, y, ch);
        set_pixel(grid, col, y + h - 1, ch);
    }
    // Left and Right lines
    for (int row = y; row < y + h; row++) {
        set_pixel(grid, x, row, ch);
        set_pixel(grid, x + w - 1, row, ch);
    }
}

// Midpoint Circle helper
void draw_circle_points(char grid[ROWS][COLS], int cx, int cy, int x, int y, char ch) {
    set_pixel(grid, cx + x, cy + y, ch);
    set_pixel(grid, cx - x, cy + y, ch);
    set_pixel(grid, cx + x, cy - y, ch);
    set_pixel(grid, cx - x, cy - y, ch);
    set_pixel(grid, cx + y, cy + x, ch);
    set_pixel(grid, cx - y, cy + x, ch);
    set_pixel(grid, cx + y, cy - x, ch);
    set_pixel(grid, cx - y, cy - x, ch);
}

// Midpoint Circle Algorithm
void draw_circle(char grid[ROWS][COLS], int cx, int cy, int r, char ch) {
    if (r < 0) return;
    if (r == 0) {
        set_pixel(grid, cx, cy, ch);
        return;
    }
    int x = 0;
    int y = r;
    int d = 3 - 2 * r;
    
    draw_circle_points(grid, cx, cy, x, y, ch);
    while (y >= x) {
        x++;
        if (d > 0) {
            y--;
            d = d + 4 * (x - y) + 10;
        } else {
            d = d + 4 * x + 6;
        }
        draw_circle_points(grid, cx, cy, x, y, ch);
    }
}

// Triangle Drawing (3 lines)
void draw_triangle(char grid[ROWS][COLS], int x1, int y1, int x2, int y2, int x3, int y3, char ch) {
    draw_line(grid, x1, y1, x2, y2, ch);
    draw_line(grid, x2, y2, x3, y3, ch);
    draw_line(grid, x3, y3, x1, y1, ch);
}

// Render active shapes onto canvas
void render_canvas(char grid[ROWS][COLS], Shape shapes_list[], int count) {
    init_canvas(grid);
    for (int i = 0; i < count; i++) {
        if (shapes_list[i].is_active) {
            char ch = shapes_list[i].draw_char;
            switch (shapes_list[i].type) {
                case SHAPE_LINE:
                    draw_line(grid, shapes_list[i].params.line.x1, shapes_list[i].params.line.y1,
                              shapes_list[i].params.line.x2, shapes_list[i].params.line.y2, ch);
                    break;
                case SHAPE_RECTANGLE:
                    draw_rectangle(grid, shapes_list[i].params.rect.x, shapes_list[i].params.rect.y,
                                   shapes_list[i].params.rect.w, shapes_list[i].params.rect.h, ch);
                    break;
                case SHAPE_CIRCLE:
                    draw_circle(grid, shapes_list[i].params.circle.cx, shapes_list[i].params.circle.cy,
                                shapes_list[i].params.circle.r, ch);
                    break;
                case SHAPE_TRIANGLE:
                    draw_triangle(grid, shapes_list[i].params.triangle.x1, shapes_list[i].params.triangle.y1,
                                  shapes_list[i].params.triangle.x2, shapes_list[i].params.triangle.y2,
                                  shapes_list[i].params.triangle.x3, shapes_list[i].params.triangle.y3, ch);
                    break;
            }
        }
    }
}

// Render canvas grid to console stdout
void display_canvas(char grid[ROWS][COLS]) {
    // Clear console using ANSI sequence
    printf("\x1b[H\x1b[J");
    
    printf("\x1b[35mCanvas View (%d x %d):\x1b[0m\n", COLS, ROWS);
    
    // Top border
    printf("   +");
    for (int x = 0; x < COLS; x++) printf("-");
    printf("+\n");
    
    // Rows representation
    for (int y = 0; y < ROWS; y++) {
        printf("%2d |", y);
        for (int x = 0; x < COLS; x++) {
            if (grid[y][x] == '*') {
                printf("\x1b[32m%c\x1b[0m", grid[y][x]); // Green shapes
            } else {
                printf("%c", grid[y][x]); // Default background character
            }
        }
        printf("|\n");
    }
    
    // Bottom border
    printf("   +");
    for (int x = 0; x < COLS; x++) printf("-");
    printf("+\n");
    
    // Column header indicators (x coordinate helper)
    printf("    ");
    for (int x = 0; x < COLS; x++) {
        if (x % 10 == 0) printf("%d", x / 10);
        else printf(" ");
    }
    printf("\n    ");
    for (int x = 0; x < COLS; x++) {
        printf("%d", x % 10);
    }
    printf("\n\n");
}

// Display shape configurations in table format
void print_shapes_list(Shape shapes_list[], int count) {
    printf("\x1b[1mActive Shapes (Objects):\x1b[0m\n");
    int active_found = 0;
    for (int i = 0; i < count; i++) {
        if (shapes_list[i].is_active) {
            active_found = 1;
            printf("  ID %d: ", shapes_list[i].id);
            switch (shapes_list[i].type) {
                case SHAPE_LINE:
                    printf("Line from (%d, %d) to (%d, %d)\n",
                           shapes_list[i].params.line.x1, shapes_list[i].params.line.y1,
                           shapes_list[i].params.line.x2, shapes_list[i].params.line.y2);
                    break;
                case SHAPE_RECTANGLE:
                    printf("Rectangle: top-left=(%d, %d), width=%d, height=%d\n",
                           shapes_list[i].params.rect.x, shapes_list[i].params.rect.y,
                           shapes_list[i].params.rect.w, shapes_list[i].params.rect.h);
                    break;
                case SHAPE_CIRCLE:
                    printf("Circle: center=(%d, %d), radius=%d\n",
                           shapes_list[i].params.circle.cx, shapes_list[i].params.circle.cy,
                           shapes_list[i].params.circle.r);
                    break;
                case SHAPE_TRIANGLE:
                    printf("Triangle: vertices (%d, %d), (%d, %d), (%d, %d)\n",
                           shapes_list[i].params.triangle.x1, shapes_list[i].params.triangle.y1,
                           shapes_list[i].params.triangle.x2, shapes_list[i].params.triangle.y2,
                           shapes_list[i].params.triangle.x3, shapes_list[i].params.triangle.y3);
                    break;
            }
        }
    }
    if (!active_found) {
        printf("  (No shapes created yet)\n");
    }
    printf("\n");
}

// Safe string input handler
void get_string_input(char *buffer, int size) {
    if (fgets(buffer, size, stdin) != NULL) {
        buffer[strcspn(buffer, "\r\n")] = '\0';
    }
}

// Input integer with boundaries
int get_int_input(const char *prompt, int min_val, int max_val) {
    char input_buf[64];
    int value;
    while (1) {
        printf("%s", prompt);
        get_string_input(input_buf, sizeof(input_buf));
        if (sscanf(input_buf, "%d", &value) == 1) {
            if (value >= min_val && value <= max_val) {
                return value;
            } else {
                printf("\x1b[31mValue out of range [%d, %d]. Try again.\x1b[0m\n", min_val, max_val);
            }
        } else {
            printf("\x1b[31mInvalid integer format. Try again.\x1b[0m\n");
        }
    }
}

// Add shape flow
void add_new_shape() {
    if (shape_count >= MAX_SHAPES) {
        printf("\x1b[31mError: Maximum shape capacity reached!\x1b[0m\n");
        char temp[10];
        get_string_input(temp, sizeof(temp));
        return;
    }
    
    printf("\x1b[34m--- Add New Shape ---\x1b[0m\n");
    printf("1. Line\n");
    printf("2. Rectangle\n");
    printf("3. Circle\n");
    printf("4. Triangle\n");
    int type_selection = get_int_input("Select shape type (1-4): ", 1, 4);
    
    Shape new_shape;
    new_shape.id = next_id++;
    new_shape.type = (ShapeType)type_selection;
    new_shape.draw_char = '*';
    new_shape.is_active = 1;
    
    switch (new_shape.type) {
        case SHAPE_LINE:
            printf("Enter Line Coordinates:\n");
            new_shape.params.line.x1 = get_int_input("  x1 (0-59): ", 0, 59);
            new_shape.params.line.y1 = get_int_input("  y1 (0-19): ", 0, 19);
            new_shape.params.line.x2 = get_int_input("  x2 (0-59): ", 0, 59);
            new_shape.params.line.y2 = get_int_input("  y2 (0-19): ", 0, 19);
            break;
            
        case SHAPE_RECTANGLE:
            printf("Enter Rectangle Specifications:\n");
            new_shape.params.rect.x = get_int_input("  Top-left x (0-59): ", 0, 59);
            new_shape.params.rect.y = get_int_input("  Top-left y (0-19): ", 0, 19);
            new_shape.params.rect.w = get_int_input("  Width (1-60): ", 1, 60);
            new_shape.params.rect.h = get_int_input("  Height (1-20): ", 1, 20);
            break;
            
        case SHAPE_CIRCLE:
            printf("Enter Circle Specifications:\n");
            new_shape.params.circle.cx = get_int_input("  Center x (0-59): ", 0, 59);
            new_shape.params.circle.cy = get_int_input("  Center y (0-19): ", 0, 19);
            new_shape.params.circle.r = get_int_input("  Radius (0-30): ", 0, 30);
            break;
            
        case SHAPE_TRIANGLE:
            printf("Enter Triangle Vertices:\n");
            new_shape.params.triangle.x1 = get_int_input("  x1 (0-59): ", 0, 59);
            new_shape.params.triangle.y1 = get_int_input("  y1 (0-19): ", 0, 19);
            new_shape.params.triangle.x2 = get_int_input("  x2 (0-59): ", 0, 59);
            new_shape.params.triangle.y2 = get_int_input("  y2 (0-19): ", 0, 19);
            new_shape.params.triangle.x3 = get_int_input("  x3 (0-59): ", 0, 59);
            new_shape.params.triangle.y3 = get_int_input("  y3 (0-19): ", 0, 19);
            break;
    }
    
    shapes[shape_count++] = new_shape;
    printf("\x1b[32mShape ID %d added successfully!\x1b[0m\n", new_shape.id);
    printf("Press Enter to continue...");
    char temp[10];
    get_string_input(temp, sizeof(temp));
}

// Modify shape flow
void modify_existing_shape() {
    printf("\x1b[34m--- Modify Existing Shape ---\x1b[0m\n");
    int target_id = get_int_input("Enter ID of shape to modify: ", 1, next_id - 1);
    
    int index = -1;
    for (int i = 0; i < shape_count; i++) {
        if (shapes[i].is_active && shapes[i].id == target_id) {
            index = i;
            break;
        }
    }
    
    if (index == -1) {
        printf("\x1b[31mError: Active shape with ID %d not found.\x1b[0m\n", target_id);
        printf("Press Enter to continue...");
        char temp[10];
        get_string_input(temp, sizeof(temp));
        return;
    }
    
    Shape *target = &shapes[index];
    switch (target->type) {
        case SHAPE_LINE:
            printf("Modifying Line ID %d (current: (%d,%d) to (%d,%d)):\n",
                   target->id, target->params.line.x1, target->params.line.y1,
                   target->params.line.x2, target->params.line.y2);
            target->params.line.x1 = get_int_input("  New x1 (0-59): ", 0, 59);
            target->params.line.y1 = get_int_input("  New y1 (0-19): ", 0, 19);
            target->params.line.x2 = get_int_input("  New x2 (0-59): ", 0, 59);
            target->params.line.y2 = get_int_input("  New y2 (0-19): ", 0, 19);
            break;
            
        case SHAPE_RECTANGLE:
            printf("Modifying Rectangle ID %d (current: top-left=(%d,%d), w=%d, h=%d):\n",
                   target->id, target->params.rect.x, target->params.rect.y,
                   target->params.rect.w, target->params.rect.h);
            target->params.rect.x = get_int_input("  New Top-left x (0-59): ", 0, 59);
            target->params.rect.y = get_int_input("  New Top-left y (0-19): ", 0, 19);
            target->params.rect.w = get_int_input("  New Width (1-60): ", 1, 60);
            target->params.rect.h = get_int_input("  New Height (1-20): ", 1, 20);
            break;
            
        case SHAPE_CIRCLE:
            printf("Modifying Circle ID %d (current: center=(%d,%d), r=%d):\n",
                   target->id, target->params.circle.cx, target->params.circle.cy,
                   target->params.circle.r);
            target->params.circle.cx = get_int_input("  New Center x (0-59): ", 0, 59);
            target->params.circle.cy = get_int_input("  New Center y (0-19): ", 0, 19);
            target->params.circle.r = get_int_input("  New Radius (0-30): ", 0, 30);
            break;
            
        case SHAPE_TRIANGLE:
            printf("Modifying Triangle ID %d (current: (%d,%d), (%d,%d), (%d,%d)):\n",
                   target->id, target->params.triangle.x1, target->params.triangle.y1,
                   target->params.triangle.x2, target->params.triangle.y2,
                   target->params.triangle.x3, target->params.triangle.y3);
            target->params.triangle.x1 = get_int_input("  New x1 (0-59): ", 0, 59);
            target->params.triangle.y1 = get_int_input("  New y1 (0-19): ", 0, 19);
            target->params.triangle.x2 = get_int_input("  New x2 (0-59): ", 0, 59);
            target->params.triangle.y2 = get_int_input("  New y2 (0-19): ", 0, 19);
            target->params.triangle.x3 = get_int_input("  New x3 (0-59): ", 0, 59);
            target->params.triangle.y3 = get_int_input("  New y3 (0-19): ", 0, 19);
            break;
    }
    
    printf("\x1b[32mShape ID %d modified successfully!\x1b[0m\n", target->id);
    printf("Press Enter to continue...");
    char temp[10];
    get_string_input(temp, sizeof(temp));
}

// Delete shape flow
void delete_existing_shape() {
    printf("\x1b[34m--- Delete Existing Shape ---\x1b[0m\n");
    int target_id = get_int_input("Enter ID of shape to delete: ", 1, next_id - 1);
    
    int index = -1;
    for (int i = 0; i < shape_count; i++) {
        if (shapes[i].is_active && shapes[i].id == target_id) {
            index = i;
            break;
        }
    }
    
    if (index == -1) {
        printf("\x1b[31mError: Active shape with ID %d not found.\x1b[0m\n", target_id);
        printf("Press Enter to continue...");
        char temp[10];
        get_string_input(temp, sizeof(temp));
        return;
    }
    
    shapes[index].is_active = 0;
    printf("\x1b[32mShape ID %d deleted successfully!\x1b[0m\n", target_id);
    printf("Press Enter to continue...");
    char temp[10];
    get_string_input(temp, sizeof(temp));
}
