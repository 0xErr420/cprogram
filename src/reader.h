#ifndef READER_H
#define READER_H

/// String size of 'cpu' line
///
/// Should be enough, assuming that in the file /proc/stat the line 'cpu' has a maximum of 10 fields
/// calculated: <MAX_unsigned_long_long> * <max number of fileds in 'cpu' line> + <sizeof("cpu ")>
#define CPU_READ_SIZE 224UL

/// Reader thread reads 'cpu' data and sends them to specified buffer
void *thread_Reader(void *arg);

#endif
