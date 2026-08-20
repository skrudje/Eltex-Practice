#ifndef CHANNELS_H
#define CHANNELS_H

int run_unnamed_pipe_mode(char *file_names[], int file_count);
int run_named_pipe_mode(const char *channel_name,
                        char *file_names[],
                        int file_count);

#endif
