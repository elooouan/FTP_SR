#include "get.h"

int manage_pathname(request_t* request)
{
    char* pathname = construct_pathname(request);

    int fd = open(pathname, O_WRONLY | O_CREAT, 0666); /* read and write permissions */
    if (fd < 0) {
        return -1;
    }

    return fd;
}

void create_file(int clientfd, int fd, uint32_t file_bytes_sent, uint32_t file_size, request_t* request)
{
    uint32_t total_bytes_sent = request->total_bytes_sent;

    int n;
    if (lseek(fd, request->total_bytes_sent, SEEK_SET) == -1) {
        perror("lseek failed");
        return;
    }
    while ((n = Rio_readn(clientfd, &file_bytes_sent, sizeof(file_bytes_sent))) > 0) {
        file_bytes_sent = ntohl(file_bytes_sent);
        total_bytes_sent += file_bytes_sent;
        char file_buffer[file_bytes_sent];

        Rio_readn(clientfd, file_buffer, file_bytes_sent);

        write(fd, file_buffer, file_bytes_sent);

        log_file_transfer(request->filename, file_size, total_bytes_sent);

        if (total_bytes_sent == file_size) {    
            char* part_pathname = construct_pathname(request);
            if (part_pathname) {
                char pathname[256];
                snprintf(pathname, sizeof(pathname), "clientside/%s", request->filename);
                rename(part_pathname, pathname);

                // Delete log file
                char log_path[256];
                snprintf(log_path, sizeof(log_path), "clientside/.log/.log");
                remove(log_path);

                rmdir("clientside/.log");

                printf("Transfer successfully complete.\n");
                break;
            }
        }
    }
}

void print_display_size(uint32_t file_size, double* display_size, char** unit)
{
    if (file_size >= 1024 && file_size < 1048576) { 
        *display_size /= 1024;
        *unit = "KB";
    } else if (file_size >= 1048576 && file_size < 1073741824) { 
        *display_size /= 1048576;
        *unit = "MB";
    } else if (file_size >= 1073741824) { 
        *display_size /= 1073741824;
        *unit = "GB";
    }

    printf("File size: %.2f %s\n", *display_size, *unit);
}

void get_response(int clientfd, request_t* request)
{
    uint32_t file_size, n;
    uint32_t file_bytes_sent = 0;
    if ((n = Rio_readn(clientfd, &file_size, sizeof(file_size)))> 0) {
        file_size = ntohl(file_size);

        /* error code */
        if (file_size == 0xFFFFFFFF) {
            read_error(clientfd);
        } else {
            double display_size = file_size;
            char *unit = "bytes";
            print_display_size(file_size, &display_size, &unit);
            
            int fd;
            if ((fd = manage_pathname(request)) == -1) {
                perror("opening/creating file failed");
                return;
            }

            struct timeval start, end;
            gettimeofday(&start, NULL);

            create_file(clientfd, fd, file_bytes_sent, file_size, request);

            gettimeofday(&end, NULL);
            int delay = (int)((end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6);
            printf("%.2f %s received in %d seconds\n", display_size, unit, delay);


            
            close(fd);
        }
    }
}

void read_error(int clientfd)
{
    int error_code;
    Rio_readn(clientfd, &error_code, sizeof(int));
    error_code = ntohl(error_code);

    int msg_len;
    Rio_readn(clientfd, &msg_len, sizeof(msg_len));
    msg_len = ntohl(msg_len);

    char *message = malloc(msg_len + 1);
    if (message == NULL) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }

    Rio_readn(clientfd, message, msg_len);

    message[msg_len] = '\0';

    printf("Error %d: %s\n", error_code, message);

    free(message);
}

char* construct_pathname(request_t* request)
{
    int pathname_size = strlen("clientside/") + 1 + strlen(request->filename) + strlen(".part") + 1;
    char* pathname = (char*)malloc(pathname_size);
    if (pathname == NULL) {
        perror("malloc");
        return NULL;
    }
    
    snprintf(pathname, pathname_size, "clientside/.%s.part", request->filename);

    return pathname;
}

void log_file_transfer(char* filename, uint32_t file_size, uint32_t total_bytes_sent)
{
    char log_dir[256];
    snprintf(log_dir, sizeof(log_dir), "clientside/.log");

    // Create .log directory if it doesn't exist
    if (mkdir(log_dir, 0777) == -1 && errno != EEXIST) {
        perror("mkdir .log");
        return;
    }

    char log_path[256];
    char tmp_log_path[256];

    int log_path_size = sizeof(log_dir) + 5 + 1;
    int tmp_log_path_size = sizeof(tmp_log_path) + 9 + 1; 

    snprintf(log_path, log_path_size, "%s/.log", log_dir);
    snprintf(tmp_log_path, tmp_log_path_size, "%s/.log.tmp", log_dir);

    FILE* log_file = fopen(tmp_log_path, "w");
    if (!log_file) {
        perror("fopen temp log");
        return;
    }

    fprintf(log_file,
        "{\n"
        "  \"filename\": \"%s\",\n"
        "  \"file_size\": %u,\n"
        "  \"total_bytes_sent\": %u\n"
        "}\n",
        filename, file_size, total_bytes_sent
    );

    fclose(log_file);

    if (rename(tmp_log_path, log_path) != 0) {
        perror("rename temp log");
        remove(tmp_log_path);
    }
}

/* used chatgpt for the filename extraction -> a sepcifier dans le compte rendu */
int parse_resume_log(const char* log_path, request_t* request, uint32_t* total_bytes_sent, uint32_t* file_size)
{
    FILE* f = fopen(log_path, "r");
    if (!f) {
        perror("fopen resume log");
        return -1;
    }

    char buffer[512];
    fread(buffer, 1, sizeof(buffer), f);
    fclose(f);

    // Format is fixed
    char* fname_start = strstr(buffer, "\"filename\":");
    char* size_start = strstr(buffer, "\"file_size\":");
    char* sent_start = strstr(buffer, "\"total_bytes_sent\":");

    if (!fname_start || !size_start || !sent_start) {
        fprintf(stderr, "Invalid log file format\n");
        return -1;
    }

    // Extract filename
    sscanf(fname_start, "\"filename\": \"%[^\"]\"", request->filename);
    request->filename_size = strlen(request->filename);
    request->type = 0;

    // Extract file_size and total_bytes_sent
    sscanf(size_start, "\"file_size\": %u", file_size);
    sscanf(sent_start, "\"total_bytes_sent\": %u", total_bytes_sent);

    return 0;
}

void resume_file_transfer(int clientfd, request_t* request)
{
    char* serialized_request = serialize_request(request);

    send_request(clientfd, serialized_request);
    get_response(clientfd, request);
}