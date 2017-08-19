#include <stdio.h>

#define MAX_LINE_SIZE 64
#define MAX_BRANCH_ADDRESS 512

#define TAKEN 't'
#define NOT_TAKEN 'n'

typedef enum {
    STATE_NOT_TAKEN_0,
    STATE_NOT_TAKEN_1,
    STATE_TAKEN_2,
    STATE_TAKEN_3
} state;

state addresses[MAX_BRANCH_ADDRESS + 1];

FILE *in_file;
FILE *out_file;

int total_predictions;
int right_predictions;

void predict(unsigned int from_address, 
             unsigned int to_address, 
             char branch_status)
{
    state current_state = addresses[from_address];
    state tmp = current_state;

    switch (current_state) {
        case STATE_NOT_TAKEN_0:
            if (branch_status == TAKEN)
                current_state += 1;
            else
                right_predictions += 1;
            break;
        case STATE_NOT_TAKEN_1:
            if (branch_status == TAKEN) {
                current_state += 1;
            } else {
                current_state -= 1;
                right_predictions += 1;
            }
            break;
        case STATE_TAKEN_2:
            if (branch_status == TAKEN) {
                current_state += 1;
                right_predictions += 1;
            } else {
                current_state -= 1;
            }
            break;
        case STATE_TAKEN_3:
            if (branch_status == TAKEN)
                right_predictions += 1;
            else
                current_state -= 1;
            break;
        default:
            break;
    }

    addresses[from_address] = current_state;

    if (tmp == STATE_NOT_TAKEN_0 || tmp == STATE_NOT_TAKEN_1)
        fprintf(out_file, "%u %c %u :: PREDICT NOT TAKEN (%d)\n", from_address, 
            branch_status, to_address, tmp);
    else
        fprintf(out_file, "%u %c %u :: PREDICT TAKEN (%d)\n", from_address, 
            branch_status, to_address, tmp);

    total_predictions += 1;
}

void run_predictor(const char *in_filename, const char *out_filename, state start_state)
{
    char line[MAX_LINE_SIZE];
    int line_number;
    unsigned int from_address;
    unsigned int to_address;
    char branch_status;

    line_number = 1;

    total_predictions = 0;
    right_predictions = 0;

    if ((in_file = fopen(in_filename, "r")) == NULL)
        return;

    if ((out_file = fopen(out_filename, "w+")) == NULL)
        return;

    int i;
    for (i = 0; i < MAX_BRANCH_ADDRESS + 1; ++i)
        addresses[i] = start_state;

    while (fgets(line, MAX_LINE_SIZE, in_file) != NULL) {
        int r = sscanf(line, "%u %c %u", &from_address, &branch_status, &to_address);
        if (r == EOF || r < 3 || from_address > 512 || to_address > 512 ||
            branch_status != 't' && branch_status != 'n')
            fprintf(stderr, "error: ignoring line %d due error(s)\n", line_number);
        else
            predict(from_address, to_address, branch_status);
        line_number += 1;
    }

    fclose(in_file);
}

int main(int argc, char **argv)
{
    if (argc < 3)
        return 1;

    run_predictor(argv[1], argv[2], STATE_TAKEN_3);

    if (total_predictions) {
        printf("Total branches: %d\n", total_predictions);
        printf("Correct predictions: %d\n", right_predictions);
        printf("Average prediction: %.1f%%\n", (float) right_predictions / total_predictions * 100);
    }

    return 0;
}