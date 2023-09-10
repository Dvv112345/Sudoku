#include <stdio.h>
#include <ctype.h>

#define SIZE 81
#define LENGTH 9
#define PEER_COUNT 20

typedef struct position
{
    // Specifies the row, column of a square
    int row;
    int col;
}position_t;

typedef struct square
{
    // Specifies the position and value for a square
    position_t pos;
    int value;
}square_t;

typedef struct variable
{
    // Specifies the position and domain for a variable
    position_t pos;
    int domain[LENGTH];
    int dom_size;
}variable_t;

typedef square_t broad_t[SIZE];

typedef struct problem
{
    // Assigned square and unassigned square
    variable_t variables[SIZE];
    int solved; 
}problem_t;

typedef struct arc
{
    // pair of positions
    position_t x;
    position_t y;
}arc_t;


variable_t init_var(position_t pos, int value);
int load_broad(broad_t broad, problem_t *problem);
void print_broad(broad_t broad);
void peer(position_t pos, position_t peers[PEER_COUNT]);
int revise(problem_t *problem, position_t x, position_t y);
int pos2index(position_t pos);
int AC_3(problem_t *problem, position_t start);
void prob_dupe(problem_t *problem, problem_t *dupe);
position_t select_var(problem_t *problem);
int backtrack(problem_t *problem);
int solve(problem_t *problem);
int degree(problem_t *problem, position_t pos);
void set_broad(problem_t *problem, broad_t broad);

int main(void)
{
    broad_t broad;
    problem_t problem;
    int response = load_broad(broad, &problem);
    if (response)
    {
        print_broad(broad);
    }
    else
    {
        printf("ERROR\n");
    }
    if (backtrack(&problem))
    {
        printf("SUCCESS!\n");
        set_broad(&problem, broad);
        print_broad(broad);
    }
    else
    {
        printf("CANNOT BE SOLVED!\n");
        
    }    
}

variable_t init_var(position_t pos, int value)
{
    variable_t variable;
    variable.pos = pos;
    if (value == 0)
    {
        variable.dom_size = 9;
        for (int i = 0; i<LENGTH; i++)
        {
            variable.domain[i] = i + 1;
        }
    }
    else
    {
        variable.dom_size = 1;
        variable.domain[0] = value;
    }
    return variable;
}

