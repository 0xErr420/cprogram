#ifndef READER_H
#define READER_H

/// Reader thread: reads 'cpu' data and sends them to specified buffer
///
/// ARG fields required: Producer
void *thread_Reader(void *arg);

#endif
