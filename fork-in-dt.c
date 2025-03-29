#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_CAPACITY 128

int extract_token_from_line(const char *line, const char *key, char **token) {
    char *key_pos = strstr(line, key);
    if (!key_pos) return 0;

    char *start_pos = strchr(key_pos, '"');
    if (!start_pos) return 0;

    char *end_pos = NULL;
    int escape = 0;
    for (char *c = start_pos + 1; *c != '\0'; c++) {
        if (*c == '\\' && !escape) {
            escape = 1;
        } else if (*c == '"' && !escape) {
            end_pos = c;
            break;
        } else {
            escape = 0;
        }
    }

    if (!end_pos) return 0;

    size_t token_length = end_pos - start_pos - 1;
    *token = malloc(token_length + 1);
    if (!*token) {
        perror("malloc failed");
        return 0;
    }
    strncpy(*token, start_pos + 1, token_length);
    (*token)[token_length] = '\0';

    return 1;
}

void fix_escaped_chars(char *str) {
    char *src = str, *dst = str;
    while (*src) {
        if (*src == '\\' && (*(src + 1) == '"' || *(src + 1) == '\\')) {
            *dst++ = *src++;
        }
        *dst++ = *src++;
    }
    *dst = '\0';
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <filename> [output_file]\n", argv[0]);
        return 1;
    }

    char command[512];
    snprintf(command, sizeof(command), "difft --dump-syntax \"%s\"", argv[1]);

    FILE *fp = popen(command, "r");
    if (!fp) {
        perror("popen failed");
        return 1;
    }

    size_t capacity = INITIAL_CAPACITY;
    size_t count = 0;
    char **tokens = malloc(capacity * sizeof(char *));
    if (!tokens) {
        perror("malloc failed");
        return 1;
    }

    char line[1024];
    while (fgets(line, sizeof(line), fp) != NULL) {
        if (strlen(line) == 0) continue;

        char *token = NULL;

        if (extract_token_from_line(line, "open_content", &token) ||
            extract_token_from_line(line, "content", &token) ||
            extract_token_from_line(line, "close_content", &token)) {

            if (token && strlen(token) > 0) {
                fix_escaped_chars(token);

                if (count >= capacity) {
                    capacity *= 2;
                    char **new_tokens = realloc(tokens, capacity * sizeof(char *));
                    if (!new_tokens) {
                        perror("realloc failed");
                        return 1;
                    }
                    tokens = new_tokens;
                }
                tokens[count++] = token;
            }
        }
    }

    pclose(fp);

    FILE *output = stdout;
    if (argc > 2) {
        output = fopen(argv[2], "w");
        if (!output) {
            perror("fopen failed");
            return 1;
        }
    }

    fprintf(output, "{\"tokens\": [");
    for (size_t i = 0; i < count; i++) {
        fprintf(output, "\"%s\"", tokens[i]);
        if (i < count - 1) fprintf(output, ", ");
    }
    fprintf(output, "]}\n");

    if (argc > 2) {
        fclose(output);
        printf("Output saved to %s\n", argv[2]);
    }

    for (size_t i = 0; i < count; i++) {
        free(tokens[i]);
    }
    free(tokens);

    return 0;
}
