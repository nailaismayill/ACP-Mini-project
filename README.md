# 2D Graphics Editor (C)

A console-based 2D graphics editor written in C that uses `*` and `_` characters to draw shapes on a text canvas (40 rows × 80 columns).

## Features

- Draw **circles**, **rectangles**, **lines**, and **triangles**
- **Add**, **delete**, and **modify** objects
- All shapes stored in a 2D character array
- Interactive text menu

## Shapes & Algorithms

| Shape     | Algorithm Used               |
|-----------|------------------------------|
| Circle    | Midpoint Circle Algorithm    |
| Rectangle | Outline via 4 edges          |
| Line      | Bresenham's Line Algorithm   |
| Triangle  | Three Bresenham lines        |

## How to Compile & Run

```bash
gcc -o graphics_editor graphics_editor.c -lm
./graphics_editor
```

## Menu Options

```
1. Display canvas        — render all shapes and print the canvas
2. List objects          — show all active objects with their IDs
3. Add circle            — enter center (col row) and radius
4. Add rectangle         — enter top-left and bottom-right corners
5. Add line              — enter start and end points
6. Add triangle          — enter three vertices
7. Delete object         — remove a shape by ID
8. Modify object         — change parameters of an existing shape
9. Clear all objects     — wipe the canvas
0. Exit
```

## Input Format

Coordinates are entered as **col row** (x y), where:
- `col` → horizontal position (0–79)
- `row` → vertical position (0–39)

Choose `*` or `_` as the drawing character for each shape.

## Example Canvas Output

```
+--------------------------------------------------------------------------------+
|             *****                                                              |
|           **     **                                                            |
|          *         *         **************************                        |
|         *           *        *                        *                        |
|        *             *       *                        *                        |
|             ...               ...                                              |
|____                                 *     *                                    |
|    ________                        *       *                                   |
+--------------------------------------------------------------------------------+
```

## File Structure

```
graphics_editor.c   — full source code
README.md           — this file
```