// Load the broad from a file
int load_broad(broad_t broad, problem_t *problem)
{
    int count = 0;
    problem->solved = 0;
    position_t pos;
    pos.row = 1;
    pos.col = 1;
    char value;
    square_t square;
    variable_t variable;
    while((value = getchar()) != EOF)
    {
        if (isdigit(value))
        {
            if (count >= SIZE || pos.row > LENGTH
     || pos.col > LENGTH
    )
            {
                return 0;
            }
            square.pos = pos;
            square.value = value - '0';
            broad[count] = square;
            problem->variables[count] = init_var(pos, square.value);
            count++;
            pos.col++;
            if (square.value != 0)
            {
                problem->solved++;
            }
        }
        else if (value == '\n')
        {
            pos.row++;
            pos.col = 1;
        }
    }
    if (count != SIZE)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

// Print the broad
void print_broad(broad_t broad)
{
    printf("-------------------------------------\n");
    for (int row = 0; row < LENGTH; row++)
    {
        for (int col = 0; col < LENGTH
; col++)
        {
            printf("| %d ", broad[row*LENGTH
     + col].value);
        }
        printf("|\n");
        printf("-------------------------------------\n");
    }
}

// Find peers of a square
void peer(position_t pos, position_t peers[PEER_COUNT])
{
    int count = 0;
    position_t current;
    for (int i = 1; i <= LENGTH; i++)
    {
        if (i != pos.row)
        {
            current.row = i;
            current.col = pos.col;
            peers[count] = current;
            count++;
        }

        if (i != pos.col)
        {
            current.col = i;
            current.row = pos.row;
            peers[count] = current;
            count++;
        }
    }

    int box_row = ((pos.row - 1)/3) * 3;
    int box_col = ((pos.col - 1)/3) *3;
    for (int i = 1; i <= 3; i++)
    {
        for (int j = 1; j <= 3; j++)
        {
            current.row = box_row + i;
            current.col = box_col + j;
            if (current.col != pos.col && current.row != pos.row)
            {
                peers[count] = current;
                count++;
            }
        }
    }
}

int pos2index(position_t pos)
{
    return ((pos.row - 1)*9 + pos.col - 1);
}

int revise(problem_t *problem, position_t x_pos, position_t y_pos)
{
    int revised = 0;
    variable_t x = problem->variables[pos2index(x_pos)];
    variable_t y = problem->variables[pos2index(y_pos)];
    int count = 0;
    int new[LENGTH];
    int valid = 0;
    for (int i = 0; i < x.dom_size; i++)
    {
        valid = 0;
        for (int j = 0; j < y.dom_size; j++)
        {
            if (y.domain[j]!=x.domain[i])
            {
                valid = 1;
                break;
            }
        }
        if (valid)
        {
            new[count] = x.domain[i];
            count++;
        }
    }
    if (count != problem->variables[pos2index(x_pos)].dom_size)
    {
        revised = 1;
        problem->variables[pos2index(x_pos)].dom_size = count;
        for (int i = 0; i<count; i++)
        {
            problem->variables[pos2index(x_pos)].domain[i] = new[i];
        }
    }
    return revised;
}

int AC_3(problem_t *problem, position_t start)
{
    position_t peers[PEER_COUNT];
    peer(start, peers);
    int arc_count = PEER_COUNT;
    int count = 0;
    position_t x_pos;
    position_t y_pos;
    int size;
    arc_t arcs[SIZE * SIZE];
    while (count < arc_count)
    {
        if (count < 20)
        {
            x_pos = peers[count];
            y_pos = start;
        }
        else
        {
            x_pos = arcs[count - 20].x;
            y_pos = arcs[count - 20].y;
        }

        if (revise(problem, x_pos, y_pos))
        {
            size = problem->variables[pos2index(x_pos)].dom_size;
            if (size == 0)
            {
                return 0;
            }
            else if (size == 1)
            {
                problem->solved++;
            }
            position_t new[PEER_COUNT];
            peer(x_pos, new);
            for (int i = 0; i < PEER_COUNT; i++)
            {
                if (new[i].col != y_pos.col && new[i].row != y_pos.row)
                {
                    arcs[arc_count-PEER_COUNT].x = new[i];
                    arcs[arc_count - PEER_COUNT].y = x_pos;
                    arc_count++; 
                }
            }
        }
        
        count++;
    }
    return 1;
}

void prob_dupe(problem_t *problem, problem_t *dupe)
{
    dupe->solved = problem->solved;
    for (int i = 0; i < SIZE; i++)
    {
        dupe->variables[i] = problem->variables[i];
    }
}

int degree(problem_t *problem, position_t pos)
{
    position_t peers[PEER_COUNT];
    peer(pos, peers);
    int count = 0;
    for (int i = 0; i < PEER_COUNT; i++)
    {
        if (problem->variables[pos2index(peers[i])].dom_size > 1)
        {
            count++;
        }
    }
    return count;
}

position_t select_var(problem_t *problem)
{
    int mrv = LENGTH + 1;
    int h_deg = PEER_COUNT + 1;
    position_t pos;
    position_t current;
    int dom;
    int deg;
    for (int i = 0; i < SIZE; i++)
    {
        dom = problem->variables[i].dom_size;
        if (dom > 1)
        {
            current = problem->variables[i].pos;
            if (dom < mrv)
            {
                mrv = dom;
                pos = current;
                h_deg = degree(problem, pos);
            }
            else if (dom == mrv)
            {
                deg = degree(problem, current);
                if (deg > h_deg)
                {
                    pos = current;
                    h_deg = deg;
                }
            }
        }
    }
    return pos;
}

int backtrack(problem_t *problem)
{
    if (problem->solved == SIZE)
    {
        return 1;
    }
    problem_t backup;
    prob_dupe(problem, &backup);
    position_t selected = select_var(problem);
    int dom_size = problem->variables[pos2index(selected)].dom_size;
    int result;
    for (int i = 0; i < dom_size; i++)
    {
        problem->variables[pos2index(selected)].dom_size = 1;
        problem->variables[pos2index(selected)].domain[0] = problem->variables[pos2index(selected)].domain[i];
        problem->solved++;
        if (!AC_3(problem, selected))
        {
            prob_dupe(&backup, problem);
            continue;
        }
        result = backtrack(problem);
        if (result)
        {
            return result;
        }
        prob_dupe(&backup, problem);
    }

    return 0;
}

int solve(problem_t *problem)
{
    for (int i = 0; i<SIZE; i++)
    {
        if (!AC_3(problem, problem->variables[i].pos))
        {
            return 0;
        }
    }
    if (backtrack(problem))
    {
        return 1;
    }
    return 0;
}

void set_broad(problem_t *problem, broad_t broad)
{
    for (int i = 0; i < SIZE; i++)
    {
        broad[i].value = problem->variables[i].domain[0];
    }
}